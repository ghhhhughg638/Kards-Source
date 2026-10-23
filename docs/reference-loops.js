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
  let collapsed = 0, guard = 0;
  while (guard++ < 60 && collapseOne(graph, templates)) collapsed++;
  return collapsed;
}

function collapseOne(graph, templates) {
  const node = (id) => graph.nodes.find((n) => n.id === id);
  const into = (n, pin) => graph.edges.filter((e) => e.to.node === n.id && (pin === undefined || e.to.pin === pin));
  const from = (n, pin) => graph.edges.filter((e) => e.from.node === n.id && (pin === undefined || e.from.pin === pin));
  const source = (n, pin) => { const e = into(n, pin)[0]; return e && { node: node(e.from.node), pin: e.from.pin }; };
  const target = (n, pin) => { const e = from(n, pin)[0]; return e && { node: node(e.to.node), pin: e.to.pin }; };
  const isSet = (n, re) => n && n.definition.name.startsWith("Set ") && re.test(n.definition.name.slice(4));
  const isConst = (n, pin, value) => !into(n, pin).length && String(n.values?.[pin]) === String(value);
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
    const bodyStart = target(set, "exec-out"), completed = target(branch, "false");

    // back edge: Counter = Counter + 1 -> branch
    const increment = preds(branch).find((n) => n.definition.name === `Set ${counterVar}` && source(n, "value")?.node.definition.name === "Add_IntInt");
    if (!increment) continue;
    const add = source(increment, "value").node;
    if (source(add, "arg0")?.node.definition.name !== `Get ${counterVar}` || !isConst(add, "arg1", 1)) continue;

    // entry: [Break = false ->] Counter = 0 -> Index = 0 -> branch
    const indexInit = preds(branch).find((n) => n !== set && n.definition.name === `Set ${indexVar}` && isConst(n, "value", 0));
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
    const sameArray = (n, pin) => { const s = source(n, pin); return s && s.node === array.node && s.pin === array.pin; };
    const reads = graph.nodes.filter((n) =>
      (n.definition.name === "Array_Get" && sameArray(n, "arg0") && source(n, "arg1")?.node.definition.name === `Get ${indexVar}`) ||
      (n.definition.name === "Get Array Item" && sameArray(n, "array") && source(n, "index")?.node.definition.name === `Get ${indexVar}`));
    const itemPin = (n) => (n.definition.name === "Array_Get" ? "arg2" : "result");

    // nothing outside the loop may read the counter / break flag / array length
    const region = new Set([branch, less, length, set, increment, add, indexInit, counterInit, ...(breakVar ? [and, not, breakInit, ...breakSets] : []), ...reads]);
    const readers = (name) => graph.nodes.filter((n) => n.definition.name === `Get ${name}`);
    const outside = (n) => from(n).some((e) => !region.has(node(e.to.node)));
    if (readers(counterVar).some(outside) || (breakVar && readers(breakVar).some(outside)) || outside(length)) continue;

    // ---- rewrite ----
    const arrayType = array.node.definition.pins.find((p) => p.id === array.pin)?.type || "";
    const elementType = /^TArray<(.+)>$/.exec(arrayType)?.[1];
    if (!elementType || elementType === "unknown") continue; // an untyped source cannot be wired to a typed loop
    const template = breakVar ? templates.withBreak : templates.plain;
    const definition = structuredClone(template);
    definition.pins = definition.wild.map((p) => ({ ...p, type: p.type.split("$T").join(elementType) }));
    const loop = { id: `loop-${branch.id}`, x: branch.x, y: branch.y, title: "", comment: "", values: {}, folded: {}, definition };
    let serial = 0;
    const edge = (f, t) => ({ id: `${loop.id}-edge-${serial++}`, from: f, to: t });
    const added = [
      ...entries.map((e) => edge(e.from, { node: loop.id, pin: "exec-in" })),
      edge({ node: array.node.id, pin: array.pin }, { node: loop.id, pin: "Array" }),
    ];
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
