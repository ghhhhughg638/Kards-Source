import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import {
  clone,
  connect,
  makeNode,
  makeProject,
  validateProject,
  builtins,
  setTypeHierarchy,
} from "../docs/model.js";
import {
  buildExport,
  callDefinition,
  checkName,
  customEventDefinition,
  dynamicDefinitions,
  getDefinition,
  makeFunctionGraph,
  makeMacroGraph,
  makeVariable,
  newParam,
  overridableFunctions,
  overrideSignature,
  setDefinition,
  syncDefinitions,
} from "../docs/blueprint.js";
import { engineClasses } from "../docs/ue-nodes.js";
const catalog = JSON.parse(
  await readFile(new URL("../docs/catalog.json", import.meta.url), "utf8"),
);
setTypeHierarchy({ ...engineClasses, ...catalog.classes });

test("names must be English identifiers", () => {
  assert.equal(checkName("GetDamage"), null);
  assert.equal(checkName("_Private1"), null);
  assert.match(checkName("获取伤害"), /不支持中文/);
  assert.match(checkName("1Bad"), /不能以数字开头/);
  assert.match(checkName("has space"), /英文字母/);
  assert.match(checkName("Self"), /保留/);
  assert.match(checkName("health", ["Health"]), /已被使用/);
});

test("a new project has exactly one fixed EventGraph and no functions", () => {
  const p = makeProject();
  assert.equal(p.version, 2);
  assert.equal(p.graphs.length, 1);
  assert.equal(p.graphs[0].kind, "event");
  assert.equal(p.graphs[0].name, "EventGraph");
  assert.equal(p.asset.parentClass, "UBaseCardObject");
  validateProject(clone(p));
});

test("functions get entry and result nodes that follow the signature", () => {
  const p = makeProject();
  const fn = makeFunctionGraph("ComputeDamage", {
    inputs: [newParam("Base", "int32")],
    outputs: [newParam("Total", "int32")],
  });
  p.graphs.push(fn);
  const entry = fn.nodes.find((n) => n.definition.kind === "functionEntry");
  const result = fn.nodes.find((n) => n.definition.kind === "functionResult");
  assert(entry && result);
  assert(entry.definition.pins.some((x) => x.name === "Base" && x.direction === "out"));
  assert(result.definition.pins.some((x) => x.name === "Total" && x.direction === "in"));
  connect(fn, { node: entry.id, pin: "then" }, { node: result.id, pin: "exec" });
  connect(
    fn,
    { node: entry.id, pin: `in:${fn.inputs[0].id}` },
    { node: result.id, pin: `out:${fn.outputs[0].id}` },
  );
  validateProject(clone(p));
});

test("editing a signature updates call nodes and drops wires to removed pins", () => {
  const p = makeProject();
  const fn = makeFunctionGraph("Heal", { inputs: [newParam("Amount", "int32")] });
  p.graphs.push(fn);
  const call = makeNode(callDefinition(fn), 100, 100);
  const other = makeNode(builtins.find((b) => b.name === "Integer"), 0, 0);
  p.graphs[0].nodes.push(call, other);
  connect(
    p.graphs[0],
    { node: other.id, pin: "Value" },
    { node: call.id, pin: `in:${fn.inputs[0].id}` },
  );
  fn.inputs[0].name = "Points";
  syncDefinitions(p);
  assert(call.definition.pins.some((x) => x.name === "Points"));
  assert.equal(p.graphs[0].edges.length, 1);
  fn.inputs = [];
  syncDefinitions(p);
  assert.equal(p.graphs[0].edges.length, 0);
});

test("deleting a variable removes its get/set nodes; renaming keeps wires", () => {
  const p = makeProject();
  const v = makeVariable("Health", "int32");
  p.variables.push(v);
  const get = makeNode(getDefinition(v), 0, 0),
    set = makeNode(setDefinition(v), 200, 0);
  p.graphs[0].nodes.push(get, set);
  connect(p.graphs[0], { node: get.id, pin: "value" }, { node: set.id, pin: "value" });
  v.name = "Armor";
  syncDefinitions(p);
  assert.equal(get.definition.pins[0].name, "Armor");
  assert.equal(p.graphs[0].edges.length, 1);
  p.variables = [];
  syncDefinitions(p);
  assert.equal(p.graphs[0].nodes.length, 0);
  assert.equal(p.graphs[0].edges.length, 0);
});

test("custom events live only in the EventGraph and can be called there", () => {
  const p = makeProject();
  const event = makeNode(
    customEventDefinition("Pulse", [newParam("Strength", "float")]),
    0,
    0,
  );
  p.graphs[0].nodes.push(event);
  validateProject(clone(p));
  const defs = dynamicDefinitions(p, p.graphs[0]);
  assert(defs.some((d) => d.bp?.type === "callEvent" && d.name === "Pulse"));
  const fn = makeFunctionGraph("F");
  p.graphs.push(fn);
  assert(!dynamicDefinitions(p, fn).some((d) => d.bp?.type === "callEvent"));
  fn.nodes.push(clone(event));
  assert.throws(() => validateProject(clone(p)), /EventGraph/);
});

test("macros keep exec pins and cannot instantiate themselves", () => {
  const p = makeProject();
  const macro = makeMacroGraph("Twice", {
    inputs: [newParam("In", "exec")],
    outputs: [newParam("Out", "exec")],
  });
  p.graphs.push(macro);
  const entry = macro.nodes.find((n) => n.definition.kind === "macroEntry");
  assert(entry.definition.pins.some((x) => x.type === "exec" && x.direction === "out"));
  const own = dynamicDefinitions(p, macro).filter((d) => d.bp?.type === "macro");
  assert.equal(own.length, 0);
  assert.equal(
    dynamicDefinitions(p, p.graphs[0]).filter((d) => d.bp?.type === "macro").length,
    1,
  );
});

test("parent-class overrides come from BlueprintNativeEvent/ImplementableEvent functions", () => {
  const list = overridableFunctions(catalog.nodes);
  assert(list.length > 5);
  assert(list.every((d) => d.class === "UBaseCardObject" && !d.event));
  const sig = overrideSignature(list[0]);
  assert(sig.inputs.every((x) => x.name !== "target"));
});

test("import rejects version 1 files and invalid names", () => {
  const p = makeProject();
  const old = clone(p);
  old.version = 1;
  assert.throws(() => validateProject(old), /版本 2/);
  const badName = clone(p);
  badName.name = "我的卡牌";
  assert.throws(() => validateProject(badName), /资源名称/);
  const badFn = clone(p);
  badFn.graphs.push(makeFunctionGraph("函数一"));
  assert.throws(() => validateProject(badFn), /名称/);
  const twoEvents = clone(p);
  twoEvents.graphs.push({ ...clone(p.graphs[0]), id: "x" });
  assert.throws(() => validateProject(twoEvents), /EventGraph|函数或宏/);
});

test("export is comprehensive and imports back unchanged", () => {
  const p = makeProject();
  p.name = "BP_Test";
  const v = makeVariable("Health", "int32");
  p.variables.push(v);
  const fn = makeFunctionGraph("Heal", { inputs: [newParam("Amount", "int32")] });
  p.graphs.push(fn);
  const native = catalog.nodes.find((n) => n.name === "GiveBlitz");
  p.graphs[0].nodes.push(makeNode(native, 40, 40), makeNode(getDefinition(v), 0, 200));
  const out = buildExport(p, {
    classes: { ...engineClasses, ...catalog.classes },
    enums: catalog.enums,
    repository: catalog.repository,
    exportedAt: "2026-01-01T00:00:00.000Z",
  });
  assert.equal(out.asset.objectPath, "/Game/KARDS/Cards/BP_Test.BP_Test");
  assert.equal(out.asset.generatedClass, "BP_Test_C");
  assert.deepEqual(out.asset.parent.hierarchy.slice(0, 2), ["UBaseCardObject", "UPrimaryDataAsset"]);
  assert.equal(out.variables[0].name, "Health");
  assert.equal(out.functions[0].name, "Heal");
  assert.equal(out.eventGraph, p.graphs[0].id);
  assert(out.dependencies.nativeFunctions.some((f) => f.name === "GiveBlitz" && f.source));
  assert(out.dependencies.nativeClasses.some((c) => c.name === "UBaseCardObject"));
  assert.equal(out.stats.variables, 1);
  assert.equal(out.exporter.engineVersion, "5.6");
  const back = validateProject(JSON.parse(JSON.stringify(out)));
  assert.equal(back.graphs.length, 2);
  assert.equal(back.variables[0].id, v.id);
  assert.equal(back.graphs[1].inputs[0].name, "Amount");
});

test("local variables belong to one function and are removed with it", () => {
  const p = makeProject();
  const fn = makeFunctionGraph("Calc", {
    inputs: [newParam("Base", "int32")],
    locals: [{ name: "Temp", type: "int32" }],
  });
  const other = makeFunctionGraph("Other");
  p.graphs.push(fn, other);
  const defs = dynamicDefinitions(p, fn);
  assert(defs.some((d) => d.bp?.type === "getLocal" && d.property === "Temp"));
  assert(defs.some((d) => d.bp?.type === "setLocal"));
  assert(!dynamicDefinitions(p, other).some((d) => d.bp?.type === "getLocal"));
  assert(!dynamicDefinitions(p, p.graphs[0]).some((d) => d.bp?.type === "getLocal"));
  const get = makeNode(defs.find((d) => d.bp?.type === "getLocal"), 100, 100);
  fn.nodes.push(get);
  validateProject(clone(p));
  fn.locals = [];
  syncDefinitions(p);
  assert(!fn.nodes.some((n) => n.id === get.id));
  fn.locals = [{ id: "l1", name: "Base", type: "int32", default: 0 }];
  assert.throws(() => validateProject(clone(p)), /局部变量/);
});

test("event dispatchers: call/bind nodes, delegate pins and renames", () => {
  const p = makeProject();
  p.dispatchers.push({
    id: "d1",
    name: "OnHealed",
    description: "",
    params: [newParam("Amount", "int32")],
  });
  const event = makeNode(
    customEventDefinition("HandleHealed", [newParam("Amount", "int32")]),
    0,
    0,
  );
  const defs = dynamicDefinitions(p, p.graphs[0]);
  const bind = makeNode(defs.find((d) => d.name === "Bind Event to OnHealed"), 300, 0);
  const call = makeNode(defs.find((d) => d.name === "Call OnHealed"), 300, 200);
  p.graphs[0].nodes.push(event, bind, call);
  event.definition.bp.params[0].id = p.dispatchers[0].params[0].id;
  syncDefinitions(p);
  connect(p.graphs[0], { node: event.id, pin: "delegate" }, { node: bind.id, pin: "event" });
  assert.equal(p.graphs[0].edges.length, 1);
  validateProject(clone(p));
  // changing the dispatcher signature breaks the delegate wire
  p.dispatchers[0].params = [newParam("Amount", "float")];
  syncDefinitions(p);
  assert.equal(p.graphs[0].edges.length, 0);
  p.dispatchers = [];
  syncDefinitions(p);
  assert(!p.graphs[0].nodes.some((n) => n.definition.bp?.type === "dispatcher"));
  const pure = makeFunctionGraph("Q", { pure: true });
  p.dispatchers.push({ id: "d2", name: "X", description: "", params: [] });
  assert(!dynamicDefinitions(p, pure).some((d) => d.bp?.type === "dispatcher"));
});

test("macro nesting is limited and cycles are rejected", () => {
  const p = makeProject();
  const macros = Array.from({ length: 9 }, (_, i) => makeMacroGraph(`M${i}`));
  p.graphs.push(...macros);
  for (let i = 0; i < 7; i++) {
    const defs = dynamicDefinitions(p, macros[i]);
    const next = defs.find((d) => d.bp?.graphId === macros[i + 1].id);
    assert(next, `M${i} may contain M${i + 1}`);
    macros[i].nodes.push(makeNode(next, 200, 200));
  }
  // M0..M7 is eight levels; one more would exceed the limit
  assert(!dynamicDefinitions(p, macros[7]).some((d) => d.bp?.graphId === macros[8].id));
  validateProject(clone(p));
  macros[7].nodes.push(
    makeNode(
      { ...dynamicDefinitions(p, p.graphs[0]).find((d) => d.bp?.graphId === macros[0].id) },
      200,
      200,
    ),
  );
  assert.throws(() => validateProject(clone(p)), /宏不能引用自身/);
});

test("export lists dispatchers and local variables", () => {
  const p = makeProject();
  p.dispatchers.push({ id: "d1", name: "OnDone", description: "", params: [] });
  p.graphs.push(makeFunctionGraph("F", { locals: [{ name: "Tmp", type: "bool" }] }));
  const out = buildExport(p, { classes: engineClasses, enums: {} });
  assert.equal(out.dispatchers[0].name, "OnDone");
  assert.equal(out.functions[0].locals[0].name, "Tmp");
  assert.equal(out.stats.dispatchers, 1);
  assert.equal(out.stats.localVariables, 1);
  validateProject(JSON.parse(JSON.stringify(out)));
});

test("object inputs default to Self only when this blueprint is that type", async () => {
  const { defaultValue } = await import("../docs/model.js");
  const self = "UBaseCardObject*";
  assert.equal(defaultValue("UBaseCardObject*", {}, self), "Self");
  assert.equal(defaultValue("UObject*", {}, self), "Self");
  assert.equal(defaultValue("UPrimaryDataAsset*", {}, self), "Self");
  // ChangeAttack belongs to ACardFunctionsStub, which this card blueprint is not
  assert.equal(defaultValue("ACardFunctionsStub*", {}, self), "None");
  assert.equal(defaultValue("UWebSocketSubsystem*", {}, self), "None");
  assert.equal(defaultValue("FGameplayTag", {}, self), "None");
  assert.equal(defaultValue("int32", {}, self), 0);
  // without a blueprint context the old behaviour is kept
  assert.equal(defaultValue("ACardFunctionsStub*"), "Self");
});
