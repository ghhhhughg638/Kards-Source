import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import {
  clone,
  builtins,
  makeGraph,
  makeNode,
  createDemo,
  validateProject,
  connect,
} from "../docs/model.js";
const catalog = JSON.parse(
  await readFile(new URL("../docs/catalog.json", import.meta.url), "utf8"),
);
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
test("demo survives a JSON round trip, including definitions and edges", () => {
  const p = createDemo(catalog.nodes);
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
  const p = createDemo(catalog.nodes);
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
  const p = createDemo(catalog.nodes);
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

test("full example includes a valid Cast & Call graph with execution and typed target links", () => {
  const full = [
    ...builtins,
    ...typeNodes({ ...engineClasses, ...catalog.classes }, catalog.properties),
    ...catalog.nodes,
  ];
  const demo = createDemo(full);
  validateProject(clone(demo));
  assert.equal(demo.graphs.length, 3);
  const example = demo.graphs.find((g) => g.name.includes("Cast & Call"));
  assert.equal(example.edges.length, 5);
  assert(example.nodes.some((n) => n.definition.kind === "cast"));
});
