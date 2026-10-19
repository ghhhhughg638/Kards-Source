import test from "node:test";
import assert from "node:assert/strict";
import { readFile, readdir } from "node:fs/promises";
import { compareVersions, entriesSince, isOutdated, isStamped } from "../docs/update.js";

const read = (p) => readFile(new URL(p, import.meta.url), "utf8");
const changelog = JSON.parse(await read("../docs/changelog.json"));

test("versions compare numerically, not as text", () => {
  assert.equal(compareVersions("2.10.0", "2.9.1"), 1);
  assert.equal(compareVersions("2.6.0", "2.6"), 0);
  assert.equal(compareVersions("1.9", "2.0.0"), -1);
});

test("only stamped builds can be outdated", () => {
  const latest = { sha: "bbb", version: "2.7.0" };
  assert(!isStamped({ sha: "__BUILD_SHA__" }));
  assert(!isOutdated({ sha: "__BUILD_SHA__", version: "__BUILD_VERSION__" }, latest));
  assert(isOutdated({ sha: "aaa", version: "2.6.0" }, latest));
  assert(!isOutdated({ sha: "bbb", version: "2.7.0" }, latest));
  assert(!isOutdated({ sha: "aaa" }, {}));
});

test("the dialog lists what changed since the running version", () => {
  const since = entriesSince(changelog, "2.5.0");
  assert.deepEqual(since.map((e) => e.version), ["2.8.1", "2.8.0", "2.7.2", "2.7.1", "2.7.0", "2.6.0"]);
  assert.equal(entriesSince(changelog, "9.9.9").length, 1); // fall back to the newest entry
  assert.deepEqual(entriesSince(null, "1.0.0"), []);
});

test("changelog is newest first and every item has Chinese and English text", () => {
  for (let i = 1; i < changelog.length; i++)
    assert.equal(compareVersions(changelog[i - 1].version, changelog[i].version), 1);
  for (const entry of changelog) {
    assert(entry.title.zh && entry.title.en && entry.date);
    for (const item of entry.items) assert(item.zh && item.en);
  }
});

test("the deploy stamp placeholders appear exactly once in app.js", async () => {
  const app = await read("../docs/app.js");
  assert.equal(app.split("__BUILD_SHA__").length - 1, 1);
  assert.equal(app.split("__BUILD_VERSION__").length - 1, 1);
});

test("Refresh / Update re-download every file the page loads", async () => {
  const app = await read("../docs/app.js");
  const list = app.slice(app.indexOf("const SITE_FILES = ["), app.indexOf("];", app.indexOf("const SITE_FILES = [")));
  const files = (await readdir(new URL("../docs/", import.meta.url))).filter((f) => /\.(js|css|json)$/.test(f));
  for (const f of files) assert(list.includes(`"./${f}"`), `${f} is missing from SITE_FILES`);
});

test("the deploy workflow stamps the build and writes version.json", async () => {
  const workflow = await read("../.github/workflows/pages.yml");
  assert(workflow.includes("__BUILD_SHA__") && workflow.includes("version.json"));
  assert(workflow.includes("docs/changelog.json"));
});
