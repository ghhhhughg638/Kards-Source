/* global document */
import { test, expect } from "@playwright/test";

test("reference library searches, selects functions and copies an editable card without console errors", async ({ page }) => {
  const errors = []; page.on("pageerror", (e) => errors.push(e.message));
  await page.goto("/reference.html");
  await expect(page.locator("#ref-total")).toHaveText("2019 个");
  await expect(page.locator("#ref-summary h2")).not.toHaveText("选择一张原版卡牌");
  await expect(page.locator(".ref-node").first()).toBeVisible();
  await page.locator("#ref-search").fill("WhichChooseOne");
  await expect(page.locator("#ref-cards .ref-card").first()).toBeVisible();
  await page.locator("#ref-cards .ref-card").first().click();
  await expect(page.locator("#ref-status")).toContainText("条源指令已保留");
  const options = await page.locator("#ref-graph option").allTextContents();
  expect(options.length).toBeGreaterThan(0);
  if (options.length > 1) { await page.locator("#ref-graph").selectOption({ index: 1 }); await expect(page.locator("#ref-instructions")).not.toBeEmpty(); }
  await page.locator("#ref-graph").selectOption({ index: 0 });
  await page.locator("#ref-fit").click();
  const advanced = page.locator(".ref-advanced").first();
  if (await advanced.count()) {
    const id = await advanced.getAttribute("data-advanced-node");
    const node = page.locator(`[data-ref-node="${id}"]`), before = await node.locator("[data-ref-pin]").count();
    await advanced.click();
    expect(await node.locator("[data-ref-pin]").count()).toBeGreaterThan(before);
    await page.locator("#ref-fit").click();
    await node.locator(".ref-advanced").click();
  }
  await page.locator("#ref-source summary").click();
  await expect(page.locator("#ref-instructions")).toBeVisible();
  await page.locator("#ref-source summary").click();
  await page.screenshot({ path: "test-results/original-reference.png" });
  await page.locator("#ref-copy").click();
  await expect(page).toHaveURL(/index\.html/);
  await expect(page.locator("#project-name")).not.toHaveValue("");
  await expect(page.locator("#save-state")).toHaveText("本地已保存");
  const folded = page.locator("[data-clear-fold]").first();
  if (await folded.count()) {
    const before = await page.locator("[data-clear-fold]").count();
    await folded.click();
    await expect(page.locator("[data-clear-fold]")).toHaveCount(before - 1);
  }
  expect(errors).toEqual([]);
});

test("reference enum constants are selectable inline and keep their enum type", async ({ page }) => {
  await page.goto("/reference.html");
  await expect(page.locator("#ref-total")).toHaveText("2019 个");
  await page.locator("#ref-search").fill("EnumCompareFaction");
  const items = page.locator("#ref-cards .ref-card");
  if (await items.count()) await items.first().click();
  else { await page.locator("#ref-search").fill("EnumCompareFaction"); await page.locator("#ref-cards .ref-card").first().click(); }
  await expect(page.locator("#ref-status")).toContainText("条源指令已保留");
  let found = false;
  for (let i = 0; i < await page.locator("#ref-graph option").count(); i++) {
    await page.locator("#ref-graph").selectOption({ index: i });
    const selects = page.locator("[data-ref-pin] select");
    if (await selects.count()) {
      const select = selects.first(); const values = await select.locator("option").evaluateAll((es) => es.map((e) => e.value));
      if (values.length < 2) continue;
      await select.selectOption(values[1]);
      await expect(select).toHaveValue(values[1]);
      await expect(page.locator("#ref-status")).toContainText("已调整参考选项");
      found = true; break;
    }
  }
  expect(found).toBe(true);
});

test("the main editor exposes the original-library entry and typed enum nodes", async ({ page }) => {
  await page.goto("/index.html");
  await expect(page.locator(".reference-library-link")).toHaveAttribute("href", "./reference.html");
  await expect(page.locator("#project-name")).not.toHaveValue("");
  await page.locator("#node-search").fill("Select (Enum)");
  await expect(page.locator("#node-list")).toContainText("Select");
});

test("viewer nodes use the editor node styling; AND/OR/compare nodes look like every other node", async ({ page }) => {
  await page.goto("/reference.html#ffc141adeef542f6");
  await expect(page.locator(".ref-node").first()).toBeVisible();
  expect(await page.locator(".ref-node.node").count()).toBeGreaterThan(5);
  expect(await page.locator(".ref-node.ue-compact").count()).toBe(0);
  const operator = page.locator(".ref-node.ue-slim").first();
  await expect(operator.locator(".node-head")).toBeVisible();
  expect(await operator.locator(".pin").count()).toBeGreaterThan(1);
  expect(await page.locator(".ref-node .pin").count()).toBeGreaterThan(10);
  expect(await page.locator(".ref-pin-row, .ref-dot").count()).toBe(0);
});

test.describe("touch gestures", () => {
  test.use({ hasTouch: true, viewport: { width: 1000, height: 900 } });
  test("pinch zoom still works when the first finger lands on a node's input", async ({ page }) => {
    await page.goto("/reference.html#f2f31478cbff8fb7");
    await expect(page.locator(".ref-node").first()).toBeVisible();
    const id = await page.evaluate(() => document.querySelector(".ref-node input, .ref-node select").closest("[data-ref-node]").dataset.refNode);
    await page.locator("#ref-locate").selectOption(id);
    const spot = await page.evaluate(() => {
      const vp = document.getElementById("ref-viewport").getBoundingClientRect();
      for (const e of document.querySelectorAll(".ref-node input, .ref-node select")) {
        const r = e.getBoundingClientRect(), c = [Math.round(r.x + r.width / 2), Math.round(r.y + r.height / 2)];
        if (c[0] > vp.x + 30 && c[0] < vp.right - 30 && c[1] > vp.y + 30 && c[1] < vp.bottom - 130 && r.width > 16) return c;
      }
      return null;
    });
    expect(spot).not.toBeNull();
    const before = await page.locator("#ref-zoom").textContent();
    const cdp = await page.context().newCDPSession(page);
    const points = (d) => [{ x: spot[0], y: spot[1], id: 1 }, { x: spot[0] + 70 + d, y: spot[1] + 50 + d / 2, id: 2 }];
    await cdp.send("Input.dispatchTouchEvent", { type: "touchStart", touchPoints: points(0) });
    for (let i = 1; i <= 8; i++) await cdp.send("Input.dispatchTouchEvent", { type: "touchMove", touchPoints: points(i * 12) });
    await cdp.send("Input.dispatchTouchEvent", { type: "touchEnd", touchPoints: [] });
    await expect(page.locator("#ref-zoom")).not.toHaveText(before);
  });

  test("one finger on a node drags the node, one finger on empty canvas pans", async ({ page }) => {
    await page.goto("/reference.html#1a1410e4609e6a61");
    await expect(page.locator(".ref-node").first()).toBeVisible();
    const cdp = await page.context().newCDPSession(page);
    const swipe = async (x, y, dx, dy) => {
      await cdp.send("Input.dispatchTouchEvent", { type: "touchStart", touchPoints: [{ x, y, id: 1 }] });
      for (let i = 1; i <= 10; i++) await cdp.send("Input.dispatchTouchEvent", { type: "touchMove", touchPoints: [{ x: x + dx * i / 10, y: y + dy * i / 10, id: 1 }] });
      await cdp.send("Input.dispatchTouchEvent", { type: "touchEnd", touchPoints: [] });
    };
    const node = page.locator(".ref-node").first();
    const head = await node.locator(".node-head").boundingBox();
    const stage = () => page.locator("#ref-stage").evaluate((e) => e.style.transform);
    const nodeBefore = await node.boundingBox(), stageBefore = await stage();
    await swipe(head.x + 20, head.y + head.height / 2, 60, 40);
    const nodeAfter = await node.boundingBox();
    expect(nodeAfter.x - nodeBefore.x).toBeGreaterThan(40);
    expect(nodeAfter.y - nodeBefore.y).toBeGreaterThan(25);
    expect(await stage()).toBe(stageBefore);
    const vp = await page.locator("#ref-viewport").boundingBox();
    await swipe(vp.x + vp.width - 40, vp.y + vp.height - 120, -50, -30);
    expect(await stage()).not.toBe(stageBefore);
  });
});

test("Settings shows the running version block (a local copy says it is not a published build)", async ({ page }) => {
  await page.goto("/index.html");
  await page.locator("#settings").click();
  await expect(page.locator(".version-field")).toBeVisible();
  await expect(page.locator(".version-field .version-line")).toContainText("本地开发版");
  await expect(page.locator("#ver-check")).toBeDisabled();
  await expect(page.locator("#ver-refresh")).toBeEnabled();
});

test("AND / compare nodes are slim: no subtitle, footer or wide add-pin button", async ({ page }) => {
  await page.goto("/reference.html#1a1410e4609e6a61");
  await expect(page.locator(".ref-node").first()).toBeVisible();
  const and = page.locator(".ref-node.ue-slim").first();
  const size = await and.evaluate((n) => [n.offsetWidth, n.offsetHeight]);
  expect(size[0]).toBeLessThan(170);
  expect(size[1]).toBeLessThan(120);
  await expect(and.locator(".node-subtitle")).toBeHidden();
});

test("dragging a node moves it and re-routes only its own wires", async ({ page }) => {
  await page.goto("/reference.html#1a1410e4609e6a61");
  await expect(page.locator(".ref-node").first()).toBeVisible();
  const picked = await page.evaluate(() => {
    const wires = [...document.querySelectorAll("#ref-wires path")];
    return wires.length;
  });
  expect(picked).toBeGreaterThan(5);
  const before = await page.evaluate(() => [...document.querySelectorAll("#ref-wires path")].map((p) => p.getAttribute("d")));
  const view = page.viewportSize();
  let node, box;
  for (const candidate of await page.locator(".ref-node").all()) {
    const b = await candidate.boundingBox();
    if (b && b.x > 0 && b.y > 0 && b.x + 60 < view.width && b.y + 40 < view.height) { node = candidate; box = b; break; }
  }
  expect(node, "a node on screen").toBeTruthy();
  await page.mouse.move(box.x + 20, box.y + 10);
  await page.mouse.down();
  for (let i = 1; i <= 12; i++) await page.mouse.move(box.x + 20 + i * 5, box.y + 10 + i * 3);
  await page.mouse.up();
  const after = await node.boundingBox();
  expect(after.x - box.x).toBeGreaterThan(40);
  expect(after.y - box.y).toBeGreaterThan(20);
  const now = await page.evaluate(() => [...document.querySelectorAll("#ref-wires path")].map((p) => p.getAttribute("d")));
  expect(now.length).toBe(before.length);
  const changed = now.filter((d, i) => d !== before[i]).length;
  expect(changed).toBeGreaterThan(0);
  expect(changed).toBeLessThan(before.length);
});
