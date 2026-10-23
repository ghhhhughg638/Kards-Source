// Shared node capabilities and UE-derived presentation. By Codex.
import { ueFunctionMetadata, ueEditorColors as colors } from "./ue-editor-metadata.js";

const aliases = { AND: "BooleanAND", OR: "BooleanOR", NAND: "BooleanNAND", NOR: "BooleanNOR", XOR: "BooleanXOR", NOT: "Not_PreBool", AppendString: "Concat_StrStr", Min_int32: "Min", Max_int32: "Max", Min_float: "FMin", Max_float: "FMax" };
const functionName = (d) => d.signature?.split(".").at(-1) || d.name;
export function functionMetadata(d) {
  if (d.bp || d.event || d.signature?.includes(".") && !/^U?Kismet(Math|String)Library\./.test(d.signature)) return {};
  let name = functionName(d);
  name = aliases[name] || name;
  name = name.replace(/_(int32|float)$/, (_, t) => t === "int32" ? "_IntInt" : "_FloatFloat");
  return ueFunctionMetadata[name] || {};
}
const dataInputs = (d) => d.pins.filter((p) => p.direction === "in" && p.type !== "exec" && p.id !== "target" && p.name !== "self");

export function pinColor(type, enums = {}) {
  type = String(type).replace(/\bclass\s+/g, "").trim();
  const inner = /^T(?:Array|Set|Map)\s*<(.+)>$/.exec(type);
  if (inner) return pinColor(inner[1].split(",")[0].trim(), enums);
  let name;
  if (type === "exec") name = "Execution";
  else if (type.includes("$") || type === "unknown") name = "Wildcard";
  else if (type === "bool") name = "Boolean";
  else if (enums[type] || /^E[A-Z]|^Enum/.test(type) || type === "CardMoveReason" || /^(u?int8|byte)$/.test(type)) name = "Byte";
  else if (/^(u?int64)$/.test(type)) name = "Int64";
  else if (/^(u?int\d*)$/.test(type)) name = "Int";
  else if (type === "float") name = "Float";
  else if (type === "double") name = "Double";
  else if (type === "FName") name = "Name";
  else if (type === "FText") name = "Text";
  else if (type === "FString") name = "String";
  else if (/^FVector/.test(type)) name = "Vector";
  else if (type === "FRotator") name = "Rotator";
  else if (type === "FTransform") name = "Transform";
  else if (/Delegate</.test(type)) name = "Delegate";
  else if (/TSoftClassPtr</.test(type)) name = "SoftClass";
  else if (/TSoftObjectPtr</.test(type)) name = "SoftObject";
  else if (/TSubclassOf<|^UClass\*/.test(type)) name = "Class";
  else if (/TScriptInterface</.test(type)) name = "Interface";
  else if (/^F[A-Z]/.test(type)) name = "Struct";
  else name = "Object";
  return colors[`${name}PinTypeColor`] || colors.DefaultPinTypeColor;
}

export function nodeAppearance(d, enums = {}) {
  const meta = functionMetadata(d), kind = d.kind || (d.event ? "event" : d.pure ? "pure" : "function");
  // AND / OR / compare / arithmetic nodes keep the same header and pin rows as every other node,
  // just without the subtitle, footer and wide "add pin" button, so they stay small.
  const compact = false, slim = !!meta.compactTitle && !d.pins.some((p) => p.type === "exec");
  let color = colors.FunctionCallNodeTitleColor;
  if (/event/i.test(kind)) color = colors.EventNodeTitleColor;
  else if (/Entry|Result|Exit/.test(kind)) color = colors.FunctionTerminatorNodeTitleColor;
  else if (["variable", "getter", "setter"].includes(kind)) color = pinColor(d.pins.find((p) => p.type !== "exec" && p.id !== "target")?.type, enums);
  else if (/^Sequence/.test(d.name)) color = colors.ExecSequenceNodeTitleColor;
  else if (d.name === "Branch") color = colors.ExecBranchNodeTitleColor;
  else if (kind === "flow" || kind === "macro") color = "#a0a0a0";
  else if (d.pure || kind === "pure") color = colors.PureFunctionCallNodeTitleColor;
  else if (kind === "cast" || kind === "conversion") color = "#4bb8bd";
  return { compact, slim, title: meta.compactTitle || d.name, color };
}

export function dynamicPinSpec(d) {
  if (d.bp || d.event) return null;
  const name = functionName(d);
  if (functionMetadata(d).associative) return { kind: "operands", min: 2, pins: dataInputs(d) };
  if (d.class !== "UE" && !/^(ue|studio)\./.test(d.id || "")) return null;
  if (/^(Sequence|Sequence3|Sequence4|MultiGate)$/.test(name)) return { kind: "exec", min: 1, pins: d.pins.filter((p) => p.direction === "out" && p.type === "exec") };
  if (name === "MakeArray") return { kind: "array", min: 0, pins: dataInputs(d) };
  if (name === "SelectIndex") return { kind: "select", min: 2, pins: dataInputs(d).filter((p) => p.id !== "Index") };
  if (name === "SwitchOnInt") return { kind: "integerCases", min: 0, pins: d.pins.filter((p) => p.direction === "out" && p.type === "exec" && p.name !== "Default") };
  if (/^SwitchOn(String|Name)$/.test(name)) return { kind: "namedCases", min: 0, pins: d.pins.filter((p) => p.direction === "out" && p.type === "exec" && p.id !== "Default") };
  return null;
}

export function canRemoveDynamicPin(d, id) {
  const spec = dynamicPinSpec(d);
  return !!spec && spec.pins.length > spec.min && spec.pins.some((p) => p.id === id);
}

export function addDynamicPin(graph, node, enums = {}) {
  const d = node.definition, spec = dynamicPinSpec(d);
  if (!spec || d.pins.length >= 126 || spec.kind === "operands" && spec.pins.length >= 26) return false;
  d.editorNodeClass = ({ operands: "K2Node_CommutativeAssociativeBinaryOperator", array: "K2Node_MakeArray", select: "K2Node_Select", integerCases: "K2Node_SwitchInteger", namedCases: d.name === "SwitchOnName" ? "K2Node_SwitchName" : "K2Node_SwitchString", exec: d.name === "MultiGate" ? "K2Node_MultiGate" : "K2Node_ExecutionSequence" })[spec.kind];
  const next = (prefix) => { let i = 0; while (d.pins.some((p) => p.id === `${prefix}${i}`)) i++; return `${prefix}${i}`; };
  const append = (p, template = p) => {
    d.pins.push(p);
    if (Array.isArray(d.wild)) d.wild.push({ ...template });
    if (p.direction === "in") node.values[p.id] = enums[p.type]?.[0] ?? (p.type === "bool" ? false : /^(u?int|float|double)/.test(p.type) ? 0 : p.type.endsWith("*") || /^T(SoftObjectPtr|SoftClassPtr|SubclassOf)</.test(p.type) ? "None" : "");
  };
  if (spec.kind === "namedCases") {
    let i = 0; while (d.pins.some((p) => p.id === `Out${i}` || p.id === `Case${i}`)) i++;
    const type = d.pins.find((p) => p.id === "Selection").type;
    append({ id: `Case${i}`, name: `Case ${i}`, type, direction: "in" });
    node.values[`Case${i}`] = `Case${i}`;
    append({ id: `Out${i}`, name: `Out ${i}`, type: "exec", direction: "out" });
  } else if (spec.kind === "exec") {
    const prefix = d.name === "MultiGate" ? "Out" : "Then", id = next(prefix);
    append({ id, name: id.replace(/(\D+)(\d+)/, "$1 $2"), type: "exec", direction: "out" });
  } else if (spec.kind === "integerCases") {
    let i = 0; while (d.pins.some((p) => p.id === String(i))) i++;
    append({ id: String(i), name: String(i), type: "exec", direction: "out" });
  } else {
    const first = spec.pins[0], template = d.wild?.find((p) => p.id === first?.id);
    const type = first?.type || d.pins.find((p) => p.direction === "out")?.type.replace(/^TArray<(.+)>$/, "$1") || "$T";
    const id = next("UserPin");
    const name = spec.kind === "operands" ? String.fromCharCode(65 + spec.pins.length) : `[${spec.pins.length}]`;
    append({ id, name, type, direction: "in" }, { id, name, type: template?.type || (spec.kind === "array" || spec.kind === "select" ? "$T" : type), direction: "in" });
  }
  d.dynamicPinOrder = dynamicPinSpec(d).pins.map((p) => p.id);
  return true;
}

export function removeDynamicPin(graph, node, id) {
  const d = node.definition;
  if (!canRemoveDynamicPin(d, id)) return false;
  const removed = new Set([id]);
  if (dynamicPinSpec(d).kind === "namedCases") removed.add(id.replace(/^Out/, "Case"));
  d.pins = d.pins.filter((p) => !removed.has(p.id));
  if (d.wild) d.wild = d.wild.filter((p) => !removed.has(p.id));
  graph.edges = graph.edges.filter((e) => !(e.from.node === node.id && removed.has(e.from.pin) || e.to.node === node.id && removed.has(e.to.pin)));
  for (const pin of removed) { delete node.values[pin]; if (node.folded) delete node.folded[pin]; }
  // Keep identifiers stable so other wires survive; only display labels change.
  const spec = dynamicPinSpec(d);
  if (spec.kind === "operands" || spec.kind === "array" || spec.kind === "select") spec.pins.forEach((p, i) => {
    p.name = spec.kind === "operands" ? String.fromCharCode(65 + i) : `[${i}]`;
    const template = d.wild?.find((x) => x.id === p.id); if (template) template.name = p.name;
  });
  d.dynamicPinOrder = dynamicPinSpec(d).pins.map((p) => p.id);
  return true;
}

export function applyUEPalette(root) {
  for (const [name, type] of Object.entries({exec: "exec", object: "UObject*", int: "int32", bool: "bool", string: "FString", enum: "uint8", float: "float", name: "FName", text: "FText", vector: "FVector", rotator: "FRotator", delegate: "Delegate<void>", wildcard: "$T"})) { root.style.setProperty(`--pin-${name}`, pinColor(type)); if (name !== "text") root.style.setProperty(`--${name}`, pinColor(type)); }
}
