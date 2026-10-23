import { pinColor, nodeAppearance, dynamicPinSpec, canRemoveDynamicPin, addDynamicPin, removeDynamicPin, applyUEPalette } from "./ue-node-features.js";
import { buildReferenceProject } from "./reference-graph.js";
import { createWorkspace } from "./workspace.js";
import { validateProject, setEnumTable, setTypeHierarchy, resolveWildcards } from "./model.js";
import { nodeChinese } from "./i18n.js";

applyUEPalette(document.documentElement);
const $ = (id) => document.getElementById(id);
const esc = (s) => String(s ?? "").replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[c]);
const factions = { Britain: "英国", Germany: "德国", Soviet: "苏联", Japan: "日本", USA: "美国", France: "法国", Finland: "芬兰", Italy: "意大利", Poland: "波兰", Neutral: "中立", Anzac: "澳新", ANZAC: "澳新", NotAvailable: "其他" };
let library, card, project, page = 0, request = 0, view = { x: 30, y: 30, zoom: 0.8 };
const narrow = matchMedia("(max-width: 1024px)");
function showPanel(panel) {
  document.querySelector(".ref-layout").dataset.panel = panel;
  if (panel === "graph") requestAnimationFrame(() => { if (graph()) { renderGraph(!viewReady); viewReady = true; } });
}
let viewReady = false, focusedNode;
$("ref-browse").onclick = () => showPanel("library");
$("ref-close-library").onclick = () => showPanel("graph");
$("ref-close-details").onclick = () => { document.querySelector(".ref-layout").dataset.panel = "graph"; };
$("ref-more").onclick = () => {
  const open = $("ref-more").getAttribute("aria-expanded") !== "true";
  $("ref-more").setAttribute("aria-expanded", String(open));
  $("ref-actions").classList.toggle("open", open);
};
function closeTools() { $("ref-actions").classList.remove("open"); $("ref-more").setAttribute("aria-expanded", "false"); }
$("ref-viewport").addEventListener("pointerdown", closeTools);
$("ref-info").onclick = () => {
  $("ref-info-content").replaceChildren($("ref-summary").cloneNode(true));
  $("ref-info-content").firstChild.removeAttribute("id");
  for (const [title, content] of [["类默认值与资源", $("ref-defaults").textContent], ["源指令", $("ref-instructions").textContent]]) {
    const d = document.createElement("details"), heading = document.createElement("summary"), pre = document.createElement("pre");
    heading.textContent = title; pre.textContent = content; d.append(heading, pre); $("ref-info-content").append(d);
  }
  closeTools(); $("ref-info-dialog").showModal();
};
$("ref-mode").onchange = () => {
  if (!card) return;
  const previous = project.activeGraph;
  project = buildReferenceProject(card, library, { expanded: $("ref-mode").value === "expanded" });
  if (project.graphs.some((g) => g.id === previous)) project.activeGraph = previous;
  closeTools(); renderGraph();
};
const pageSize = 40;
const cache = new Map();
const graph = () => project?.graphs.find((g) => g.id === project.activeGraph);
const status = (message, error = false) => { $("ref-status").textContent = message; $("ref-status").classList.toggle("error", error); };
const snapshot = (key) => {
  const url = new URL(key, import.meta.url), build = new URL(import.meta.url).searchParams.get("v");
  if (build) url.searchParams.set("v", build);
  return url.href;
};

const searchText = (s) => String(s ?? "").normalize("NFKC").toLowerCase().replace(/<[^>]*>/g, "").replace(/[\s\p{P}]+/gu, "");
const plainText = (s) => String(s ?? "").replace(/<[^>]*>/g, "");
function filteredCards() {
  const terms = $("ref-search").value.trim().split(/\s+/).map(searchText).filter(Boolean), faction = $("ref-faction").value, type = $("ref-type").value;
  return library.cards.filter((c) => (!faction || c.faction === faction) && (!type || c.type === type) && (!$("ref-with-logic").checked || !c.dataOnly) && terms.every((term) => c.searchIndex.includes(term)));

}
function renderList() {
  const cards = filteredCards(); page = Math.min(page, Math.max(0, Math.ceil(cards.length / pageSize) - 1));
  $("ref-result-count").textContent = `${cards.length} 个匹配 / ${library.cards.length} 个蓝图`;
  $("ref-page").textContent = `${page + 1} / ${Math.max(1, Math.ceil(cards.length / pageSize))}`;
  $("ref-prev").disabled = page === 0; $("ref-next").disabled = (page + 1) * pageSize >= cards.length;
  $("ref-cards").innerHTML = cards.slice(page * pageSize, (page + 1) * pageSize).map((c) => `<button type="button" class="ref-card ${c.id === card?.id ? "active" : ""}" data-card="${esc(c.id)}"><strong>${esc(c.zh.title || c.title)}</strong>${c.zh.title && c.zh.title !== c.title ? `<span class="ref-card-english">${esc(c.title)}</span>` : ""}<small>${esc(c.name)}</small>${c.zh.text || c.text ? `<span class="ref-card-effect">${esc(plainText(c.zh.text || c.text))}</span>` : ""}<span class="ref-card-tags">${esc(factions[c.faction] || c.faction)} · ${esc(c.type)} · ${c.cost} 费 · ${c.dataOnly ? "仅默认数据" : `${c.functionCount} 函数`}${Object.values(c.zh.origins || {}).includes("reference") ? " · 参考译文" : ""}</span></button>`).join("") || '<p class="ref-muted">没有匹配的卡牌，可试试“总部 伤害”等效果关键词。</p>';
  $("ref-cards").querySelectorAll("[data-card]").forEach((b) => b.addEventListener("click", async () => { await openCard(b.dataset.card); if (narrow.matches) showPanel("graph"); }));
}
async function openCard(id, graphName) {
  const ticket = ++request;
  status("正在读取蓝图…");
  try {
    let payload = cache.get(id);
    if (!payload) {
      const response = await fetch(snapshot(`./reference-data/cards/${id}.json`));
      if (!response.ok) throw new Error(`蓝图文件加载失败 (${response.status})`);
      payload = await response.json(); cache.set(id, payload);
      if (cache.size > 12) cache.delete(cache.keys().next().value);
    }
    if (ticket !== request) return;
    card = { ...payload, translation: library.cards.find((c) => c.id === id)?.zh || {} }; document.querySelector(".ref-heading strong").textContent = card.translation.title || card.card.title; project = buildReferenceProject(card, library, { expanded: $("ref-mode").value === "expanded" });
    setEnumTable(library.enums); setTypeHierarchy(project.types);
    if (graphName) project.activeGraph = project.graphs.find((g) => g.id === graphName || g.sourceName === graphName)?.id || project.activeGraph;
    $("ref-summary").innerHTML = `<h2>${esc(card.translation.title || card.card.title)}</h2>${card.translation.title && card.translation.title !== card.card.title ? `<p class="ref-original-text">${esc(card.card.title)}</p>` : ""}<div class="ref-package">${esc(card.name)} · ${esc(card.package)}</div><div class="ref-badges"><span class="ref-badge">${esc(factions[card.card.faction] || card.card.faction)}</span><span class="ref-badge">${esc(card.card.type)} · ${esc(card.card.rarity)}</span><span class="ref-badge">${card.card.cost} 费 · ${card.card.operationCost} 行动 · ${card.card.attack}/${card.card.defense}</span><span class="ref-badge">${card.functions.length} 函数</span></div><p>${esc(plainText(card.translation.text || card.card.text))}</p>${card.translation.text && card.translation.text !== card.card.text ? `<p class="ref-original-text">英文原文：${esc(card.card.text)}</p>` : ""}${Object.values(card.translation.origins || {}).includes("reference") ? '<p class="ref-translation-note">包含参考译文：解包未提供对应中文时补译，英文原文保留。</p>' : ""}`;
    $("ref-defaults").textContent = JSON.stringify({ package: card.package, parent: card.parent, defaults: card.defaults, provenance: card.provenance }, null, 2);
    $("ref-graph").innerHTML = project.graphs.map((g) => `<option value="${esc(g.id)}" ${g.id === project.activeGraph ? "selected" : ""}>${esc(g.sourceName || g.name)}${g.pure ? " · Pure" : ""}</option>`).join("");
    for (const id of ["ref-graph", "ref-copy", "ref-export"]) $(id).disabled = false;
    viewReady = false; renderList(); renderGraph();
    status(`${card.functions.reduce((s, f) => s + f.body.length, 0)} 条源指令已保留 · 自动布局 · ${$("ref-mode").value === "expanded" ? "完整展开逻辑" : "精简参考"}${project.reference.diagnostics.length ? ` · ${project.reference.diagnostics.length} 个待检查控制流` : ""}`);
  } catch (e) { if (ticket === request) status(e.message, true); }
}
function pinControl(n, p) {
  if (p.direction !== "in" || p.type === "exec") return "";
  if (graph().edges.some((e) => e.to.node === n.id && e.to.pin === p.id)) return "";
  if (n.folded?.[p.id]) return `<span class="ref-folded" title="${esc(n.folded[p.id].text)}">ƒ ${esc(n.folded[p.id].text)}</span>`;
  const value = n.values?.[p.id] ?? "", attrs = `data-node-value="${esc(n.id)}" data-pin-value="${esc(p.id)}"`;
  if (library.enums[p.type]) {
    const vs = [...new Set([...library.enums[p.type], String(value)].filter(Boolean))];
    return `<select ${attrs} aria-label="${esc(`${n.definition.name} ${p.name} ${p.type}`)}">${vs.map((v) => `<option value="${esc(v)}" ${v === String(value) ? "selected" : ""}>${esc(library.enumLabels?.[p.type]?.[v] || v)}</option>`).join("")}</select>`;
  }
  if (value === "") return "";
  if (typeof value === "boolean") return `<input type="checkbox" class="pin-checkbox" ${attrs} ${value ? "checked" : ""} aria-label="${esc(p.name)} 默认布尔值">`;
  return `<input ${attrs} aria-label="${esc(p.name)}" value="${esc(value)}" type="${typeof value === "number" ? "number" : "text"}" />`;
}
function renderGraph(refit = true) {
  const g = graph(); if (!g) return;
  history.replaceState(null, "", `#${card.id}~${encodeURIComponent(g.id)}`);
  $("ref-empty").hidden = !!g.nodes.length;
  $("ref-empty").textContent = "此图没有可执行效果节点。可查看类默认值或切换其他函数。";
  $("ref-nodes").innerHTML = g.nodes.map((n) => `<article class="ref-node ${esc(n.definition.kind || "function")} ${nodeAppearance(n.definition, library.enums).compact ? "ue-compact" : ""}" style="--node-color:${nodeAppearance(n.definition, library.enums).color};left:${n.x}px;top:${n.y}px" data-ref-node="${esc(n.id)}"><div class="ref-node-head"><strong>${esc(nodeAppearance(n.definition, library.enums).compact ? nodeAppearance(n.definition, library.enums).title : nodeChinese(n.definition))}</strong><small>${esc(n.definition.name)}</small></div><div class="ref-node-body">${n.definition.pins.filter((p) => n.showAdvanced || !n.definition.advancedPins?.includes(p.id) || g.edges.some((e) => e.to.node === n.id && e.to.pin === p.id)).map((p) => `<div class="ref-pin-row ${p.direction === "out" ? "output" : ""}" data-ref-pin="${esc(p.id)}" style="--ref-pin:${pinColor(p.type, library.enums)}">${p.direction === "in" ? `<i class="ref-dot ${p.type === "exec" ? "exec" : ""}"></i>` : ""}<span class="ref-pin-label">${esc(p.name)}</span>${p.type !== "exec" ? `<span class="ref-pin-type" title="${esc(p.type)}">${esc(p.type)}</span>` : ""}${pinControl(n, p)}${canRemoveDynamicPin(n.definition, p.id) ? `<button type="button" class="remove-user-pin" data-ref-remove-pin="${esc(p.id)}" data-pin-owner="${esc(n.id)}" aria-label="删除 ${esc(p.name)} 引脚">×</button>` : ""}${p.direction === "out" ? `<i class="ref-dot ${p.type === "exec" ? "exec" : ""}"></i>` : ""}</div>`).join("")}${$("ref-mode").value === "compact" && n.definition.advancedPins?.length ? `<button class="ref-advanced" data-advanced-node="${esc(n.id)}">${n.showAdvanced ? "收起参数" : `更多参数 (${n.definition.advancedPins.length})`} ▾</button>` : ""}${dynamicPinSpec(n.definition) ? `<button type="button" class="add-user-pin" data-ref-add-pin="${esc(n.id)}">＋ 添加引脚／分支</button>` : ""}</div><div class="ref-node-note">${n.definition.reference ? `${esc(n.definition.reference.function)} · @${n.definition.reference.sourceOffset}` : "Function signature"}</div></article>`).join("");
  // Size columns using real content height; busy native functions must not overlap.
  const columns = new Map();
  for (const n of g.nodes) {
    const ys = columns.get(n.x) || 60; n.y = ys;
    const el = [...$("ref-nodes").children].find((e) => e.dataset.refNode === n.id);
    el.style.top = `${ys}px`; columns.set(n.x, ys + el.offsetHeight + 65);
    el.addEventListener("click", (e) => { if (!e.target.closest("input,select,button")) inspect(n); });
  }
  $("ref-nodes").querySelectorAll("[data-advanced-node]").forEach((b) => b.addEventListener("click", () => { const n = g.nodes.find((n) => n.id === b.dataset.advancedNode); n.showAdvanced = !n.showAdvanced; renderGraph(false); }));
  $("ref-nodes").querySelectorAll("[data-node-value]").forEach((el) => el.addEventListener("change", () => {
    const n = g.nodes.find((n) => n.id === el.dataset.nodeValue), p = n.definition.pins.find((p) => p.id === el.dataset.pinValue), current = n.values[p.id];
    n.values[p.id] = typeof current === "boolean" ? el.checked : typeof current === "number" ? Number(el.value) : el.value;
    status("已调整参考选项，复制或导出时会保留；原版参考数据未被覆盖。");
  }));
  const source = g.kind === "event" ? card.functions.filter((f) => f.name.startsWith("ExecuteUbergraph_") || f.event) : card.functions.filter((f) => f.name === g.sourceName);
  $("ref-instructions").textContent = JSON.stringify(source, null, 2);
  $("ref-instruction-count").textContent = ` · ${source.reduce((s, f) => s + f.body.length, 0)} 条`;
  $("ref-locate").innerHTML = '<option value="">定位节点…</option>' + g.nodes.map((n) => `<option value="${esc(n.id)}">${esc(nodeChinese(n.definition))} · ${esc(n.definition.name)}</option>`).join("");
  for (const row of $("ref-nodes").querySelectorAll("[data-ref-pin]")) {
    const nodeId = row.closest("[data-ref-node]").dataset.refNode, port = row.dataset.refPin;
    const edge = g.edges.find((e) => e.from.node === nodeId && e.from.pin === port || e.to.node === nodeId && e.to.pin === port);
    if (!edge) continue;
    const target = edge.from.node === nodeId ? edge.to.node : edge.from.node;
    row.classList.add("ref-connected"); row.title = "点击沿连线定位节点";
    row.addEventListener("click", (e) => { if (!e.target.closest("input,select,button")) { e.stopPropagation(); focusNode(target); } });
  }
  requestAnimationFrame(() => { if (refit) focusNode(); else transform(); drawWires(); });
}
$("ref-nodes").addEventListener("click", (e) => {
  const add = e.target.closest("[data-ref-add-pin]"), remove = e.target.closest("[data-ref-remove-pin]");
  if (!add && !remove) return;
  const n = graph().nodes.find((n) => n.id === (add?.dataset.refAddPin || remove.dataset.pinOwner));
  if (!n) return;
  if (add) addDynamicPin(graph(), n, library.enums); else removeDynamicPin(graph(), n, remove.dataset.refRemovePin);
  resolveWildcards(graph()); renderGraph(false);
  status("已修改当前参考副本的引脚；复制或导出时保留，原版源数据未覆盖。");
});
function focusNode(id = focusedNode) {
  const g = graph(); if (!g) return;
  const n = g.nodes.find((n) => n.id === id) || g.nodes.find((n) => n.definition.event || n.definition.bp?.type === "entry") || g.nodes[0];
  if (!n || !$("ref-viewport").clientWidth) return;
  focusedNode = n.id; viewReady = true;
  const viewport = $("ref-viewport");
  view = { x: Math.max(12, (viewport.clientWidth - 310 * .95) / 2) - n.x * .95, y: 28 - n.y * .95, zoom: .95 };
  transform(); $("ref-locate").value = n.id;
}
$("ref-locate").onchange = () => focusNode($("ref-locate").value);
$("ref-readable").onclick = () => focusNode();
function inspect(n) {
  focusedNode = n.id;
  if (narrow.matches) document.querySelector(".ref-layout").dataset.panel = "details";
  $("ref-nodes").querySelectorAll(".selected").forEach((e) => e.classList.remove("selected"));
  [...$("ref-nodes").children].find((e) => e.dataset.refNode === n.id)?.classList.add("selected");
  $("ref-details").innerHTML = `<h3>${esc(n.definition.name)}</h3><p class="ref-muted">${esc(n.definition.signature || "")}</p>${n.definition.reference?.signatureEvidence ? '<p class="ref-muted">参数类型来自源调用反射；缺少原始参数名的输入按 Arg0、Arg1 标记。</p>' : ""}${n.definition.pins.filter((p) => p.type !== "exec").map((p) => `<div class="ref-detail-pin"><strong>${esc(p.name)} · ${esc(p.type)}</strong><code>${esc(n.folded?.[p.id]?.text ?? n.values[p.id] ?? p.direction)}</code></div>`).join("")}<details><summary>源位置与表达式</summary><pre>${esc(JSON.stringify({ source: n.definition.reference, folded: n.folded }, null, 2))}</pre></details>`;
}
function transform() {
  $("ref-stage").style.transform = `translate(${view.x}px,${view.y}px) scale(${view.zoom})`;
  $("ref-zoom").textContent = `${Math.round(view.zoom * 100)}%`;
}
function fit() {
  if (!$("ref-viewport").clientWidth || !$("ref-viewport").clientHeight) return;
  viewReady = true;
  const nodes = graph()?.nodes || []; if (!nodes.length) { view = { x: 30, y: 30, zoom: 1 }; transform(); return; }
  const bounds = $("ref-viewport").getBoundingClientRect();
  const right = Math.max(...nodes.map((n) => n.x + 360)), bottom = Math.max(...[...$("ref-nodes").children].map((e) => parseFloat(e.style.top) + e.offsetHeight));
  view = { x: 20, y: 25, zoom: Math.max(0.035, Math.min(1, (bounds.width - 40) / right, (bounds.height - 50) / bottom)) }; transform();
}
function drawWires() {
  const g = graph(); if (!g) return;
  const elements = new Map([...$("ref-nodes").children].map((e) => [e.dataset.refNode, e]));
  const position = (ep, output) => {
    const el = elements.get(ep.node), n = g.nodes.find((n) => n.id === ep.node), row = [...(el?.querySelectorAll("[data-ref-pin]") || [])].find((e) => e.dataset.refPin === ep.pin);
    if (!row) return null;
    return { x: n.x + (output ? el.offsetWidth - 12 : 13), y: n.y + row.offsetTop + row.offsetHeight / 2 };
  };
  $("ref-wires").innerHTML = g.edges.map((e) => {
    const a = position(e.from, true), b = position(e.to, false); if (!a || !b) return "";
    const bend = Math.max(60, Math.abs(b.x - a.x) * 0.45);
    const type = g.nodes.find((n) => n.id === e.from.node)?.definition.pins.find((p) => p.id === e.from.pin)?.type;
    return `<path style="stroke:${pinColor(type, library.enums)}" class="ref-wire ${type !== "exec" ? "data" : e.from.pin === "false" ? "false" : e.from.pin.startsWith("Case_") ? "case" : ""}" d="M${a.x},${a.y} C${a.x + bend},${a.y} ${b.x - bend},${b.y} ${b.x},${b.y}" />`;
  }).join("");
}
function zoom(delta, point) {
  const old = view.zoom, next = Math.max(0.035, Math.min(2, old * delta));
  const p = point || { x: $("ref-viewport").clientWidth / 2, y: $("ref-viewport").clientHeight / 2 };
  view.x = p.x - (p.x - view.x) * next / old; view.y = p.y - (p.y - view.y) * next / old; view.zoom = next; transform();
}
$("ref-viewport").addEventListener("wheel", (e) => { if (e.target.closest("select,input")) return; e.preventDefault(); const r = $("ref-viewport").getBoundingClientRect(); zoom(e.deltaY > 0 ? 0.9 : 1.1, { x: e.clientX - r.left, y: e.clientY - r.top }); }, { passive: false });
const pointers = new Map();
let gesture, moved = false, suppressClickUntil = 0;
function resetGesture() {
  const ps = [...pointers.values()], r = $("ref-viewport").getBoundingClientRect();
  if (!ps.length) { gesture = null; return; }
  const center = ps.length > 1 ? { x: (ps[0].x + ps[1].x) / 2, y: (ps[0].y + ps[1].y) / 2 } : ps[0];
  gesture = { x: center.x - r.left, y: center.y - r.top, vx: view.x, vy: view.y, zoom: view.zoom, distance: ps.length > 1 ? Math.max(1, Math.hypot(ps[0].x - ps[1].x, ps[0].y - ps[1].y)) : 0 };
}
$("ref-viewport").addEventListener("pointerdown", (e) => {
  if (e.target.closest("input,select,button,a") || (e.pointerType === "mouse" && (e.button !== 0 || e.target.closest(".ref-node")))) return;
  if (!pointers.size) moved = false;
  pointers.set(e.pointerId, { x: e.clientX, y: e.clientY }); resetGesture();
  // Capture only after movement: a stationary tap must still reach its node.
});
$("ref-viewport").addEventListener("pointermove", (e) => {
  if (!pointers.has(e.pointerId) || !gesture) return;
  pointers.set(e.pointerId, { x: e.clientX, y: e.clientY });
  const ps = [...pointers.values()], r = $("ref-viewport").getBoundingClientRect();
  const center = ps.length > 1 ? { x: (ps[0].x + ps[1].x) / 2, y: (ps[0].y + ps[1].y) / 2 } : ps[0];
  const x = center.x - r.left, y = center.y - r.top;
  if (!moved && ps.length === 1 && Math.hypot(x - gesture.x, y - gesture.y) < 5) return;
  moved = true; suppressClickUntil = Date.now() + 400;
  $("ref-viewport").setPointerCapture(e.pointerId);
  const next = ps.length > 1 ? Math.max(0.035, Math.min(2, gesture.zoom * Math.hypot(ps[0].x - ps[1].x, ps[0].y - ps[1].y) / gesture.distance)) : gesture.zoom;
  view.x = x - (gesture.x - gesture.vx) * next / gesture.zoom;
  view.y = y - (gesture.y - gesture.vy) * next / gesture.zoom;
  view.zoom = next; transform();
});
function endPointer(e) {
  if (moved) suppressClickUntil = Date.now() + 400;
  pointers.delete(e.pointerId); resetGesture();
}
for (const name of ["pointerup", "pointercancel", "lostpointercapture"]) $("ref-viewport").addEventListener(name, endPointer);
$("ref-viewport").addEventListener("click", (e) => { if (Date.now() < suppressClickUntil) { e.preventDefault(); e.stopPropagation(); } }, true);
$("ref-fit").onclick = fit; $("ref-zoom-in").onclick = () => zoom(1.2); $("ref-zoom-out").onclick = () => zoom(1 / 1.2);
$("ref-graph").onchange = () => { project.activeGraph = $("ref-graph").value; renderGraph(); };
for (const id of ["ref-search", "ref-faction", "ref-type", "ref-with-logic"]) $(id).addEventListener(id === "ref-search" ? "input" : "change", () => { page = 0; renderList(); });
$("ref-prev").onclick = () => { page -= 1; renderList(); }; $("ref-next").onclick = () => { page += 1; renderList(); };
$("ref-copy").onclick = () => {
  try {
    const copy = validateProject(structuredClone(project)); copy.id = crypto.randomUUID();
    const workspace = createWorkspace(localStorage).load(); copy.name = workspace.uniqueName(copy.name); workspace.save(copy);
    location.href = "./index.html";
  } catch (e) { status(`复制失败：${e.message}`, true); }
};
$("ref-export").onclick = () => {
  try {
    const copy = validateProject(structuredClone(project));
    const url = URL.createObjectURL(new Blob([JSON.stringify(copy, null, 2)], { type: "application/json" }));
    const a = document.createElement("a"); a.href = url; a.download = `${card.name}.kardsbp.json`; a.click(); setTimeout(() => URL.revokeObjectURL(url), 1000);
  } catch (e) { status(`导出失败：${e.message}`, true); }
};
let viewportWidth = innerWidth;
window.addEventListener("resize", () => {
  if (graph() && $("ref-viewport").clientWidth && innerWidth !== viewportWidth) renderGraph(true);
  viewportWidth = innerWidth;
});
try {
  const [response, chineseResponse] = await Promise.all([fetch(snapshot("./reference-data/index.json")), fetch(snapshot("./reference-data/zh-Hans.json"))]); if (!response.ok) throw new Error(`索引加载失败 (${response.status})`);
  if (!chineseResponse.ok) throw new Error(`中文索引加载失败 (${chineseResponse.status})`);
  const chinese = await chineseResponse.json();
  library = await response.json();
  for (const c of library.cards) {
    c.zh = chinese.cards[c.id] || {};
    c.searchIndex = searchText(`${c.name} ${c.title} ${c.text} ${c.zh.title || ""} ${c.zh.text || ""} ${c.package} ${factions[c.faction] || c.faction} ${(c.functionNames || []).join(" ")}`);
  }
  $("ref-total").textContent = `${library.cards.length} 个`;
  for (const [id, key] of [["ref-faction", "faction"], ["ref-type", "type"]]) for (const v of [...new Set(library.cards.map((c) => c[key]))].sort()) { const option = document.createElement("option"); option.value = v; option.textContent = key === "faction" ? factions[v] || v : v; $(id).append(option); }
  renderList(); status(`${library.counts.cards} 张卡 · ${library.counts.functions} 个函数 · ${library.counts.statements} 条源指令 · Codex`);
  const [id, fn] = location.hash.slice(1).split("~");
  const first = library.cards.find((c) => c.id === id) || library.cards.find((c) => c.name === "card_event_stranglehold") || library.cards.find((c) => !c.dataOnly);
  if (first) await openCard(first.id, fn ? decodeURIComponent(fn) : undefined);
  if (narrow.matches && !id) showPanel("library");
} catch (e) { status(`蓝图库加载失败：${e.message}`, true); $("ref-empty").textContent = e.message; }
