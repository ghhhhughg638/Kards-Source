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
    const node = page.locator(`[data-ref-node="${id}"]`), before = await node.locator(".ref-pin-row").count();
    await advanced.click();
    expect(await node.locator(".ref-pin-row").count()).toBeGreaterThan(before);
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
    const selects = page.locator(".ref-pin-row select");
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
