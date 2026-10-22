// Cooked Kismet -> compact, inspectable Studio graphs. By Codex.
// Source instructions stay in the reference data. Layout is reconstructed, not
// claimed to be the original editor layout. Calls/expressions are not executed.
import { makeFunctionGraph, checkName } from "./blueprint.js";
import { compatible } from "./model.js";

const CALLS = new Set(["VirtualFunction", "LocalVirtualFunction", "FinalFunction", "LocalFinalFunction", "CallMath", "CallMulticastDelegate"]);
const ASSIGN = new Set(["Let", "LetBool", "LetObj", "LetValueOnPersistentFrame"]);
const META = new Set(["ComputedJump", "PushExecutionFlow", "PopExecutionFlow", "Jump", "EndOfScript"]);
const TEMP = /^(CallFunc_|Temp_|K2Node_|UberGraph)/;
const pin = (id, name, type, direction) => ({ id, name, type, direction });
const safe = (s, prefix = "Ref") => {
  let n = String(s).replace(/[^A-Za-z0-9_]/g, "_").slice(0, 58);
  if (checkName(n)) n = `${prefix}_${n}`.slice(0, 64);
  return n;
};
const peel = (x) => x?.op === "Context" ? peel(x.value) : x;
const variable = (x) => x?.op === "Variable" ? x.name : null;
const isEnum = (t, library) => Object.hasOwn(library.enums || {}, t);
const isConst = (x) => x && (/Const$/.test(x.op) || ["True", "False", "Self", "NoObject", "Nothing"].includes(x.op));

export function expressionType(x, library) {
  if (!x) return "unknown";
  if (x.type && x.type !== "unknown") return x.type;
  if (x.op === "Context") return expressionType(x.value, library);
  if (x.op === "StructMemberContext") return expressionType(x.member, library);
  if (["True", "False"].includes(x.op)) return "bool";
  if (x.op === "IntConst") return "int32";
  if (x.op === "ByteConst") return "uint8";
  if (["DoubleConst", "FloatConst"].includes(x.op)) return x.op === "DoubleConst" ? "double" : "float";
  if (x.op === "NameConst") return "FName";
  if (["StringConst", "UnicodeStringConst"].includes(x.op)) return "FString";
  if (x.op === "TextConst") return "FText";
  if (x.op === "Self") return "UBaseCardObject*";
  if (x.op === "SwitchValue") return expressionType(x.cases?.[0]?.value || x.default, library);
  const s = library.signatures?.[x.signature];
  if (s) return s.params.find((p) => p.return)?.type || s.params.find((p) => p.direction === "out")?.type || "unknown";
  if (/Equal|Less|Greater|Boolean|Not_PreBool/.test(x.name || "")) return "bool";
  return "unknown";
}

export function enumValue(x, type, library) {
  if (!isEnum(type, library)) return x?.value;
  if (typeof x?.value === "number") return library.enumValues?.[type]?.[String(x.value)] ?? `#${x.value}`;
  return String(x?.value ?? "").split("::").pop();
}

export function expressionText(x, library, depth = 0) {
  if (!x) return "None";
  if (depth > 12) return "…";
  const rec = (v) => expressionText(v, library, depth + 1);
  if (x.op === "Variable") return `${x.scope === "member" ? "Self." : ""}${x.name}`;
  if (x.op === "Context") return `${rec(x.object)}.${rec(x.value)}`;
  if (x.op === "Self") return "Self";
  if (x.op === "True" || x.op === "False") return x.op.toLowerCase();
  if (x.op === "NoObject" || x.op === "Nothing") return "None";
  if (x.op === "StructMemberContext") return `${rec(x.object)}.${x.member?.name || "member"}`;
  if (x.op === "ArrayGetByRef") return `${rec(x.array)}[${rec(x.index)}]`;
  if (x.op === "Cast" || x.op === "DynamicCast") return `${x.type}(${rec(x.value)})`;
  if (x.op === "SwitchValue") return `Select(${rec(x.index)}; ${(x.cases || []).map((c) => `${rec(c.match)}: ${rec(c.value)}`).join("; ")}; default: ${rec(x.default)})`;
  if (x.op === "StructConst") return `${x.type}{${(x.values || []).map(rec).join(", ")}}`;
  if (CALLS.has(x.op)) {
    const enumType = (x.args || []).map((a) => expressionType(a, library)).find((t) => isEnum(t, library));
    const op = { EqualEqual_ByteByte: "==", NotEqual_ByteByte: "!=", BooleanAND: "&&", BooleanOR: "||", Less_IntInt: "<", Greater_IntInt: ">", Add_IntInt: "+", Subtract_IntInt: "−" }[x.name];
    if (op && x.args?.length === 2) return x.args.map((a) => enumType && a.op === "ByteConst" ? `${enumType}::${enumValue(a, enumType, library)}` : rec(a)).join(` ${op} `);
    return `${x.name}(${(x.args || []).map(rec).join(", ")})`;
  }
  if (x.op === "ObjectConst") return x.value || "None";
  if (x.value !== undefined) return typeof x.value === "object" ? JSON.stringify(x.value) : String(x.value);
  return x.op;
}

function buildBody(fn, graph, library, prefix, entry, result, diagnostics, expanded) {
  const body = fn.body || [];
  const byAt = new Map(body.map((s, i) => [s.at, i]));
  const writes = new Map();
  for (const s of body) {
    if (ASSIGN.has(s.op) && variable(s.target)) {
      const values = writes.get(s.target.name) || [];
      values.push(s.value); writes.set(s.target.name, values);
    }
    const c = peel(ASSIGN.has(s.op) ? s.value : s);
    if (!CALLS.has(c?.op)) continue;
    const sig = library.signatures?.[c.signature];
    (sig?.params || []).filter((p) => !p.return).forEach((p, i) => {
      const arg = c.args?.[i];
      if (p.direction === "out" && variable(arg) && sig.pure) {
        const values = writes.get(arg.name) || [];
        values.push(c); writes.set(arg.name, values);
      }
    });
  }
  const resolve = (x, seen = new Set()) => {
    if (expanded) return x;
    if (x?.op === "Variable" && TEMP.test(x.name) && writes.get(x.name)?.length === 1 && !seen.has(x.name) && !effectful(writes.get(x.name)[0])) {
      const next = new Set(seen); next.add(x.name);
      return resolve(writes.get(x.name)[0], next);
    }
    if (!x || typeof x !== "object") return x;
    if (Array.isArray(x)) return x.map((v) => resolve(v, seen));
    return Object.fromEntries(Object.entries(x).map(([k, v]) => [k, typeof v === "object" ? resolve(v, seen) : v]));
  };
  const effectful = (x) => {
    if (!x || typeof x !== "object") return false;
    if (Array.isArray(x)) return x.some(effectful);
    if (CALLS.has(x.op) && library.signatures?.[x.signature]?.pure !== true) return true;
    return Object.values(x).some((v) => typeof v === "object" && effectful(v));
  };
  const visible = new Map();
  const statements = new Map();
  const producers = new Map();
  const keyOf = (v) => `${v?.scope === "member" ? "member" : "local"}:${v?.name?.toLowerCase()}`;
  const produce = (v, node, port) => {
    if (!variable(v)) return;
    const key = keyOf(v), list = producers.get(key) || [];
    list.push({ node, port }); producers.set(key, list);
  };
  for (const start of (Array.isArray(entry) ? entry : [entry]).filter(Boolean)) {
    for (const binding of start.bindings || []) produce(binding.variable, start.node, binding.pin);
  }
  const make = (s, name, kind = "function") => {
    const n = { id: `${prefix}-${s.at}`, x: 0, y: 0, title: "", comment: "", values: {}, folded: {}, definition: { id: `reference.${name}`, name, class: "Reference", category: kind === "flow" ? "flow" : "cards", kind, pure: false, pins: [pin("exec-in", "Exec", "exec", "in"), pin("exec-out", "Then", "exec", "out")], reference: { sourceOffset: s.at, function: fn.name } } };
    graph.nodes.push(n); visible.set(s.at, n); statements.set(n.id, s);
    return n;
  };
  const input = (n, id, name, type, expr) => {
    type = type || "unknown";
    n.definition.pins.push(pin(id, name, type, "in"));
    if (isConst(expr) && expr.op !== "StructConst") n.values[id] = isEnum(type, library) ? enumValue(expr, type, library) : expr.op === "True" ? true : expr.op === "False" ? false : expr.op === "Self" ? "Self" : typeof expr.value === "object" ? expressionText(expr, library) : expr.value ?? "None";
    else n.folded[id] = { text: expressionText(expr, library), expression: expr, type };
  };
  const callNode = (s, expression, outTarget) => {
    const c = peel(expression);
    const sig = library.signatures?.[c.signature];
    const n = make(s, c.name, sig?.pure ? "pure" : "function");
    const enumType = c.args?.map((a) => expressionType(a, library)).find((t) => isEnum(t, library));
    const enumCompare = enumType && /Equal.*ByteByte/.test(c.name);
    if (enumCompare) {
      n.definition.name = `${/NotEqual/.test(c.name) ? "Not Equal" : "Equal"} (${enumType})`;
      n.definition.enumComparison = enumType;
    }
    if (sig?.pure) {
      n.definition.pure = true;
      n.definition.pins = [];
      visible.delete(s.at);
    }
    n.definition.signature = c.signature;
    if (sig?.evidence) n.definition.reference.signatureEvidence = sig.evidence;
    if (expression.op === "Context") input(n, "target", "Target", expressionType(expression.object, library), resolve(expression.object));
    const params = sig?.params.filter((p) => !p.return) || c.args.map((a, i) => ({ name: `Arg${i}`, type: expressionType(a, library), direction: variable(a)?.startsWith(`CallFunc_${c.name}_`) ? "out" : "in" }));
    params.forEach((p, i) => {
      if (p.direction === "out") {
        n.definition.pins.push(pin(`arg${i}`, p.name, p.type === "unknown" ? expressionType(c.args[i], library) : p.type, "out"));
        produce(c.args[i], n, `arg${i}`);
      }
      else input(n, enumCompare ? i === 0 ? "A" : "B" : `arg${i}`, p.name, enumCompare ? enumType : p.type === "unknown" ? expressionType(c.args[i], library) : p.type, resolve(c.args[i]));
    });
    if (outTarget) { n.definition.pins.push(pin("return", sig?.params.find((p) => p.return)?.name || "ReturnValue", expressionType(outTarget, library), "out")); produce(outTarget, n, "return"); }
    const dataInputs = n.definition.pins.filter((p) => p.type !== "exec" && p.direction === "in" && p.id !== "target");
    n.definition.advancedPins = dataInputs.slice(3).filter((p) => !isEnum(p.type, library)).map((p) => p.id);
    return n;
  };
  for (const s of body) {
    if (META.has(s.op)) continue;
    if (s.op === "Return") {
      const n = result && !visible.has(s.at) ? { ...result, id: `${prefix}-${s.at}`, definition: structuredClone(result.definition), values: {}, folded: {} } : make(s, "Return", "functionResult");
      if (result) {
        n.definition.bp.extra = true; graph.nodes.push(n); visible.set(s.at, n); statements.set(n.id, s);
      }
      n.definition.pins = n.definition.pins.filter((p) => p.id !== "exec-out");
      n.definition.reference = { sourceOffset: s.at, function: fn.name };
      const outputs = fn.params.filter((p) => p.direction === "out" || p.return);
      n.definition.pins.filter((p) => p.direction === "in" && p.type !== "exec").forEach((p, i) => {
        const output = outputs[i];
        if (!output) return;
        const value = output.return && s.value?.op !== "Nothing" ? resolve(s.value) : { op: "Variable", name: output.name, type: output.type, scope: "local" };
        if (isConst(value)) n.values[p.id] = isEnum(p.type, library) ? enumValue(value, p.type, library) : value.op === "True" ? true : value.op === "False" ? false : value.value ?? "None";
        else n.folded[p.id] = { text: expressionText(value, library), expression: value, type: p.type };
      });
      continue;
    }
    if (s.op === "JumpIfNot" || s.op === "PopExecutionFlowIfNot") {
      const previous = body[byAt.get(s.at) - 1];
      const condition = ASSIGN.has(previous?.op) && variable(previous.target) === variable(s.condition) ? previous.value : s.condition;
      const c = peel(resolve(condition));
      const enumType = c?.args?.map((a) => expressionType(a, library)).find((t) => isEnum(t, library));
      const n = make(s, enumType ? `${/NotEqual/.test(c.name) ? "Not Equal" : "Equal"} (${enumType})` : "Branch", "flow");
      n.definition.pins = [pin("exec-in", "Exec", "exec", "in"), pin("true", "True", "exec", "out"), pin("false", "False", "exec", "out")];
      if (enumType && /Equal.*ByteByte/.test(c.name)) {
        n.definition.enumComparison = enumType;
        n.definition.reference.comparison = c;
        input(n, "A", "A", enumType, c.args[0]); input(n, "B", "B", enumType, c.args[1]);
      } else input(n, "condition", "Condition", "bool", resolve(expanded ? s.condition : condition));
      continue;
    }
    if (ASSIGN.has(s.op)) {
      const c = peel(s.value);
      const temp = s.target?.scope === "frame" || TEMP.test(s.target?.name || "");
      if (temp && !effectful(s.value) && !expanded) continue;
      // Enum execution cases stay on the function; do not duplicate the
      // compiler-generated byte-comparison temporary as a separate call.
      const next = body[byAt.get(s.at) + 1];
      if (expanded && temp && /Equal.*ByteByte/.test(c?.name || "") && c.args?.some((a) => isEnum(expressionType(a, library), library)) && ["JumpIfNot", "PopExecutionFlowIfNot"].includes(next?.op) && variable(next.condition) === variable(s.target)) continue;
      if (CALLS.has(c?.op) && !(expanded && !temp && library.signatures?.[c.signature]?.pure)) callNode(s, s.value, s.target);
      else {
        const n = make(s, `Set ${s.target?.name || "Value"}`, "setter");
        input(n, "value", s.target?.name || "Value", expressionType(s.target, library), resolve(s.value));
        n.definition.pins.push(pin("value-out", "Value", expressionType(s.target, library), "out"));
        produce(s.target, n, "value-out");
      }
      continue;
    }
    const c = peel(s), sig = library.signatures?.[c?.signature];
    if (CALLS.has(c?.op)) {
      if (sig?.pure && !expanded) continue;
      callNode(s, s);
    } else if (s.op === "SetArray") {
      const n = make(s, "Make / Set Array", "setter");
      input(n, "array", s.target?.name || "Array", expressionType(s.target, library), s.target);
      if (expanded) (s.values || []).forEach((v, i) => input(n, `item${i}`, `[${i}]`, expressionType(v, library), v));
      n.definition.reference.values = s.values;
      n.comment = `${s.values?.length || 0} elements`;
    } else {
      const n = make(s, s.op.replace(/([a-z])([A-Z])/g, "$1 $2"), "function");
      n.definition.reference.instruction = s;
      if (expanded) for (const [key, value] of Object.entries(s)) if (value?.op) input(n, key, key, expressionType(value, library), value);
    }
  }
  // Resolve Kismet's execution stack rather than treating Pop as Return. The
  // same shared block can be reached with different continuations.
  const work = [];
  const seenStates = new Set();
  const edges = new Set();
  const wire = (from, port, into) => {
    if (!from || !into || from.id === into.id) return;
    if (!from.definition.pins.some((p) => p.id === port && p.type === "exec")) return;
    if (!into.definition.pins.some((p) => p.direction === "in" && p.type === "exec")) return;
    let p = port;
    const existing = graph.edges.find((e) => e.from.node === from.id && e.from.pin === port);
    if (existing && existing.to.node !== into.id) {
      p = `${port}-${into.id}`;
      if (!from.definition.pins.some((x) => x.id === p)) from.definition.pins.push(pin(p, `${port} · shared path`, "exec", "out"));
    }
    const key = `${from.id}:${p}>${into.id}`;
    if (edges.has(key)) return;
    edges.add(key); graph.edges.push({ id: `edge-${prefix}-${graph.edges.length}`, from: { node: from.id, pin: p }, to: { node: into.id, pin: into.definition.pins.find((x) => x.type === "exec" && x.direction === "in")?.id || "exec-in" } });
  };
  const follow = (index, stack, from, port) => {
    const skipped = new Set();
    while (index !== undefined && index >= 0 && index < body.length) {
      const key = `${index}|${stack.join(",")}`;
      if (skipped.has(key)) return;
      skipped.add(key);
      const s = body[index], n = visible.get(s.at);
      if (n) {
        wire(from, port, n);
        if (!seenStates.has(key)) { seenStates.add(key); work.push({ index, stack: [...stack] }); }
        return;
      }
      if (stack.length > 16) { diagnostics.push(`Execution stack exceeds 16 at ${fn.name}:${s.at}`); return; }
      if (s.op === "Jump") index = byAt.get(s.to);
      else if (s.op === "PushExecutionFlow") { stack = [...stack, s.to]; index += 1; }
      else if (s.op === "PopExecutionFlow") index = byAt.get(stack.at(-1)), stack = stack.slice(0, -1);
      else if (s.op === "EndOfScript") return;
      else index += 1;
    }
  };
  const starts = Array.isArray(entry) ? entry : [entry];
  for (const start of starts.filter(Boolean)) follow(start.address === undefined ? 0 : byAt.get(start.address), [], start.node, start.port || "then");
  while (work.length) {
    const { index, stack } = work.shift(), s = body[index], n = visible.get(s.at);
    if (s.op === "Return") continue;
    if (s.op === "JumpIfNot" || s.op === "PopExecutionFlowIfNot") {
      follow(index + 1, [...stack], n, "true");
      follow(s.op === "JumpIfNot" ? byAt.get(s.to) : byAt.get(stack.at(-1)), s.op === "JumpIfNot" ? [...stack] : stack.slice(0, -1), n, "false");
    } else follow(index + 1, [...stack], n, "exec-out");
  }
  for (const n of graph.nodes) for (const [id, folded] of Object.entries(n.folded || {})) {
    const input = n.definition.pins.find((p) => p.id === id);
    const candidates = producers.get(keyOf(folded.expression));
    if (folded.expression?.op !== "Variable" || candidates?.length !== 1) continue;
    const { node, port } = candidates[0], output = node.definition.pins.find((p) => p.id === port);
    if (node === n || !input || !output || !compatible(output.type, input.type) || graph.edges.some((e) => e.to.node === n.id && e.to.pin === id)) continue;
    graph.edges.push({ id: `data-${prefix}-${graph.edges.length}`, from: { node: node.id, pin: port }, to: { node: n.id, pin: id } });
  }
  return { visible, statements, input, resolve };
}

// Cooked switches are a chain of enum comparisons. Recover the case outputs
// on the originating ExpandEnumAsExecs function instead of displaying bytes.
export function compactEnumBranches(graph, library) {
  for (const call of [...graph.nodes]) {
    const sig = library.signatures?.[call.definition.signature];
    const branch = sig?.params.filter((p) => !p.return).findIndex((p) => p.name === "Branches" && p.direction === "out" && isEnum(p.type, library));
    if (branch === undefined || branch < 0) continue;
    const original = graph.edges.find((e) => e.from.node === call.id && e.from.pin === "exec-out");
    if (!original) continue;
    let current = graph.nodes.find((n) => n.id === original.to.node);
    const chain = [], cases = [];
    const enumType = sig.params.filter((p) => !p.return)[branch].type;
    let variableName;
    while (current?.definition.enumComparison === enumType && !chain.includes(current.id)) {
      const c = current.definition.reference.comparison;
      const value = c.args.find((x) => x.op === "ByteConst");
      const v = c.args.find((x) => x.op === "Variable");
      if (!value || !v || (variableName && v.name !== variableName)) break;
      variableName = v.name;
      const incoming = graph.edges.filter((e) => e.to.node === current.id && graph.nodes.find((n) => n.id === e.from.node)?.definition.pins.find((p) => p.id === e.from.pin)?.type === "exec");
      const predecessor = chain.at(-1) || call.id;
      if (incoming.some((e) => e.from.node !== predecessor)) break;
      const equalPort = /NotEqual/.test(c.name) ? "false" : "true";
      const hit = graph.edges.find((e) => e.from.node === current.id && e.from.pin === equalPort);
      if (!hit) break;
      cases.push({ value: enumValue(value, enumType, library), target: hit.to });
      chain.push(current.id);
      const next = graph.edges.find((e) => e.from.node === current.id && e.from.pin !== equalPort);
      current = next && graph.nodes.find((n) => n.id === next.to.node);
    }
    if (!cases.length || new Set(cases.map((c) => c.value)).size !== cases.length) continue;
    const removed = new Set(chain);
    if (graph.edges.some((e) => e.from.node === call.id && e.from.pin === `arg${branch}` && !removed.has(e.to.node))) continue;
    graph.nodes = graph.nodes.filter((n) => !removed.has(n.id));
    graph.edges = graph.edges.filter((e) => !removed.has(e.from.node) && !removed.has(e.to.node) && !(e.from.node === call.id && e.from.pin === "exec-out"));
    call.definition.pins = call.definition.pins.filter((p) => p.id !== `arg${branch}` && !(p.type === "exec" && p.direction === "out"));
    call.definition.expandedEnum = enumType;
    const into = current && !removed.has(current.id) && current.definition.pins.find((p) => p.type === "exec" && p.direction === "in");
    const fallback = into ? { node: current.id, pin: into.id } : null;
    for (const value of new Set([...library.enums[enumType], ...cases.map((c) => c.value)])) {
      const id = `Case_${value}`, target = cases.find((c) => c.value === value)?.target || fallback;
      call.definition.pins.push(pin(id, value, "exec", "out"));
      if (target) graph.edges.push({ id: `enum-${call.id}-${id}`, from: { node: call.id, pin: id }, to: target });
    }
  }
  return graph;
}

// Expand actual data operations and getters, rather than printing a nested
// expression in a pin. Constants remain editable values on their input pins.
export function expandDataExpressions(graph, library) {
  let sequence = 0;
  const getters = new Map();
  const connect = (source, destination, port) => {
    if (!source) return;
    graph.edges.push({ id: `expanded-edge-${graph.id}-${sequence++}`, from: source, to: { node: destination.id, pin: port } });
  };
  const fill = (n, id, name, type, expr) => {
    n.definition.pins.push(pin(id, name, type || "unknown", "in"));
    if (isConst(expr) && expr.op !== "StructConst") {
      n.values[id] = isEnum(type, library) ? enumValue(expr, type, library) : expr.op === "True" ? true : expr.op === "False" ? false : expr.op === "Self" ? "Self" : expr.value ?? "None";
    } else connect(materialize(expr, type, n.definition.reference), n, id);
  };
  const materialize = (expr, hint, origin) => {
    if (!expr) return null;
    const c = peel(expr), sig = library.signatures?.[c?.signature];
    let type = expressionType(expr, library);
    if (type === "unknown") type = hint || "unknown";
    const key = expr.op === "Variable" ? `${origin?.function}:${expr.scope}:${expr.name}:${type}` : null;
    if (key && getters.has(key)) return getters.get(key);
    const n = { id: `expanded-${graph.id}-${sequence++}`, x: 0, y: 0, title: "", comment: "", values: {}, folded: {}, showAdvanced: true, definition: { id: `reference.data.${expr.op}`, name: expr.op, class: "Reference", category: "cards", kind: "pure", pure: true, pins: [], reference: { ...origin, expression: expr, expanded: true } } };
    graph.nodes.push(n);
    if (CALLS.has(c?.op)) {
      n.definition.name = c.name; n.definition.signature = c.signature;
      const enumType = c.args?.map((a) => expressionType(a, library)).find((t) => isEnum(t, library));
      if (enumType && /Equal.*ByteByte/.test(c.name)) {
        n.definition.name = `${/NotEqual/.test(c.name) ? "Not Equal" : "Equal"} (${enumType})`;
        n.definition.enumComparison = enumType;
      }
      if (expr.op === "Context") fill(n, "target", "Target", expressionType(expr.object, library), expr.object);
      const params = sig?.params.filter((p) => !p.return) || [];
      (c.args || []).forEach((a, i) => {
        const p = params[i];
        if (p?.direction === "out") return;
        const compare = enumType && /Equal.*ByteByte/.test(c.name);
        fill(n, compare ? i === 0 ? "A" : "B" : `arg${i}`, p?.name || `Arg${i}`, compare ? enumType : p?.type || expressionType(a, library), a);
      });
    } else if (expr.op === "Variable") {
      n.definition.name = `Get ${expr.name}`; n.definition.kind = "getter";
      n.comment = expr.scope === "member" ? "Member variable" : "Local / frame variable; source binding retained";
    } else if (expr.op === "Context" && expr.value?.op === "Variable") {
      n.definition.name = `Get ${expr.value.name}`; n.definition.kind = "getter";
      fill(n, "target", "Target", expressionType(expr.object, library), expr.object);
    } else if (expr.op === "StructMemberContext") {
      n.definition.name = `Break ${expressionType(expr.object, library)} · ${expr.member?.name || "Member"}`;
      fill(n, "struct", "Struct", expressionType(expr.object, library), expr.object);
    } else if (expr.op === "ArrayGetByRef") {
      n.definition.name = "Get Array Item";
      fill(n, "array", "Array", expressionType(expr.array, library), expr.array);
      fill(n, "index", "Index", "int32", expr.index);
    } else if (expr.op === "SwitchValue") {
      const indexType = expressionType(expr.index, library);
      n.definition.name = isEnum(indexType, library) ? `Select (${indexType})` : "Select";
      fill(n, "index", "Index", indexType, expr.index);
      (expr.cases || []).forEach((item, i) => fill(n, `case${i}`, isEnum(indexType, library) ? enumValue(item.match, indexType, library) : expressionText(item.match, library), type, item.value));
      fill(n, "default", "Default", type, expr.default);
    } else if (expr.op === "StructConst") {
      n.definition.name = `Make ${expr.type}`;
      (expr.values || []).forEach((v, i) => fill(n, `member${i}`, `Member ${i}`, expressionType(v, library), v));
    } else if (["Cast", "DynamicCast"].includes(expr.op)) {
      n.definition.name = `${expr.op === "DynamicCast" ? "Cast To" : "Convert To"} ${type}`;
      fill(n, "value", "Object / Value", expressionType(expr.value, library), expr.value);
    } else {
      n.definition.name = expr.op.replace(/([a-z])([A-Z])/g, "$1 $2");
      for (const [field, value] of Object.entries(expr)) if (value?.op) fill(n, field, field, expressionType(value, library), value);
    }
    n.definition.pins.push(pin("value", "Value", type, "out"));
    const output = { node: n.id, pin: "value" };
    if (key) getters.set(key, output);
    return output;
  };
  for (const n of [...graph.nodes]) {
    n.showAdvanced = true;
    for (const [id, folded] of Object.entries(n.folded || {})) {
      if (graph.edges.some((e) => e.to.node === n.id && e.to.pin === id)) continue;
      connect(materialize(folded.expression, folded.type, n.definition.reference), n, id);
    }
  }
  return graph;
}

export function buildReferenceProject(card, library, { expanded = true } = {}) {
  const diagnostics = [];
  const eventGraph = { id: `event-${card.id}`, name: "EventGraph", kind: "event", nodes: [], edges: [], view: { x: 30, y: 40, zoom: 0.8 } };
  const project = { format: "kards-blueprint", version: 2, id: `reference-${card.id}`, name: safe(card.name), asset: { parentClass: "UBaseCardObject", packagePath: card.package.slice(0, card.package.lastIndexOf("/")), description: "Original-card reference, reconstructed by Codex" }, card: { ...card.card }, variables: [], dispatchers: [], classDefaults: {}, enums: library.enums, graphs: [eventGraph], activeGraph: eventGraph.id, reference: { ...card.provenance, originalName: card.name, package: card.package, diagnostics } };
  for (const [key, v] of Object.entries(card.defaults)) {
    if (!/^[A-Za-z_]\w*$/.test(key)) continue;
    const value = ["string", "number", "boolean"].includes(typeof v) ? v : Array.isArray(v) && v.every((x) => ["string", "number", "boolean"].includes(typeof x)) ? v : v?.SourceString || v?.LocalizedString || v?.CultureInvariantString || v?.ObjectName;
    if (value !== undefined) project.classDefaults[key] = value;
  }
  for (const p of card.variables.filter((p) => !TEMP.test(p.name))) {
    project.variables.push({ id: `var-${card.id}-${safe(p.name)}`, name: safe(p.name, "Member"), type: p.type, default: typeof card.defaults[p.name] === "boolean" || typeof card.defaults[p.name] === "number" || typeof card.defaults[p.name] === "string" ? card.defaults[p.name] : "", category: "Original", tooltip: `Original member: ${p.name}`, instanceEditable: false, exposeOnSpawn: false });
  }
  const uber = card.functions.find((f) => f.name.startsWith("ExecuteUbergraph_"));
  const wrappers = new Map();
  for (const f of card.functions) {
    const call = f.body.map(peel).find((s) => s?.name === uber?.name && s.args?.[0]?.op === "IntConst");
    if (call) wrappers.set(f.name, call.args[0].value);
  }
  const events = [];
  const makeEvent = (f) => {
    const n = { id: `event-${card.id}-${safe(f.name)}`, x: 40, y: 80 + events.length * 240, title: "", comment: "", values: {}, definition: { id: `reference.event.${f.name}`, name: f.name, class: "UBaseCardObject", category: "events", kind: "event", event: true, pins: [pin("then", "Then", "exec", "out"), ...f.params.filter((p) => p.direction === "in").map((p, i) => pin(`p${i}`, p.name, p.type, "out"))] } };
    eventGraph.nodes.push(n); return n;
  };
  for (const f of card.functions) {
    if (f === uber || /__DelegateSignature$/.test(f.name)) continue;
    const outputs = f.params.filter((p) => p.direction === "out" || p.return);
    if (wrappers.has(f.name) && !outputs.length) {
      const node = makeEvent(f);
      const params = f.params.filter((p) => p.direction === "in");
      const bindings = f.body.filter((s) => s.op === "LetValueOnPersistentFrame" && s.value?.op === "Variable").map((s) => ({ variable: s.target, pin: `p${params.findIndex((p) => p.name === s.value.name)}` })).filter((b) => b.pin !== "p-1");
      events.push({ node, address: wrappers.get(f.name), bindings });
      continue;
    }
    if (f.event && !outputs.length && !f.pure) {
      const node = makeEvent(f); events.push({ node });
      buildBody(f, eventGraph, library, `${card.id}-${safe(f.name)}`, { node, bindings: f.params.filter((p) => p.direction === "in").map((p, i) => ({ variable: { op: "Variable", name: p.name }, pin: `p${i}` })) }, null, diagnostics, expanded);
      continue;
    }
    const params = (ps) => ps.map((p, i) => ({ id: `p${i}`, name: safe(p.name, "Param"), type: p.type, sourceName: p.name }));
    const g = makeFunctionGraph(safe(f.name, "Function"), { pure: f.pure, inputs: params(f.params.filter((p) => p.direction === "in" && !p.return)), outputs: params(outputs), description: `Original function: ${f.name}` });
    g.id = `fn-${card.id}-${safe(f.name)}`;
    // Boundary ids must follow the deterministic graph id.
    g.nodes = [];
    const initialized = makeFunctionGraph(g.name, { pure: g.pure, inputs: g.inputs, outputs: g.outputs });
    initialized.id = g.id;
    for (const n of initialized.nodes) { n.definition.bp.graphId = g.id; n.definition.id = n.definition.id.replace(n.definition.id.split(".").at(-1), g.id); }
    g.nodes = initialized.nodes;
    g.sourceName = f.name;
    g.referenceLocals = f.locals;
    project.graphs.push(g);
    const entry = g.nodes.find((n) => n.definition.bp?.type === "entry");
    const result = g.nodes.find((n) => n.definition.bp?.type === "result");
    if (f.body.length) {
      if (result) g.nodes = g.nodes.filter((n) => n !== result);
      buildBody(f, g, library, `${card.id}-${safe(f.name)}`, { node: entry, bindings: g.inputs.map((p) => ({ variable: { op: "Variable", name: p.sourceName }, pin: `in:${p.id}` })) }, result, diagnostics, expanded);
    }
  }
  if (uber) {
    if (!events.some((e) => e.address !== undefined)) {
      const node = makeEvent({ name: "EventGraph", params: [] }); events.push({ node, address: uber.body.find((s) => s.op !== "ComputedJump")?.at });
    }
    buildBody(uber, eventGraph, library, `${card.id}-uber`, events.filter((e) => e.address !== undefined), null, diagnostics, expanded);
  }
  for (const g of project.graphs) {
    compactEnumBranches(g, library);
    if (expanded) expandDataExpressions(g, library);
    const entries = g.nodes.filter((n) => n.definition.kind === "event" || n.definition.bp?.type === "entry");
    const depth = new Map(entries.map((n) => [n.id, 0]));
    const todo = [...entries.map((n) => n.id)];
    while (todo.length) {
      const id = todo.shift();
      const node = g.nodes.find((n) => n.id === id);
      for (const edge of g.edges.filter((e) => e.from.node === id && node?.definition.pins.some((p) => p.id === e.from.pin && p.type === "exec"))) if (!depth.has(edge.to.node)) { depth.set(edge.to.node, depth.get(id) + 1); todo.push(edge.to.node); }
    }
    // Put data dependencies to the left of their consumers, instead of
    // stacking every getter below the event in one enormous first column.
    const rank = (n, seen = new Set()) => {
      if (depth.has(n.id)) return depth.get(n.id);
      if (seen.has(n.id) || seen.size > 24) return 0;
      const next = new Set(seen); next.add(n.id);
      const targets = g.edges.filter((e) => e.from.node === n.id).map((e) => g.nodes.find((x) => x.id === e.to.node)).filter(Boolean);
      const column = targets.length ? Math.min(...targets.map((t) => rank(t, next))) - 1 : 0;
      depth.set(n.id, column); return column;
    };
    for (const n of g.nodes) rank(n);
    const shift = -Math.min(0, ...depth.values());
    const rows = new Map();
    for (const n of g.nodes) {
      const col = (depth.get(n.id) ?? 0) + shift, row = rows.get(col) || 0;
      n.x = 50 + col * 410; n.y = 60 + row * 270; rows.set(col, row + 1);
    }
    g.reference = { sourceFunction: g.sourceName || uber?.name, instructionCount: g.kind === "event" ? uber?.body.length || 0 : card.functions.find((f) => f.name === g.sourceName)?.body.length || 0 };
  }
  if (!eventGraph.nodes.length && project.graphs.length > 1) project.activeGraph = project.graphs[1].id;
  return project;
}
