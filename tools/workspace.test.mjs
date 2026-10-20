import test from "node:test";
import assert from "node:assert/strict";
import zlib from "node:zlib";
import { createWorkspace, LEGACY_KEY } from "../docs/workspace.js";
import { crc32, makeZip, readZip } from "../docs/zip.js";

const memory = () => {
  const map = new Map();
  return {
    map,
    getItem: (k) => (map.has(k) ? map.get(k) : null),
    setItem: (k, v) => map.set(k, String(v)),
    removeItem: (k) => map.delete(k),
  };
};
const card = (id, name) => ({ id, name, card: { title: `T-${name}`, faction: "Soviet", type: "infantry" } });

test("workspace keeps several cards, the active one and their order", () => {
  const ws = createWorkspace(memory()).load();
  ws.save(card("a", "BP_A"));
  ws.save(card("b", "BP_B"));
  assert.deepEqual(ws.ids(), ["a", "b"]);
  assert.equal(ws.active(), "b");
  assert.equal(ws.read("a").name, "BP_A");
  assert.deepEqual(ws.list().map((c) => c.title), ["T-BP_A", "T-BP_B"]);
  ws.setActive("a");
});

test("workspace survives a reload of the storage and removes cards", () => {
  const storage = memory();
  const one = createWorkspace(storage).load();
  one.save(card("a", "BP_A"));
  one.save(card("b", "BP_B"));
  one.save(card("c", "BP_C"));
  one.setActive("b");
  const two = createWorkspace(storage).load();
  assert.deepEqual(two.ids(), ["a", "b", "c"]);
  assert.equal(two.active(), "b");
  two.remove("b");
  assert.deepEqual(two.ids(), ["a", "c"]);
  assert.equal(two.active(), "c"); // the neighbour that took its place
  assert.equal(storage.getItem("kards-blueprint-studio.card.b"), null);
  two.remove("a");
  two.remove("c");
  assert.equal(two.active(), null);
});

test("asset names are unique across the workspace", () => {
  const ws = createWorkspace(memory()).load();
  ws.save(card("a", "BP_Card"));
  assert(ws.nameTaken("bp_card"));
  assert(!ws.nameTaken("BP_Card", "a"));
  assert.equal(ws.uniqueName("BP_Card"), "BP_Card_2");
  ws.save(card("b", "BP_Card_2"));
  assert.equal(ws.uniqueName("BP_Card"), "BP_Card_3");
  assert.equal(ws.uniqueName("BP_Other"), "BP_Other");
});

test("a pre-workspace single project becomes the first card", () => {
  const storage = memory();
  storage.setItem(LEGACY_KEY, JSON.stringify(card("old", "BP_Old")));
  const ws = createWorkspace(storage).load();
  assert.deepEqual(ws.ids(), ["old"]);
  assert.equal(ws.active(), "old");
  assert.equal(storage.getItem(LEGACY_KEY), null);
});

test("zip round trip keeps names (including non-ASCII) and content", async () => {
  const files = [
    { name: "BP_A.kardsbp.json", data: JSON.stringify({ a: 1, text: "你好" }) },
    { name: "BP_B.kardsbp.json", data: "{}" },
  ];
  const zip = makeZip(files, new Date(2026, 0, 2, 3, 4, 6));
  assert.equal(zip[0], 0x50);
  const back = await readZip(zip);
  assert.deepEqual(back.map((f) => f.name), files.map((f) => f.name));
  assert.equal(JSON.parse(back[0].text).text, "你好");
  assert.equal(crc32(new TextEncoder().encode("123456789")), 0xcbf43926);
});

test("zip reader also handles deflate entries made by other tools", async () => {
  const text = JSON.stringify({ hello: "world".repeat(50) });
  const data = zlib.deflateRawSync(Buffer.from(text));
  const name = Buffer.from("x.kardsbp.json");
  const local = Buffer.alloc(30);
  local.writeUInt32LE(0x04034b50, 0);
  local.writeUInt16LE(8, 8);
  local.writeUInt32LE(data.length, 18);
  local.writeUInt32LE(text.length, 22);
  local.writeUInt16LE(name.length, 26);
  const dir = Buffer.alloc(46);
  dir.writeUInt32LE(0x02014b50, 0);
  dir.writeUInt16LE(8, 10);
  dir.writeUInt32LE(data.length, 20);
  dir.writeUInt32LE(text.length, 24);
  dir.writeUInt16LE(name.length, 28);
  dir.writeUInt32LE(0, 42);
  const end = Buffer.alloc(22);
  end.writeUInt32LE(0x06054b50, 0);
  end.writeUInt16LE(1, 8);
  end.writeUInt16LE(1, 10);
  const body = Buffer.concat([local, name, data]);
  end.writeUInt32LE(dir.length + name.length, 12);
  end.writeUInt32LE(body.length, 16);
  const zip = Buffer.concat([body, dir, name, end]);
  const back = await readZip(new Uint8Array(zip));
  assert.equal(back[0].text, text);
  await assert.rejects(readZip(new Uint8Array([1, 2, 3])), /ZIP/);
});

test("two tabs sharing one storage never drop each other's cards", () => {
  const storage = memory();
  const tabA = createWorkspace(storage).load();
  tabA.save(card("a", "BP_A"));
  const tabB = createWorkspace(storage).load();
  tabB.save(card("b", "BP_B"));
  // tab A still holds the old index in memory, then saves again
  tabA.save(card("a", "BP_A"));
  tabA.save(card("c", "BP_C"));
  assert.deepEqual(createWorkspace(storage).load().ids().sort(), ["a", "b", "c"]);
  tabB.remove("b");
  assert.deepEqual(createWorkspace(storage).load().ids().sort(), ["a", "c"]);
});

test("cards that fell out of the index are adopted again on load", () => {
  const map = new Map();
  const storage = {
    get length() {
      return map.size;
    },
    key: (i) => [...map.keys()][i] ?? null,
    getItem: (k) => (map.has(k) ? map.get(k) : null),
    setItem: (k, v) => map.set(k, String(v)),
    removeItem: (k) => map.delete(k),
  };
  const one = createWorkspace(storage).load();
  one.save(card("a", "BP_A"));
  one.save(card("b", "BP_B"));
  // an old build rewrites the index and forgets card b
  storage.setItem("kards-blueprint-studio.workspace", JSON.stringify({ order: ["a"], active: "a", meta: {} }));
  const two = createWorkspace(storage).load();
  assert.deepEqual(two.ids().sort(), ["a", "b"]);
  assert.equal(two.adopted, 1);
  assert.equal(two.meta("b").name, "BP_B");
  // a broken entry is skipped, not fatal
  storage.setItem("kards-blueprint-studio.card.x", "{nope");
  assert.deepEqual(createWorkspace(storage).load().ids().sort(), ["a", "b"]);
});
