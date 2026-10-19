import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import {
  clone,
  builtins,
  makeGraph,
  makeNode,
  makeProject,
  validateProject,
  connect,
} from "../docs/model.js";
const catalog = JSON.parse(
  await readFile(new URL("../docs/catalog.json", import.meta.url), "utf8"),
);

// A small project assembled from real catalog nodes; replaces the removed built-in demo.
function fixtureProject(defs = catalog.nodes) {
  const p = makeProject(),
    g = p.graphs[0],
    find = (name) => [...defs, ...builtins].find((n) => n.name === name);
  const event = makeNode(find("OnPlayedFromHand"), 70, 180),
    branch = makeNode(find("Branch"), 390, 180),
    blitz = makeNode(find("GiveBlitz"), 720, 110),
    guard = makeNode(find("GiveGuard"), 720, 370),
    condition = makeNode(find("Boolean"), 80, 410);
  condition.values = { Value: true };
  g.nodes = [event, branch, blitz, guard, condition];
  connect(g, { node: event.id, pin: "exec-out" }, { node: branch.id, pin: "exec-in" });
  connect(g, { node: condition.id, pin: "Value" }, { node: branch.id, pin: "Condition" });
  connect(g, { node: branch.id, pin: "True" }, { node: blitz.id, pin: "exec-in" });
  connect(g, { node: branch.id, pin: "False" }, { node: guard.id, pin: "exec-in" });
  return p;
}
test("all native definitions have unique IDs and valid, distinct pins", () => {
  const ids = new Set();
  for (const d of catalog.nodes) {
    assert(!ids.has(d.id), d.id);
    ids.add(d.id);
    const pins = new Set();
    for (const p of d.pins) {
      assert(!pins.has(p.id), `${d.id}.${p.id}`);
      pins.add(p.id);
      assert(["in", "out"].includes(p.direction));
      assert(p.type);
    }
  }
  assert(catalog.nodes.length > 500);
  assert(catalog.enums.EFactionEnum.includes("Soviet"));
});
test("project survives a JSON round trip, including definitions and edges", () => {
  const p = fixtureProject();
  assert.deepEqual(validateProject(JSON.parse(JSON.stringify(p))), p);
  assert.equal(p.graphs[0].edges.length, 4);
});
test("connecting pins checks direction and exact type", () => {
  const g = makeGraph(),
    a = makeNode(builtins[2], 0, 0),
    b = makeNode(builtins[0], 100, 0),
    i = makeNode(builtins[3], 0, 100);
  g.nodes = [a, b, i];
  connect(g, { node: a.id, pin: "Value" }, { node: b.id, pin: "Condition" });
  assert.throws(
    () =>
      connect(
        g,
        { node: i.id, pin: "Value" },
        { node: b.id, pin: "Condition" },
      ),
    /类型不匹配/,
  );
  assert.equal(g.edges.length, 1);
  assert.throws(
    () =>
      connect(g, { node: a.id, pin: "Value" }, { node: i.id, pin: "Value" }),
    /输出端口/,
  );
});
test("a new input connection replaces the old input and duplicate is a no-op", () => {
  const g = makeGraph(),
    a = makeNode(builtins[2], 0, 0),
    b = makeNode(builtins[2], 0, 100),
    branch = makeNode(builtins[0], 200, 0);
  g.nodes = [a, b, branch];
  const to = { node: branch.id, pin: "Condition" };
  connect(g, { node: a.id, pin: "Value" }, to);
  connect(g, { node: b.id, pin: "Value" }, to);
  assert.equal(g.edges.length, 1);
  assert.equal(g.edges[0].from.node, b.id);
  assert.equal(connect(g, { node: b.id, pin: "Value" }, to), false);
});
test("import rejects unsupported versions and dangling, duplicate or mismatched connections", () => {
  const p = fixtureProject();
  const badVersion = clone(p);
  badVersion.version = 999;
  assert.throws(() => validateProject(badVersion));
  const dangling = clone(p);
  dangling.graphs[0].edges[0].from.node = "missing";
  assert.throws(() => validateProject(dangling), /连接/);
  const duplicate = clone(p);
  duplicate.graphs[0].nodes.push(clone(duplicate.graphs[0].nodes[0]));
  assert.throws(() => validateProject(duplicate), /节点标识/);
  const mismatch = clone(p);
  mismatch.graphs[0].edges[0].from.pin = "targetCard";
  assert.throws(() => validateProject(mismatch), /类型不匹配/);
});
test("import restores missing view defaults but rejects nonfinite node positions", () => {
  const p = fixtureProject();
  delete p.graphs[0].view;
  validateProject(p);
  assert.equal(p.graphs[0].view.zoom, 1);
  p.graphs[0].nodes[0].x = Infinity;
  assert.throws(() => validateProject(p), /位置/);
});

import { engineClasses, typeNodes } from "../docs/ue-nodes.js";
import { setTypeHierarchy, compatible, matchingPin } from "../docs/model.js";
import { nodeEnglish, nodeChinese } from "../docs/i18n.js";
setTypeHierarchy({ ...engineClasses, ...catalog.classes });
test("typed object connections follow declared inheritance; casts enable downcasts", () => {
  assert(compatible("UBaseCardObject*", "UObject*"));
  assert(compatible("UBaseCardObject*", "UPrimaryDataAsset*"));
  assert(!compatible("UObject*", "UBaseCardObject*"));
  assert(!compatible("int32", "float"));
  assert(!compatible("TArray<UBaseCardObject*>", "TArray<UObject*>"));
});
test("every UE, property and cast definition has a distinct ID and valid pins", () => {
  const defs = [
    ...builtins,
    ...typeNodes({ ...engineClasses, ...catalog.classes }, catalog.properties),
    ...catalog.nodes,
  ];
  const ids = new Set();
  for (const d of defs) {
    assert(!ids.has(d.id), d.id);
    ids.add(d.id);
    const pins = new Set();
    for (const p of d.pins) {
      assert(!pins.has(p.id), d.id + "." + p.id);
      pins.add(p.id);
      assert(["in", "out"].includes(p.direction));
    }
  }
  assert(defs.some((d) => d.kind === "cast"));
  assert(builtins.some((d) => d.name === "ForLoop"));
});
test("Cast output discovers compatible native targets and property accessors", () => {
  const defs = typeNodes(
    { ...engineClasses, ...catalog.classes },
    catalog.properties,
  );
  const cast = defs.find((d) => d.id === "ue.cast.UBaseCardObject");
  const object = cast.pins.find((p) => p.id === "AsUBaseCardObject");
  const get = defs.find((d) => d.id === "UBaseCardObject.get.attack");
  assert.equal(matchingPin(get, object).id, "target");
  const reset = catalog.nodes.find(
    (d) => d.id === "UBaseCardObject.ResetCardAttributes",
  );
  assert.equal(matchingPin(reset, object).id, "target");
  const graph = makeGraph();
  const a = makeNode(cast, 0, 0),
    b = makeNode(get, 300, 0),
    c = makeNode(reset, 300, 200);
  graph.nodes = [a, b, c];
  connect(
    graph,
    { node: a.id, pin: "AsUBaseCardObject" },
    { node: b.id, pin: "target" },
  );
  connect(
    graph,
    { node: a.id, pin: "AsUBaseCardObject" },
    { node: c.id, pin: "target" },
  );
  assert.equal(graph.edges.length, 2);
});
test("event labels retain explicit English Event and Chinese display text", () => {
  const event = catalog.nodes.find((n) => n.name === "OnPlayedFromHand");
  assert.equal(nodeEnglish(event), "Event OnPlayedFromHand");
  assert(nodeChinese(event).includes("事件"));
  assert(nodeChinese(event).includes("从手牌打出"));
});

test("new projects start empty with a single event graph", () => {
  const p = makeProject();
  assert.equal(p.graphs.length, 1);
  assert.equal(p.graphs[0].nodes.length, 0);
  validateProject(clone(p));
});

test("Cast & Call graph validates with execution and typed target links", () => {
  const full = [
    ...builtins,
    ...typeNodes({ ...engineClasses, ...catalog.classes }, catalog.properties),
    ...catalog.nodes,
  ];
  const p = fixtureProject(full),
    g = makeGraph("Cast & Call"),
    find = (id) => full.find((d) => d.id === id),
    byName = (name) => full.find((d) => d.name === name);
  const entry = makeNode(byName("OnStartOfGame"), 40, 150),
    object = makeNode(byName("ObjectReference"), 40, 380),
    cast = makeNode(find("ue.cast.UBaseCardObject"), 390, 150),
    call = makeNode(find("UBaseCardObject.ResetCardAttributes"), 750, 100),
    get = makeNode(find("UBaseCardObject.get.attack"), 750, 410);
  g.nodes = [entry, object, cast, call, get];
  connect(g, { node: entry.id, pin: "exec-out" }, { node: cast.id, pin: "exec-in" });
  connect(g, { node: object.id, pin: "Value" }, { node: cast.id, pin: "Object" });
  connect(g, { node: cast.id, pin: "Success" }, { node: call.id, pin: "exec-in" });
  connect(g, { node: cast.id, pin: "AsUBaseCardObject" }, { node: call.id, pin: "target" });
  connect(g, { node: cast.id, pin: "AsUBaseCardObject" }, { node: get.id, pin: "target" });
  p.graphs.push(g);
  validateProject(clone(p));
  assert.equal(g.edges.length, 5);
  assert(g.nodes.some((n) => n.definition.kind === "cast"));
});
