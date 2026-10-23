// The cooked bytecode contains the expansion of Unreal's ForEachLoop macros, not the macro nodes:
//
//   [BreakHit = false] ; Counter = 0 ; Index = 0
//   loop:  if ([!BreakHit &&] Counter < Array.Length) { Index = Counter ; Item = Array[Index] ; <body> ;
//                                                       Counter = Counter + 1 ; goto loop }
//          else <completed>              ("Break" inside the body is: BreakHit = true ; goto the increment)
//
// Showing that verbatim turns a three-node loop into a dozen nodes. This pass recognises the
// expansion in the expanded reference graph and puts the original "For Each Loop" (or "... with
// Break") node back; a loop that does not match the pattern exactly is left untouched.
import { builtins } from "./model.js";

const COUNTER = /^Temp_int_Loop_Counter_Variable(_\d+)?$/, INDEX = /^Temp_int_Array_Index_Variable(_\d+)?$/, BREAK = /^Temp_bool_True_if_break_was_hit_Variable(_\d+)?$/;

export function collapseForEachLoops(graph) {
  const templates = { plain: builtins.find((d) => d.id === "ue.ForEachLoop"), withBreak: builtins.find((d) => d.id === "ue.ForEachLoopWithBreak") };
  if (!templates.plain || !templates.withBreak) return 0;
  foldMakeArrays(graph);
  let collapsed = 0, guard = 0;
  while (guard++ < 60 && collapseOne(graph, templates)) collapsed++;
  const forLoop = builtins.find((d) => d.id === "ue.ForLoop");
  while (forLoop && guard++ < 120 && collapseForLoop(graph, forLoop)) collapsed++;
  const switchInt = builtins.find((d) => d.id === "ue.SwitchOnInt");
  while (switchInt && guard++ < 180 && collapseSwitchInt(graph, switchInt)) collapsed++;
  while (guard++ < 240 && collapseCast(graph)) collapsed++;
  foldConstantTemps(graph);
  forwardPureTemps(graph);
  return collapsed;
}

const COUNT = /^Temp_int_Variable(_\d+)?$/;

// ForLoop(First, Last):   T = First ; loop: if (T <= Last) { <body> ; T = T + 1 ; goto loop } else <completed>
function collapseForLoop(graph, template) {
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const source = (n, pin) => { const e = into(n, pin)[0]; return e && { node: node(e.from.node), pin: e.from.pin }; };
  const target = (n, pin) => { const e = from(n, pin)[0]; return e && { node: node(e.to.node), pin: e.to.pin }; };
  const preds = (n) => into(n, "exec-in").map((e) => node(e.from.node));
  for (const branch of graph.nodes) {
    if (branch.definition.name !== "Branch") continue;
    const test = source(branch, "condition")?.node;
    if (test?.definition.name !== "LessEqual_IntInt") continue;
    const getter = source(test, "arg0")?.node, name = getter?.definition.name.slice(4);
    if (!getter?.definition.name.startsWith("Get ") || !COUNT.test(name)) continue;
    const setters = graph.nodes.filter((n) => n.definition.name === `Set ${name}`);
    const increment = setters.find((n) => target(n, "exec-out")?.node === branch && source(n, "value")?.node.definition.name === "Add_IntInt");
    const init = setters.find((n) => n !== increment && target(n, "exec-out")?.node === branch);
    if (!increment || !init || setters.length !== 2 || preds(branch).length !== 2) continue;
    const add = source(increment, "value").node;
    if (source(add, "arg0")?.node.definition.name !== `Get ${name}` || into(add, "arg1").length || String(add.values?.arg1) !== "1") continue;
    const region = new Set([branch, test, init, increment, add]);
    const getters = graph.nodes.filter((n) => n.definition.name === `Get ${name}`);
    const bodyStart = target(branch, "true"), completed = target(branch, "false");
    if ((bodyStart && region.has(bodyStart.node)) || (completed && region.has(completed.node))) continue;
    const loop = { id: `loop-${branch.id}`, x: branch.x, y: branch.y, title: "", comment: "", values: {}, folded: {}, definition: structuredClone(template) };
    let serial = 0;
    const edge = (f, t) => ({ id: `${loop.id}-edge-${serial++}`, from: f, to: t });
    const added = into(init, "exec-in").map((e) => edge(e.from, { node: loop.id, pin: "exec-in" }));
    if (!added.length) continue;
    const bind = (src, pin, valueNode, valuePin) => {
      const w = source(valueNode, valuePin);
      if (w) added.push(edge({ node: w.node.id, pin: w.pin }, { node: loop.id, pin }));
      else if (valueNode.values?.[valuePin] !== undefined) loop.values[pin] = valueNode.values[valuePin];
    };
    bind(null, "FirstIndex", init, "value");
    bind(null, "LastIndex", test, "arg1");
    if (bodyStart) added.push(edge({ node: loop.id, pin: "LoopBody" }, { node: bodyStart.node.id, pin: bodyStart.pin }));
    if (completed) added.push(edge({ node: loop.id, pin: "Completed" }, { node: completed.node.id, pin: completed.pin }));
    for (const g of getters) for (const e of from(g)) if (!region.has(node(e.to.node))) added.push(edge({ node: loop.id, pin: "Index" }, e.to));
    const doomed = new Set([...region, ...getters]);
    graph.edges = graph.edges.filter((e) => !doomed.has(node(e.from.node)) && !doomed.has(node(e.to.node)));
    graph.nodes = graph.nodes.filter((n) => !doomed.has(n));
    graph.nodes.push(loop);
    graph.edges.push(...added);
    return true;
  }
  return false;
}

// Compiler temporaries that only carry a constant (text and enum literals, Make Literal ...) are just the
// constant: write it on the pin that reads it and drop the assignment.
function foldConstantTemps(graph) {
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const TEMP_SET = /^Set Temp_[a-z]+_Variable(_\d+)?$/i;
  const pinType = (n, id) => n.definition.pins.find((p) => p.id === id)?.type;
  const constant = (n) => (n.definition.name.match(TEMP_SET) && !into(n, "value").length && n.values?.value !== undefined && n.values.value !== "" ? n.values.value : undefined);
  const gone = new Set(), added = [];
  const place = (consumers, setter) => {
    const value = constant(setter), type = pinType(setter, "value");
    if (consumers.some((e) => pinType(node(e.to.node), e.to.pin) !== type || into(node(e.to.node), e.to.pin).length > 1)) return false;
    for (const e of consumers) node(e.to.node).values = { ...node(e.to.node).values, [e.to.pin]: value };
    return true;
  };
  const drops = new Set();
  // direct wire from the assignment's own output
  for (const setter of graph.nodes) {
    if (constant(setter) === undefined) continue;
    const out = from(setter, "value-out");
    if (!out.length || graph.nodes.some((n) => n.definition.name === `Get ${setter.definition.name.slice(4)}`) || !place(out, setter)) continue;
    for (const e of out) drops.add(e);
    gone.add(setter);
  }
  // through a getter whose every writer assigns the same constant
  const writers = new Map();
  for (const n of graph.nodes) if (TEMP_SET.test(n.definition.name)) (writers.get(n.definition.name.slice(4)) || writers.set(n.definition.name.slice(4), []).get(n.definition.name.slice(4))).push(n);
  for (const [name, list] of writers) {
    if (list.some((n) => gone.has(n)) || list.some((n) => constant(n) === undefined || constant(n) !== constant(list[0]) || pinType(n, "value") !== pinType(list[0], "value"))) continue;
    const getters = graph.nodes.filter((n) => n.definition.name === `Get ${name}`);
    const consumers = getters.flatMap((g) => from(g));
    if (list.some((n) => from(n, "value-out").length) || !getters.length || !place(consumers, list[0])) continue;
    for (const e of consumers) drops.add(e);
    getters.forEach((g) => gone.add(g)); list.forEach((n) => gone.add(n));
  }
  if (!gone.size) return;
  // splice the assignments out of the execution chain
  for (const n of gone) {
    if (n.definition.kind === "getter") continue;
    for (const inn of into(n, "exec-in")) for (const out of from(n).filter((e) => pinType(n, e.from.pin) === "exec"))
      if (!gone.has(node(inn.from.node)) && !gone.has(node(out.to.node))) added.push({ id: `fold-${n.id}-${added.length}`, from: inn.from, to: out.to });
  }
  graph.edges = graph.edges.filter((e) => !drops.has(e) && !gone.has(node(e.from.node)) && !gone.has(node(e.to.node))).concat(added);
  graph.nodes = graph.nodes.filter((n) => !gone.has(n));
}

function collapseOne(graph, templates) {
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const source = (n, pin) => { const e = into(n, pin)[0]; return e && { node: node(e.from.node), pin: e.from.pin }; };
  const target = (n, pin) => { const e = from(n, pin)[0]; return e && { node: node(e.to.node), pin: e.to.pin }; };
  const isSet = (n, re) => n && n.definition.name.startsWith("Set ") && re.test(n.definition.name.slice(4));
  const isConst = (n, pin, value) => !into(n, pin).length && String(n.values?.[pin]) === String(value);
  const wiresOf = (n) => into(n).filter((e) => e.to.pin !== "exec-in").map((e) => `${e.to.pin}<${e.from.node}.${e.from.pin}`).sort().join();
  const sameCall = (a, b) => a.definition.name === b.definition.name && JSON.stringify(a.values || {}) === JSON.stringify(b.values || {}) && wiresOf(a) === wiresOf(b);
  const preds = (n) => into(n, "exec-in").map((e) => node(e.from.node));

  for (const branch of graph.nodes) {
    if (branch.definition.name !== "Branch") continue;
    // condition: Counter < Length   or   !BreakHit && Counter < Length
    let less = source(branch, "condition")?.node, and = null, not = null, breakGet = null;
    if (less?.definition.name === "BooleanAND") {
      and = less;
      const a = source(and, "arg0")?.node, b = source(and, "arg1")?.node;
      not = [a, b].find((n) => n?.definition.name === "Not_PreBool");
      less = [a, b].find((n) => n?.definition.name === "Less_IntInt");
      breakGet = not && source(not, "arg0")?.node;
      if (!not || !less || !breakGet || !/^Get /.test(breakGet.definition.name) || !BREAK.test(breakGet.definition.name.slice(4))) continue;
    }
    if (less?.definition.name !== "Less_IntInt") continue;
    const counterGet = source(less, "arg0")?.node, length = source(less, "arg1")?.node;
    if (!counterGet || !counterGet.definition.name.startsWith("Get ") || !COUNTER.test(counterGet.definition.name.slice(4)) || length?.definition.name !== "Array_Length") continue;
    const counterVar = counterGet.definition.name.slice(4), array = source(length, "arg0"), breakVar = breakGet?.definition.name.slice(4);
    if (!array) continue;

    // body entry: Index = Counter
    const set = target(branch, "true")?.node;
    if (!isSet(set, INDEX)) continue;
    const indexVar = set.definition.name.slice(4);
    if (source(set, "value")?.node.definition.name !== `Get ${counterVar}`) continue;
    const completed = target(branch, "false");

    // An array that is a function's result is fetched again on every pass: the call sits in front of the
    // test and again in front of the element read, and the back edge returns to the first one.
    let arrayGetters = [], entryCall = null, bodyCall = null, arrayFrom = { node: array.node, pin: array.pin };
    if (array.node.definition.kind === "getter" && /^Get CallFunc_/.test(array.node.definition.name) && !/^Get CallFunc_Array_Get_Item/.test(array.node.definition.name)) {
      const writers = graph.referenceProducers?.get(`local:${array.node.definition.name.slice(4).toLowerCase()}`) || [];
      const first = writers.find((w) => preds(branch).includes(w.node));
      const again = writers.find((w) => w !== first && target(set, "exec-out")?.node === w.node);
      if (first && again && writers.length === 2 && sameCall(first.node, again.node)) {
        arrayFrom = { node: first.node, pin: first.port };
        arrayGetters = graph.nodes.filter((n) => n.definition.name === array.node.definition.name);
        entryCall = first.node; bodyCall = again.node;
      }
    }
    const head = entryCall || branch;
    const bodyStart = target(bodyCall || set, "exec-out");

    // back edge: Counter = Counter + 1 -> branch
    const increment = preds(head).find((n) => n.definition.name === `Set ${counterVar}` && source(n, "value")?.node.definition.name === "Add_IntInt");
    if (!increment) continue;
    const add = source(increment, "value").node;
    if (source(add, "arg0")?.node.definition.name !== `Get ${counterVar}` || !isConst(add, "arg1", 1)) continue;

    // entry: [Break = false ->] Counter = 0 -> Index = 0 -> branch
    const indexInit = preds(head).find((n) => n !== set && n.definition.name === `Set ${indexVar}` && isConst(n, "value", 0));
    const counterInit = indexInit && preds(indexInit).find((n) => n.definition.name === `Set ${counterVar}` && isConst(n, "value", 0));
    if (!counterInit) continue;
    let breakInit = null, entryNode = counterInit;
    if (breakVar) {
      breakInit = preds(counterInit).find((n) => n.definition.name === `Set ${breakVar}` && isConst(n, "value", false));
      if (!breakInit) continue;
      entryNode = breakInit;
    }
    const entries = into(entryNode, "exec-in");
    if (!entries.length) continue;

    // "Break" inside the body: BreakHit = true, then straight to the increment
    const breakSets = breakVar ? graph.nodes.filter((n) => n.definition.name === `Set ${breakVar}` && n !== breakInit) : [];
    if (breakVar && (!breakSets.length || breakSets.some((n) => !isConst(n, "value", true) || target(n, "exec-out")?.node !== increment))) continue;

    // element reads: Array_Get(array, Index) / Get Array Item
    const sameArray = (n, pin) => { const s = source(n, pin); return s && (arrayGetters.includes(s.node) || (s.node === array.node && s.pin === array.pin)); };
    const reads = graph.nodes.filter((n) =>
      (n.definition.name === "Array_Get" && sameArray(n, "arg0") && source(n, "arg1")?.node.definition.name === `Get ${indexVar}`) ||
      (n.definition.name === "Get Array Item" && sameArray(n, "array") && source(n, "index")?.node.definition.name === `Get ${indexVar}`));
    if (arrayGetters.some((g) => from(g).some((e) => ![length, ...reads].includes(node(e.to.node))))) continue;
    const itemPin = (n) => (n.definition.name === "Array_Get" ? "arg2" : "result");

    // nothing outside the loop may read the counter / break flag / array length
    const region = new Set([branch, less, length, set, increment, add, indexInit, counterInit, ...(breakVar ? [and, not, breakInit, ...breakSets] : []), ...reads, ...(bodyCall ? [bodyCall, ...arrayGetters] : [])]);
    const readers = (name) => graph.nodes.filter((n) => n.definition.name === `Get ${name}`);
    const outside = (n) => from(n).some((e) => !region.has(node(e.to.node)));
    if (readers(counterVar).some(outside) || (breakVar && readers(breakVar).some(outside)) || outside(length)) continue;

    // ---- rewrite ----
    const arrayType = arrayFrom.node.definition.pins.find((p) => p.id === arrayFrom.pin)?.type || "";
    const elementType = /^TArray<(.+)>$/.exec(arrayType)?.[1];
    if (!elementType || elementType === "unknown") continue; // an untyped source cannot be wired to a typed loop
    const template = breakVar ? templates.withBreak : templates.plain;
    const definition = structuredClone(template);
    definition.pins = definition.wild.map((p) => ({ ...p, type: p.type.split("$T").join(elementType) }));
    const loop = { id: `loop-${branch.id}`, x: branch.x, y: branch.y, title: "", comment: "", values: {}, folded: {}, definition };
    let serial = 0;
    const edge = (f, t) => ({ id: `${loop.id}-edge-${serial++}`, from: f, to: t });
    const execPin = (n, direction) => n.definition.pins.find((p) => p.type === "exec" && p.direction === direction)?.id;
    const added = [
      ...entries.map((e) => edge(e.from, { node: entryCall?.id || loop.id, pin: entryCall ? execPin(entryCall, "in") : "exec-in" })),
      edge({ node: arrayFrom.node.id, pin: arrayFrom.pin }, { node: loop.id, pin: "Array" }),
    ];
    if (entryCall) added.push(edge({ node: entryCall.id, pin: execPin(entryCall, "out") }, { node: loop.id, pin: "exec-in" }));
    if (bodyStart && !region.has(bodyStart.node)) added.push(edge({ node: loop.id, pin: "LoopBody" }, { node: bodyStart.node.id, pin: bodyStart.pin }));
    if (completed && !region.has(completed.node)) added.push(edge({ node: loop.id, pin: "Completed" }, { node: completed.node.id, pin: completed.pin }));
    const redirect = (sourceNode, pin, outPin) => {
      for (const e of from(sourceNode, pin)) if (!region.has(node(e.to.node))) added.push(edge({ node: loop.id, pin: outPin }, e.to));
    };
    for (const g of readers(indexVar)) redirect(g, "result", "ArrayIndex");
    for (const r of reads) redirect(r, itemPin(r), "ArrayElement");
    // the temp variable the element reads wrote to (CallFunc_Array_Get_Item[_n]) is read through its own
    // getter node: when every writer of it belongs to this loop, that getter IS the loop's element
    const itemGetters = graph.nodes.filter((n) => n.definition.kind === "getter" && /^Get CallFunc_Array_Get_Item/.test(n.definition.name) &&
      graph.referenceProducers?.get(`local:${n.definition.name.slice(4).toLowerCase()}`)?.every((w) => reads.includes(w.node)));
    for (const g of itemGetters) redirect(g, "result", "ArrayElement");
    // whatever led into a "Break = true" now leads into the loop's Break pin
    for (const b of breakSets) for (const e of into(b, "exec-in")) if (!region.has(node(e.from.node))) added.push(edge(e.from, { node: loop.id, pin: "Break" }));

    const doomed = new Set([...region, ...itemGetters]);
    graph.edges = graph.edges.filter((e) => !doomed.has(node(e.from.node)) && !doomed.has(node(e.to.node)));
    graph.nodes = graph.nodes.filter((n) => !doomed.has(n));
    graph.nodes.push(loop);
    graph.edges.push(...added);
    // loop temporaries that now feed nothing
    const dead = (n) => n.definition.kind === "getter" && /^Get (Temp_int_|Temp_bool_True_if_break|CallFunc_Array_Get_Item)/.test(n.definition.name) && !graph.edges.some((e) => e.from.node === n.id);
    for (let again = true; again;) {
      const gone = graph.nodes.filter(dead);
      again = gone.length > 0;
      if (again) {
        const ids = new Set(gone.map((n) => n.id));
        graph.nodes = graph.nodes.filter((n) => !ids.has(n.id));
        graph.edges = graph.edges.filter((e) => !ids.has(e.to.node));
      }
    }
    return true;
  }
  return false;
}

// "Make / Set Array" assigns an array literal to a compiler temporary right before every use of it.
// In the blueprint that is a single Make Array node: one node per literal, in place of the temporary.
function foldMakeArrays(graph) {
  const template = builtins.find((d) => d.id === "ue.MakeArray");
  if (!template) return;
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const setters = new Map();
  for (const n of graph.nodes) {
    if (n.definition.name !== "Make / Set Array") continue;
    const getter = node(into(n, "array")[0]?.from.node);
    if (!getter || !/^Get K2Node_MakeArray_Array/.test(getter.definition.name)) continue;
    const list = setters.get(getter.definition.name) || [];
    list.push(n); setters.set(getter.definition.name, list);
  }
  let serial = 0;
  const bypass = (list, prefix) => list.flatMap((n) => into(n, "exec-in").flatMap((inn) => from(n).filter((e) => n.definition.pins.find((p) => p.id === e.from.pin)?.type === "exec")
    .filter((out) => !list.includes(node(inn.from.node)) && !list.includes(node(out.to.node)))
    .map((out) => ({ id: `${prefix}-edge-${serial++}`, from: inn.from, to: out.to }))));
  const splice = (list) => {
    const added = bypass(list, `skip-${list[0].id}`), gone = new Set(list);
    graph.edges = graph.edges.filter((e) => !gone.has(node(e.from.node)) && !gone.has(node(e.to.node))).concat(added);
    graph.nodes = graph.nodes.filter((n) => !gone.has(n));
  };
  // an item-less "Make / Set Array" on a function-result temporary copies the array before it is iterated
  const copies = graph.nodes.filter((n) => n.definition.name === "Make / Set Array" && !n.definition.pins.some((p) => /^item\d+$/.test(p.id)) &&
    /^Get CallFunc_/.test(node(into(n, "array")[0]?.from.node)?.definition.name || ""));
  if (copies.length) splice(copies);
  for (const [getterName, list] of setters) {
    const items = (n) => n.definition.pins.filter((p) => /^item\d+$/.test(p.id)).map((p) => {
      const wire = into(n, p.id)[0];
      return { id: p.id, type: p.type, wire: wire?.from, constant: n.values?.[p.id] };
    });
    const signature = (n) => JSON.stringify(items(n).map((i) => (i.wire ? [i.wire.node, i.wire.pin] : [i.constant])));
    const first = list[0], elementType = /^TArray<(.+)>$/.exec(first.definition.pins.find((p) => p.id === "array")?.type || "")?.[1];
    if (!elementType || elementType === "unknown" || !items(first).length || list.some((n) => signature(n) !== signature(first))) continue;
    const getters = graph.nodes.filter((n) => n.definition.name === getterName);
    const count = items(first).length;
    const definition = structuredClone(template);
    definition.wild = [
      ...Array.from({ length: count }, (_, i) => ({ ...template.wild[0], id: `Item${i}`, name: String(i) })),
      template.wild.find((p) => p.id === "Array"),
    ];
    definition.pins = definition.wild.map((p) => ({ ...p, type: p.type.split("$T").join(elementType) }));
    const make = { id: `makearray-${getters[0]?.id || first.id}`, x: first.x, y: first.y, title: "", comment: "", values: {}, folded: {}, definition };
    items(first).forEach((item, i) => { if (!item.wire) make.values[`Item${i}`] = item.constant; });
    const added = items(first).flatMap((item, i) => item.wire ? [{ id: `${make.id}-edge-${serial++}`, from: item.wire, to: { node: make.id, pin: `Item${i}` } }] : []);
    for (const g of getters) for (const e of from(g)) if (!list.includes(node(e.to.node))) added.push({ id: `${make.id}-edge-${serial++}`, from: { node: make.id, pin: "Array" }, to: e.to });
    added.push(...bypass(list, make.id));
    const doomed = new Set([...list, ...getters]);
    graph.edges = graph.edges.filter((e) => !doomed.has(node(e.from.node)) && !doomed.has(node(e.to.node)));
    graph.nodes = graph.nodes.filter((n) => !doomed.has(n));
    graph.nodes.push(make);
    graph.edges.push(...added);
  }
}

// A compiler temporary assigned once from a pure expression is that expression: wire the readers to it.
// The Select node's unassigned "default" temporary is not a value at all.
function forwardPureTemps(graph) {
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const isPure = (n) => n.definition.pins.every((p) => p.type !== "exec") && n.definition.kind !== "getter";
  const gone = new Set(), drops = new Set(), added = [];
  for (const g of graph.nodes.filter((n) => /^Get K2Node_Select_Default/.test(n.definition.name) && !graph.referenceProducers?.get(`local:${n.definition.name.slice(4).toLowerCase()}`)?.length)) {
    gone.add(g);
  }
  const names = new Set(graph.nodes.filter((n) => /^Set Temp_[a-z]+_Variable(_\d+)?$/i.test(n.definition.name)).map((n) => n.definition.name.slice(4)));
  for (const name of names) {
    const writers = graph.nodes.filter((n) => n.definition.name === `Set ${name}`);
    if (writers.length !== 1) continue;
    const [setter] = writers, wire = into(setter, "value")[0], src = wire && node(wire.from.node);
    if (!src || !isPure(src) || gone.has(src)) continue;
    const getters = graph.nodes.filter((n) => n.definition.name === `Get ${name}`);
    const readers = [...getters.flatMap((g) => from(g)), ...from(setter, "value-out")];
    if (!readers.length) continue;
    const typeOf = (id, pin, n) => n.definition.pins.find((p) => p.id === pin)?.type;
    const from_ = src.definition.pins.find((p) => p.id === wire.from.pin)?.type;
    if (readers.some((e) => into(node(e.to.node), e.to.pin).length > 1 || typeOf(null, e.to.pin, node(e.to.node)) !== from_)) continue;
    for (const e of readers) { drops.add(e); added.push({ id: `fwd-${setter.id}-${added.length}`, from: wire.from, to: e.to }); }
    getters.forEach((g) => gone.add(g)); gone.add(setter);
    for (const inn of into(setter, "exec-in")) for (const out of from(setter).filter((e) => setter.definition.pins.find((p) => p.id === e.from.pin)?.type === "exec"))
      added.push({ id: `fwd-${setter.id}-${added.length}`, from: inn.from, to: out.to });
  }
  if (!gone.size) return;
  graph.edges = graph.edges.filter((e) => !drops.has(e) && !gone.has(node(e.from.node)) && !gone.has(node(e.to.node))).concat(added.filter((e) => !gone.has(node(e.from.node)) && !gone.has(node(e.to.node))));
  graph.nodes = graph.nodes.filter((n) => !gone.has(n));
}

// Switch on Int compiles to a chain of tests whose results go through a temporary that the viewer cannot see:
//   t = (Selection != 1) ; if (t) { t = (Selection != 2) ; if (t) { ... default } else <case 2> } else <case 1>
// The comparisons are left as loose pure nodes just before their Branch.
function collapseSwitchInt(graph, template) {
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const target = (n, pin) => { const e = from(n, pin)[0]; return e && { node: node(e.to.node), pin: e.to.pin }; };
  const tests = graph.nodes.filter((n) => n.definition.name === "Branch" && /^Get K2Node_SwitchInteger_CmpSuccess/.test(node(into(n, "condition")[0]?.from.node)?.definition.name || ""));
  const compareFor = (branch) => {
    const index = graph.nodes.indexOf(branch);
    for (let i = index - 1; i >= 0 && i >= index - 6; i--) {
      const n = graph.nodes[i];
      if (n.definition.name === "NotEqual_IntInt" && !from(n).length) return n;
    }
  };
  const info = new Map(tests.map((b) => [b, compareFor(b)]));
  if ([...info.values()].some((c) => !c)) { for (const [b, c] of info) if (!c) info.delete(b); }
  const chained = new Set([...info.keys()].map((b) => target(b, "true")?.node).filter((n) => info.has(n)));
  for (const first of info.keys()) {
    if (chained.has(first)) continue;
    const chain = [first];
    for (let next = target(first, "true")?.node; info.has(next) && !chain.includes(next); next = target(next, "true")?.node) chain.push(next);
    const compares = chain.map((b) => info.get(b));
    const selection = into(compares[0], "arg0")[0];
    const cases = compares.map((c) => String(c.values?.arg1));
    if (!selection || compares.some((c) => into(c, "arg1").length || !/^-?\d+$/.test(String(c.values?.arg1)) || into(c, "arg0")[0]?.from.node !== selection.from.node || into(c, "arg0")[0]?.from.pin !== selection.from.pin) || new Set(cases).size !== cases.length) continue;
    if (!into(first, "exec-in").length) continue;
    const definition = structuredClone(template);
    const exec = (id, name, direction) => ({ id, name, type: "exec", direction });
    definition.pins = [
      ...definition.pins.filter((p) => p.id === "exec-in" || p.id === "Selection"),
      ...cases.map((k) => exec(k, k, "out")),
      definition.pins.find((p) => p.id === "Default"),
    ];
    const loop = { id: `switch-${first.id}`, x: first.x, y: first.y, title: "", comment: "", values: {}, folded: {}, definition };
    let serial = 0;
    const edge = (f, t) => ({ id: `${loop.id}-edge-${serial++}`, from: f, to: t });
    const added = [...into(first, "exec-in").map((e) => edge(e.from, { node: loop.id, pin: "exec-in" })), edge(selection.from, { node: loop.id, pin: "Selection" })];
    chain.forEach((b, i) => { const t = target(b, "false"); if (t) added.push(edge({ node: loop.id, pin: cases[i] }, { node: t.node.id, pin: t.pin })); });
    const def = target(chain[chain.length - 1], "true");
    if (def) added.push(edge({ node: loop.id, pin: "Default" }, { node: def.node.id, pin: def.pin }));
    const getters = chain.map((b) => node(into(b, "condition")[0].from.node));
    const doomed = new Set([...chain, ...compares, ...getters.filter((g) => from(g).every((e) => chain.includes(node(e.to.node))))]);
    // entries into the middle of the chain would be lost: only collapse a self-contained chain
    if (chain.slice(1).some((b) => into(b, "exec-in").some((e) => !chain.includes(node(e.from.node))))) continue;
    graph.edges = graph.edges.filter((e) => !doomed.has(node(e.from.node)) && !doomed.has(node(e.to.node)));
    graph.nodes = graph.nodes.filter((n) => !doomed.has(n));
    graph.nodes.push(loop);
    graph.edges.push(...added);
    return true;
  }
  return false;
}

// Cast To X compiles to:  As = cast(Object) ; Success = (As is valid) ; if (Success) ... else ...
// The blueprint node is one execution node with Success / Cast Failed outputs and an "As X" pin.
function collapseCast(graph) {
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const target = (n, pin) => { const e = from(n, pin)[0]; return e && { node: node(e.to.node), pin: e.to.pin }; };
  for (const setAs of graph.nodes) {
    if (!/^Set K2Node_DynamicCast_As/.test(setAs.definition.name)) continue;
    const asName = setAs.definition.name.slice(4), castEdge = into(setAs, "value")[0], cast = castEdge && node(castEdge.from.node);
    const setOk = target(setAs, "exec-out")?.node;
    if (!cast || !/^Cast To /.test(cast.definition.name) || !setOk || setOk.definition.name !== `Set ${asName.replace(/_As.*$/, "")}_bSuccess`) continue;
    const convert = node(into(setOk, "value")[0]?.from.node), branch = target(setOk, "exec-out")?.node;
    if (convert?.definition.name !== "Convert To bool" || branch?.definition.name !== "Branch" || from(setOk, "value-out").some((e) => e.to.node !== branch.id) || into(branch, "condition")[0]?.from.node !== setOk.id) continue;
    const getters = graph.nodes.filter((n) => n.definition.name === `Get ${asName}`);
    if (!getters.some((g) => into(convert, "value").some((e) => e.from.node === g.id)) || from(cast).length !== 1) continue;
    const resultPin = cast.definition.pins.find((p) => p.direction === "out") || { id: "result", type: "UObject*" };
    const objectIn = into(cast, "value")[0] || into(cast)[0];
    if (!objectIn || into(setAs, "exec-in").length === 0) continue;
    const className = cast.definition.name.slice(8);
    const definition = {
      ...structuredClone(cast.definition), kind: "cast", pure: false, targetClass: resultPin.type.replace(/\*$/, ""),
      pins: [
        { id: "exec-in", name: "Exec", type: "exec", direction: "in" },
        { id: "Object", name: "Object", type: "UObject*", direction: "in" },
        { id: "Success", name: "Success", type: "exec", direction: "out" },
        { id: "Failed", name: "Cast Failed", type: "exec", direction: "out" },
        { id: "As", name: `As ${className.replace(/^[UA]?B?P?_?/, "") || className}`, type: resultPin.type, direction: "out" },
      ],
    };
    const make = { id: `cast-${setAs.id}`, x: setAs.x, y: setAs.y, title: "", comment: "", values: {}, folded: {}, definition };
    let serial = 0;
    const edge = (f, t) => ({ id: `${make.id}-edge-${serial++}`, from: f, to: t });
    const added = [...into(setAs, "exec-in").map((e) => edge(e.from, { node: make.id, pin: "exec-in" })), edge(objectIn.from, { node: make.id, pin: "Object" })];
    const ok = target(branch, "true"), failed = target(branch, "false");
    if (ok) added.push(edge({ node: make.id, pin: "Success" }, { node: ok.node.id, pin: ok.pin }));
    if (failed) added.push(edge({ node: make.id, pin: "Failed" }, { node: failed.node.id, pin: failed.pin }));
    for (const g of getters) for (const e of from(g)) if (e.to.node !== convert.id) added.push(edge({ node: make.id, pin: "As" }, e.to));
    const doomed = new Set([setAs, setOk, convert, branch, cast, ...getters]);
    if (graph.nodes.some((n) => n.definition.name === `Get ${setOk.definition.name.slice(4)}`)) continue;
    graph.edges = graph.edges.filter((e) => !doomed.has(node(e.from.node)) && !doomed.has(node(e.to.node)));
    graph.nodes = graph.nodes.filter((n) => !doomed.has(n));
    graph.nodes.push(make);
    graph.edges.push(...added);
    return true;
  }
  return false;
}
