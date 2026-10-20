import test from "node:test";
import assert from "node:assert/strict";
import { connect, setEnumTable, makeProject, compatible } from "../docs/model.js";
import { syncDefinitions } from "../docs/blueprint.js";
import { ueBuiltins } from "../docs/ue-nodes.js";

const enums = { EFactionEnum: ["Germany", "Soviet"], ESideEnum: ["left", "right"] };
const setup = () => {
  setEnumTable(enums);
  const p = makeProject(),
    g = p.graphs[0];
  g.nodes.push({ id: "s", definition: structuredClone(ueBuiltins.find((d) => d.switchEnum)), x: 0, y: 0, values: {} });
  g.nodes.push({
    id: "f",
    definition: { id: "x", name: "Get faction", class: "UBaseCardObject", pins: [{ id: "value", name: "faction", type: "EFactionEnum", direction: "out" }] },
    x: 0,
    y: 0,
    values: {},
  });
  return { p, g, sw: g.nodes.find((n) => n.id === "s") };
};

test("there is exactly one Switch on Enum node", () => {
  assert.equal(ueBuiltins.filter((d) => d.switchEnum).length, 1);
});

test("Switch on Enum has no cases until an enum is chosen, then one exec output per value", () => {
  const { p, sw } = setup();
  syncDefinitions(p);
  assert.deepEqual(sw.definition.pins.map((x) => x.id), ["exec-in", "Selection"]);
  sw.definition.enumType = "ESideEnum";
  syncDefinitions(p);
  assert.deepEqual(sw.definition.pins.map((x) => x.id), ["exec-in", "Selection", "Case_left", "Case_right"]);
});

test("wiring an enum into Selection picks that enum; non-enums are refused", () => {
  const { p, g, sw } = setup();
  connect(g, { node: "f", pin: "value" }, { node: "s", pin: "Selection" });
  syncDefinitions(p);
  assert.equal(sw.definition.enumType, "EFactionEnum");
  assert.equal(g.edges.length, 1);
  assert.deepEqual(sw.definition.pins.filter((x) => x.type === "exec" && x.direction === "out").map((x) => x.id), ["Case_Germany", "Case_Soviet"]);
  assert.equal(compatible("int32", "$E"), false);
});
