/* global document */
// End-to-end check of the update path: two builds are stamped like the Pages workflow does and served
// with a 10-minute cache like GitHub Pages; a page on build A must notice and move to build B.
import { test, expect } from "@playwright/test";
import http from "node:http";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";

const docs = new URL("../../docs/", import.meta.url).pathname;
const SHA = { A: "a".repeat(40), B: "b".repeat(40) }, VERSION = { A: "2.17.1", B: "9.0.0" };
let root, server, base, current = "A";

function assemble(out, sha, version) {
  fs.cpSync(docs, out, { recursive: true });
  const edit = (file, fn) => fs.writeFileSync(file, fn(fs.readFileSync(file, "utf8")));
  for (const f of fs.readdirSync(out)) {
    const full = path.join(out, f);
    if (f.endsWith(".js")) edit(full, (s) => (f === "app.js" ? s.replace("__BUILD_SHA__", sha).replace("__BUILD_VERSION__", version) : s).replace(/"(\.\/[A-Za-z0-9_-]+\.(?:js|json|css))"/g, `"$1?v=${sha}"`).replace(`./version.json?v=${sha}`, "./version.json"));
    if (f.endsWith(".html")) edit(full, (s) => s.replaceAll("__BUILD_SHA__", sha).replace(/(href|src)="(\.\/[A-Za-z0-9_-]+\.(?:js|css))"/g, `$1="$2?v=${sha}"`));
  }
  fs.writeFileSync(path.join(out, "version.json"), JSON.stringify({ sha, version, builtAt: "test" }));
}

test.beforeAll(async () => {
  root = fs.mkdtempSync(path.join(os.tmpdir(), "kards-update-"));
  for (const k of ["A", "B"]) assemble(path.join(root, k, "v1"), SHA[k], VERSION[k]);
  const types = { ".html": "text/html; charset=utf-8", ".js": "text/javascript", ".css": "text/css", ".json": "application/json", ".svg": "image/svg+xml", ".png": "image/png", ".webmanifest": "application/manifest+json" };
  server = http.createServer((req, res) => {
    const url = new URL(req.url, "http://x");
    if (url.pathname.startsWith("/__switch/")) { current = url.pathname.split("/").pop(); res.end(current); return; }
    let file = path.join(root, current, url.pathname);
    if (file.endsWith("/")) file += "index.html";
    fs.readFile(file, (err, data) => {
      if (err) { res.statusCode = 404; res.end("not found"); return; }
      res.setHeader("Cache-Control", "public, max-age=600");
      res.setHeader("Content-Type", types[path.extname(file)] || "application/octet-stream");
      res.end(data);
    });
  }).listen(0);
  await new Promise((r) => server.once("listening", r));
  base = `http://127.0.0.1:${server.address().port}`;
});
test.afterAll(() => { server.close(); fs.rmSync(root, { recursive: true, force: true }); });
test.beforeEach(async () => { current = "A"; });

const publishB = () => { current = "B"; };
const updateDialog = (page) => page.evaluate(() => { const m = document.getElementById("modal"); return m.open && /发现新版本/.test(m.textContent); });
async function runningVersion(page) {
  await page.locator("#settings").click();
  const line = (await page.locator(".version-line").textContent()).replace(/\s+/g, " ");
  await page.keyboard.press("Escape");
  return line;
}

test("a tab that stays open notices a new release by itself and updates without losing the card", async ({ page }) => {
  await page.clock.install();
  await page.goto(`${base}/v1/`);
  await expect(page.locator("#save-state")).toHaveText("本地已保存");
  await page.locator("#project-name").fill("UPDATE_SMOKE");
  await page.locator("#project-name").press("Tab");
  expect(await runningVersion(page)).toContain("2.17.1");
  publishB();
  expect(await updateDialog(page)).toBe(false);
  await page.clock.fastForward("11:00");
  await expect.poll(() => updateDialog(page)).toBe(true);
  await page.locator("#update-now").click();
  await expect(page).toHaveURL(/\?r=b{40}/);
  await expect.poll(() => runningVersion(page)).toContain("9.0.0");
  await expect(page.locator("#project-name")).toHaveValue("UPDATE_SMOKE");
});

test("a cached (stale) page is still offered the update when it is opened", async ({ page }) => {
  await page.goto(`${base}/v1/`);
  await expect(page.locator("#save-state")).toHaveText("本地已保存");
  publishB();
  await page.goto(`${base}/v1/`);
  expect(await page.evaluate(() => document.querySelector('meta[name="build"]').content)).toBe(SHA.A);
  await expect.poll(() => updateDialog(page), { timeout: 10000 }).toBe(true);
  await page.locator("#update-now").click();
  await expect.poll(() => runningVersion(page), { timeout: 15000 }).toContain("9.0.0");
});

test("Settings can check for and force the update, and the top-bar Refresh button does too", async ({ page }) => {
  await page.goto(`${base}/v1/`);
  await expect(page.locator("#save-state")).toHaveText("本地已保存");
  publishB();
  await page.locator("#settings").click();
  await page.locator("#ver-check").click();
  await expect(page.locator("#ver-status")).toContainText("有新版本");
  await expect(page.locator("#ver-status")).toContainText("9.0.0");
  await page.locator("#ver-refresh").click();
  await expect.poll(() => runningVersion(page), { timeout: 15000 }).toContain("9.0.0");
  current = "A";
  await page.goto(`${base}/v1/`);
  publishB();
  await page.locator("#force-refresh").click();
  await expect.poll(() => runningVersion(page), { timeout: 15000 }).toContain("9.0.0");
});
