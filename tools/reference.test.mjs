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

const blockade = () => {
  const meta = library.cards.find((c) => c.name === "card_event_blockade");
  setEnumTable(library.enums);
  return buildReferenceProject({ ...read(`../docs/reference-data/cards/${meta.id}.json`), translation: {} }, library, { expanded: true });
};

test("library calls such as Array_Get show no Target pin for Default__KismetArrayLibrary", () => {
  const nodes = blockade().graphs.flatMap((g) => g.nodes).filter((n) => /^Array_/.test(n.definition.name));
  assert.ok(nodes.length > 0);
  for (const n of nodes) assert.ok(!n.definition.pins.some((p) => p.id === "target"), n.definition.name);
});

const build = (name) => {
  const meta = library.cards.find((c) => c.name === name);
  setEnumTable(library.enums);
  return buildReferenceProject({ ...read(`../docs/reference-data/cards/${meta.id}.json`), translation: {} }, library, { expanded: true });
};
const loopNodes = (project) => project.graphs.flatMap((g) => g.nodes.map((n) => ({ g, n }))).filter(({ n }) => /^ue\.ForEachLoop/.test(n.definition.id));

test("the ForEachLoop macro expansion becomes a For Each Loop node instead of a dozen counter nodes", () => {
  const project = build("card_event_the_double_sunrise");
  const loops = loopNodes(project);
  assert.equal(loops.length, 2);
  const g = project.graphs.find((x) => x.kind === "event");
  for (const { n } of loops) {
    const linked = (pin, dir) => g.edges.filter((e) => e[dir].node === n.id && e[dir].pin === pin).length;
    assert.equal(linked("Array", "to"), 1, "array in");
    assert.equal(linked("LoopBody", "from"), 1, "loop body");
    assert.equal(linked("Completed", "from"), 1, "completed");
    assert.ok(linked("ArrayElement", "from") >= 1, "element feeds the body");
    assert.equal(n.definition.pins.find((p) => p.id === "Array").type, "TArray<UBaseCardObject*>");
  }
  const names = g.nodes.map((n) => n.definition.name);
  assert.ok(!names.some((x) => /^(Set|Get) Temp_int_(Loop_Counter|Array_Index)_Variable/.test(x)), "no loop counter/index nodes left");
  assert.ok(!names.includes("Less_IntInt") && !names.includes("Add_IntInt"));
  assert.ok(g.nodes.length < 45, `expected a short graph, got ${g.nodes.length} nodes`);
  validateProject(structuredClone(project));
});

test("a ForEachLoopWithBreak expansion turns the 'break' assignment into the Break pin", () => {
  const project = build("card_unit_17th_infantry_brigade");
  const found = loopNodes(project).find(({ n }) => n.definition.id === "ue.ForEachLoopWithBreak");
  assert.ok(found, "loop with break");
  const { g, n } = found, by = (id) => g.nodes.find((x) => x.id === id);
  const breakEdge = g.edges.find((e) => e.to.node === n.id && e.to.pin === "Break");
  assert.ok(breakEdge, "something leads into Break");
  assert.match(by(breakEdge.from.node).definition.name, /^Set /);
  assert.ok(!g.nodes.some((x) => /Temp_bool_True_if_break_was_hit/.test(x.definition.name)));
  validateProject(structuredClone(project));
});

test("a literal array feeding a loop becomes one Make Array node, and a re-fetched call result becomes one call", () => {
  const corps = build("card_event_corps_command");
  const g = corps.graphs.find((x) => (x.sourceName || x.name) === "_DoOnCardPlayedOrGotchaTriggered");
  const make = g.nodes.filter((n) => n.definition.id === "ue.MakeArray");
  assert.equal(make.length, 1);
  assert.deepEqual([make[0].values.Item0, make[0].values.Item1], ["left_most", "right_most"]);
  assert.ok(!g.nodes.some((n) => n.definition.name === "Make / Set Array" || /K2Node_MakeArray_Array/.test(n.definition.name)));
  const loop = g.nodes.find((n) => n.definition.id === "ue.ForEachLoop");
  assert.ok(g.edges.some((e) => e.from.node === make[0].id && e.to.node === loop.id && e.to.pin === "Array"));
  validateProject(structuredClone(corps));

  const howitzer = build("card_unit_3_7_inch_mountain_howitzer").graphs.find((x) => (x.sourceName || x.name) === "EventGraph");
  assert.equal(howitzer.nodes.filter((n) => n.definition.name === "GetCardsToTheLeft").length, 1, "the call is not repeated per iteration");
  assert.equal(howitzer.nodes.filter((n) => n.definition.id === "ue.ForEachLoop").length, 2);
  assert.ok(!howitzer.nodes.some((n) => /Temp_int_Loop_Counter/.test(n.definition.name)));
  validateProject(structuredClone(build("card_unit_3_7_inch_mountain_howitzer")));
});

test("a counted loop becomes a For Loop and constant temporaries become pin values", () => {
  const anzac = build("card_event_anzac_spirit").graphs.find((x) => x.kind === "event");
  const loop = anzac.nodes.find((n) => n.definition.id === "ue.ForLoop");
  assert.ok(loop, "For Loop node");
  assert.equal(String(loop.values.FirstIndex), "0");
  assert.equal(String(loop.values.LastIndex), "1");
  assert.ok(anzac.edges.some((e) => e.from.node === loop.id && e.from.pin === "LoopBody"));
  assert.ok(!anzac.nodes.some((n) => /Temp_int_Variable/.test(n.definition.name) && n.definition.kind === "getter"));
  validateProject(structuredClone(build("card_event_anzac_spirit")));

  const espionage = build("card_event_espionage_skirm").graphs.find((x) => x.kind === "event");
  assert.ok(!espionage.nodes.some((n) => n.definition.name === "Set Temp_text_Variable"));
  const note = espionage.nodes.find((n) => n.definition.name === "ShowNotification");
  assert.equal(note.values.arg1, "No cards in enemy hand.");
});

test("a switch on int comes back as one Switch node and a Select reads its condition directly", () => {
  const g = build("card_event_foreign_support_skirm").graphs.find((x) => (x.sourceName || x.name) === "RandomChooseCards");
  const sw = g.nodes.find((n) => n.definition.id === "ue.SwitchOnInt");
  assert.ok(sw, "Switch on Int");
  assert.deepEqual(sw.definition.pins.filter((p) => p.type === "exec" && p.direction === "out").map((p) => p.id), ["1", "2", "3", "Default"]);
  assert.ok(!g.nodes.some((n) => /K2Node_SwitchInteger/.test(n.definition.name)));
  assert.equal(g.edges.filter((e) => e.from.node === sw.id).length, 3, "three cases are wired");
  const scen = build("card_location_british_scen1").graphs.find((x) => x.kind === "event");
  const select = scen.nodes.find((n) => n.definition.name === "Select");
  const index = scen.edges.find((e) => e.to.node === select.id && e.to.pin === "index");
  assert.equal(scen.nodes.find((n) => n.id === index.from.node).definition.name, "Greater_IntInt");
  assert.ok(!scen.nodes.some((n) => /^Get K2Node_Select_Default|^Get Temp_bool_Variable/.test(n.definition.name)));
});

test("a compiled dynamic cast becomes one Cast node with Success and Failed outputs", () => {
  const project = build("card_location_british_scen2");
  const g = project.graphs.find((x) => x.kind === "event" && x.nodes.some((n) => /^Cast To /.test(n.definition.name)));
  const cast = g.nodes.find((n) => /^Cast To /.test(n.definition.name));
  assert.ok(cast.definition.pins.some((p) => p.id === "Success" && p.type === "exec"));
  assert.ok(g.edges.some((e) => e.from.node === cast.id && e.from.pin === "Success"));
  assert.ok(!g.nodes.some((n) => /K2Node_DynamicCast/.test(n.definition.name)));
  validateProject(structuredClone(project));
});

test("call outputs and event parameters are wired from their producing pin, not through a detached getter", () => {
  const g = build("card_event_forward_base_anzac").graphs.find((x) => x.kind === "event");
  assert.ok(!g.nodes.some((n) => /^Get K2Node_Event_/.test(n.definition.name)), "no detached event-parameter getters");
  const event = g.nodes.find((n) => n.definition.name === "OnPlayedFromHand");
  assert.ok(g.edges.some((e) => e.from.node === event.id && e.from.pin === "p0"), "the event's parameter pin feeds its readers");
  const blockade = build("card_event_blockade").graphs.find((x) => x.kind === "event");
  assert.ok(!blockade.nodes.some((n) => /^Get CallFunc_/.test(n.definition.name)), "no detached call-output getters");
});

test("loop collapsing never leaves the graph with dangling or mistyped wires", () => {
  setEnumTable(library.enums);
  for (const name of ["card_event_the_double_sunrise", "card_unit_17th_infantry_brigade", "card_event_blockade", "card_event_mass_deployment", "card_location_british_scen3", "card_event_vanguard_ita", "card_event_corps_command", "card_event_support_column", "card_unit_3_7_inch_mountain_howitzer", "card_event_defend_the_empire"]) {
    const project = build(name);
    for (const g of project.graphs) {
      const ids = new Set(g.nodes.map((n) => n.id));
      for (const e of g.edges) assert.ok(ids.has(e.from.node) && ids.has(e.to.node), `${name}: wire to a removed node`);
    }
    validateProject(structuredClone(project));
  }
});
