// Shared enum presentation for the editor and reconstructed reference graphs.
export function expandEnumExecs(definition, enums) {
  const out = { ...definition };
  const explicit = definition.expandEnumAsExecs;
  const parameter = definition.pins.find((p) => p.id === (explicit || "Branches") && enums[p.type]);
  if (!parameter || parameter.direction !== "out") return out;
  out.pins = definition.pins.filter((p) => p.id !== parameter.id && !(p.type === "exec" && p.direction === "out"));
  if (!out.pins.some((p) => p.type === "exec" && p.direction === "in")) out.pins.unshift({ id: "exec-in", name: "Exec", type: "exec", direction: "in" });
  for (const v of enums[parameter.type]) out.pins.push({ id: `Case_${v}`, name: v, type: "exec", direction: "out" });
  return { ...out, pure: false, kind: "function", expandedEnum: parameter.type };
}

export const enumNodes = [
  { id: "studio.SelectEnum", name: "Select (Enum)", label: "枚举选择 · Select", class: "UE", category: "flow", kind: "pure", pure: true, selectEnum: true, pins: [{ id: "Index", name: "Index", type: "$E", direction: "in" }, { id: "Result", name: "Result", type: "$T", direction: "out" }] },
  ...["Equal", "Not Equal"].map((name) => ({ id: `studio.${name.replace(/ /g, "")}Enum`, name: `${name} (Enum)`, label: `${name === "Equal" ? "枚举相等" : "枚举不等"} · ${name}`, class: "UE", category: "math", kind: "pure", pure: true, enumCompare: true, pins: [{ id: "A", name: "A", type: "$E", direction: "in" }, { id: "B", name: "B", type: "$E", direction: "in" }, { id: "Result", name: "Result", type: "bool", direction: "out" }] })),
];
