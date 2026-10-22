import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { buildReferenceProject, expressionText } from "../docs/reference-graph.js";
import { validateProject, setEnumTable, pinKind, makeProject, makeNode, connect } from "../docs/model.js";
import { expandEnumExecs, enumNodes } from "../docs/enum-tools.js";
import { syncDefinitions, buildExport } from "../docs/blueprint.js";

const read = (p) => JSON.parse(readFileSync(new URL(p, import.meta.url), "utf8"));
const library = read("../docs/reference-data/index.json");
const inventory = read("../docs/reference-data/inventory.json");

test("all 2019 original packages retain every source instruction including operands", () => {
  assert.equal(library.cards.length, 2019);
  assert.equal(new Set(library.cards.map((c) => c.id)).size, 2019);
  assert.deepEqual(inventory.normalizedOpcodes, inventory.rawOpcodes);
  assert.deepEqual(inventory.unsupported, {});
  assert.equal(library.counts.unsupportedOpcodes, 0);
  let functions = 0, statements = 0;
  for (const meta of library.cards) {
    const c = read(`../docs/reference-data/cards/${meta.id}.json`);
    const source = inventory.cards.find((e) => "/Game/" + e.file.slice(0, -5) === c.package);
    assert.ok(source, c.package);
    assert.equal(c.provenance.sha256, source.sha256);
    assert.match(source.sha256, /^[a-f0-9]{64}$/);
    assert.ok(!c.package.includes("/DIY/"));
    functions += c.functions.length;
    statements += c.functions.reduce((s, f) => s + f.body.length, 0);
  }
  assert.equal(functions, 5585);
  assert.equal(statements, 80736);
});

test("every original card reconstructs and round-trips through the real editor validator", () => {
  setEnumTable(library.enums);
  let expanded = 0, comparisons = 0, nodes = 0, dataEdges = 0;
  for (const meta of library.cards) {
    const card = read(`../docs/reference-data/cards/${meta.id}.json`);
    const p = buildReferenceProject(card, library);
    assert.equal(p.graphs[0].name, "EventGraph");
    assert.equal(p.card.title, card.card.title);
    assert.deepEqual(p.reference.diagnostics, [], meta.name);
    for (const g of p.graphs) {
      for (const edge of g.edges) {
        const type = g.nodes.find((n) => n.id === edge.from.node)?.definition.pins.find((p) => p.id === edge.from.pin)?.type;
        if (type !== "exec") dataEdges += 1;
      }
      for (const n of g.nodes) {
      nodes += 1;
      if (n.definition.expandedEnum) {
        expanded += 1;
        for (const value of library.enums[n.definition.expandedEnum]) assert.ok(n.definition.pins.some((p) => p.id === `Case_${value}`));
        assert.ok(!n.definition.pins.some((p) => p.id === "Branches"));
      }
      if (n.definition.enumComparison) {
        comparisons += 1;
        assert.ok(library.enums[n.definition.enumComparison]);
        assert.equal(n.definition.pins.find((p) => p.id === "A").type, n.definition.enumComparison);
        assert.equal(n.definition.pins.find((p) => p.id === "B").type, n.definition.enumComparison);
        assert.ok(!n.definition.name.includes("ByteByte"));
      }
    }
    }
    assert.doesNotThrow(() => validateProject(structuredClone(p)), meta.name);
    const exported = buildExport(p, { enums: library.enums });
    assert.deepEqual(exported.enums, library.enums);
    assert.equal(exported.reference.sha256, card.provenance.sha256);
    assert.doesNotThrow(() => validateProject(exported), meta.name + " exported");
  }
  assert.ok(nodes > 20000);
  assert.ok(expanded > 200);
  assert.ok(comparisons > 100);
  assert.ok(dataEdges > 1000);
});

test("enum comparisons show enum names, not raw bytes", () => {
  assert.equal(expressionText({ op: "CallMath", name: "EqualEqual_ByteByte", args: [{ op: "Variable", name: "faction", type: "EFactionEnum", scope: "member" }, { op: "ByteConst", value: 1 }] }, library), "Self.faction == EFactionEnum::Germany");
  setEnumTable(library.enums);
  assert.equal(pinKind("CardMoveReason"), "enum");
  assert.ok(library.enumValues.CardMoveReason["12"]);
  assert.notEqual(library.enumValues.CardMoveReason["12"], "NewEnumerator12");
});

test("Select(Enum) infers selection and value types and compares only matching enums", () => {
  setEnumTable(library.enums);
  const p = makeProject(), g = p.graphs[0];
  const select = makeNode(enumNodes.find((d) => d.selectEnum), 0, 0);
  select.definition.enumType = "EFactionEnum";
  g.nodes.push(select, { id: "value", definition: { name: "value", category: "variables", pins: [{ id: "out", name: "Out", type: "int32", direction: "out" }] }, x: 0, y: 0, values: {} });
  syncDefinitions(p);
  assert.equal(select.definition.pins.filter((p) => p.id.startsWith("Value_")).length, library.enums.EFactionEnum.length);
  connect(g, { node: "value", pin: "out" }, { node: select.id, pin: "Value_Germany" });
  syncDefinitions(p);
  assert.equal(select.definition.pins.find((p) => p.id === "Result").type, "int32");
  const compare = makeNode(enumNodes.find((d) => d.enumCompare), 0, 0);
  compare.definition.enumType = "EFactionEnum"; g.nodes.push(compare); syncDefinitions(p);
  g.nodes.push({ id: "side", x: 0, y: 0, values: {}, definition: { name: "side", category: "variables", pins: [{ id: "out", name: "Out", type: "ESideEnum", direction: "out" }] } });
  assert.throws(() => connect(g, { node: "side", pin: "out" }, { node: compare.id, pin: "A" }), /类型/);
});

test("enum execution expansion preserves other data outputs and ignores ordinary enum inputs", () => {
  const d = { name: "Choose", pure: false, pins: [{ id: "exec-in", type: "exec", direction: "in" }, { id: "exec-out", type: "exec", direction: "out" }, { id: "Branches", name: "Branches", type: "EFactionEnum", direction: "out" }, { id: "count", name: "Count", type: "int32", direction: "out" }] };
  const expanded = expandEnumExecs(d, library.enums);
  assert.ok(expanded.pins.some((p) => p.id === "count"));
  assert.ok(expanded.pins.some((p) => p.id === "Case_Germany"));
  assert.ok(!expanded.pins.some((p) => p.id === "exec-out"));
  assert.equal(expandEnumExecs({ ...d, pins: [{ id: "Branches", type: "EFactionEnum", direction: "in" }] }, library.enums).expandedEnum, undefined);
});

test("invalid imported enum declarations are rejected instead of silently changing types", () => {
  const p = makeProject(); p.enums = { Bad: ["One", "One"] };
  assert.throws(() => validateProject(p), /枚举/);
});
