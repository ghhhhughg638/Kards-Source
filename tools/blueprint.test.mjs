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
