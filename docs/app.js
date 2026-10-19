import { setUiLanguage, tr } from "./ui-i18n.js";
import {
  FORMAT,
  VERSION,
  uid,
  clone,
  builtins,
  makeNode,
  makeProject,
  pinKind,
  defaultValue,
  connect,
  validateProject,
  setTypeHierarchy,
  matchingPin,
} from "./model.js";
import { typeNodes, engineClasses } from "./ue-nodes.js";
import {
  allowedInGraph,
  arrayOf,
  buildExport,
  checkName,
  customEventDefinition,
  defaultValueFor,
  dynamicDefinitions,
  elementType,
  isArrayType,
  makeFunctionGraph,
  makeMacroGraph,
  makeVariable,
  newParam,
  overridableFunctions,
  overrideSignature,
  syncDefinitions,
  typeChoices,
} from "./blueprint.js";
import {
  translate,
  nodeKind,
  nodeEnglish,
  nodeChinese,
  kindLabels,
} from "./i18n.js";
const $ = (id) => document.getElementById(id),
  esc = (v) =>
    String(v ?? "").replace(
      /[&<>"']/g,
      (c) =>
        ({
          "&": "&amp;",
          "<": "&lt;",
          ">": "&gt;",
          '"': "&quot;",
          "'": "&#39;",
        })[c],
    );
// Display language: "both" (中文 + English), "zh" or "en". Elements marked .zh / .en are hidden by CSS.
const LANG_KEY = "kards-blueprint-studio.lang";
let langMode = "both";
try {
  const saved = localStorage.getItem(LANG_KEY);
  if (["both", "zh", "en"].includes(saved)) langMode = saved;
} catch {
  /* storage unavailable: keep the default */
}
document.documentElement.dataset.lang = langMode;
const bi = (zh, en) =>
  `<span class="zh">${zh}</span> <span class="en">${en}</span>`;
const STORAGE = "kards-blueprint-studio.v1",
  CAT = {
    all: bi("全部", "All"),
    events: bi("事件", "Events"),
    combat: bi("战斗", "Combat"),
    cards: bi("卡牌", "Cards"),
    effects: bi("效果", "Effects"),
    queries: bi("查询", "Queries"),
    match: bi("对局", "Match"),
    flow: bi("流程", "Flow"),
    variables: bi("变量", "Variables"),
    utilities: bi("工具", "Utilities"),
    properties: bi("属性", "Properties"),
    math: bi("运算", "Math"),
    arrays: bi("数组", "Array"),
    conversion: bi("转换", "Cast"),
    functions: bi("函数", "Functions"),
    macros: bi("宏", "Macros"),
  };
const COLORS = {
  event: "#b52c2c",
  function: "#2469a1",
  pure: "#508d3d",
  flow: "#727272",
  variable: "#68bbaa",
  cast: "#169ca9",
  conversion: "#4b8c4b",
  getter: "#43834a",
  setter: "#43834a",
  functionEntry: "#7b4fa0",
  functionResult: "#7b4fa0",
  macroEntry: "#9a8530",
  macroExit: "#9a8530",
  customEvent: "#b52c2c",
  call: "#2469a1",
  macro: "#727272",
};
const PINCOLORS = {
  exec: "#eeeeee",
  object: "#00a9ef",
  int: "#30e7bd",
  bool: "#d41119",
  string: "#ec3fc4",
  enum: "#0e9f6e",
  float: "#9fdb47",
  name: "#bf83ef",
  text: "#ec96b4",
  vector: "#e3bd20",
  rotator: "#aaa0eb",
  delegate: "#ff3838",
};
// Arrays and sets take the color of their element type, as in Unreal.
function pinColor(type) {
  const inner = /^T(?:Array|Set)\s*<(.*)>$/.exec(type.trim());
  if (inner) return pinColor(inner[1]);
  const kind = pinKind(type);
  return PINCOLORS[kind === "array" ? "object" : kind];
}
const PIN_LEGEND = [
  ["exec", "执行"],
  ["object", "对象"],
  ["bool", "布尔"],
  ["int", "整数"],
  ["float", "浮点"],
  ["string", "字符串"],
  ["name", "Name"],
  ["text", "Text"],
  ["enum", "枚举"],
  ["vector", "向量"],
  ["rotator", "旋转"],
  ["delegate", "委托"],
];
$("pin-legend").innerHTML = PIN_LEGEND.map(
  ([kind, label]) =>
    `<span class="pin-legend"><i style="background:${PINCOLORS[kind]}"></i> ${label}</span>`,
).join("") +
  '<span class="pin-legend" title="数组颜色随元素类型变化"><i class="legend-square" style="background:none;border:1.5px solid #9fb3a6"></i> 数组·随元素类型</span>';
const LABELS = {
  OnPlayedFromHand: "从手牌打出",
  OnStartOfTurn: "回合开始",
  OnStartOfGame: "对局开始",
  OnOtherCardDestroyed: "其他卡牌被摧毁",
  OnReceiveDamage: "受到伤害",
  OnSuppressed: "被压制",
  GiveBlitz: "赋予闪击",
  GiveGuard: "赋予守护",
  GiveAmbush: "赋予伏击",
  GiveFury: "赋予奋战",
  GiveSmokescreen: "赋予烟幕",
  GiveKreditsBySide: "给予指挥点",
  DamageCard: "对卡牌造成伤害",
  DamageMultipleCards: "造成群体伤害",
  DrawCardsFromDeckBySide: "从牌组抽牌",
  SpawnCardOnBattlefield: "在战场生成卡牌",
  SpawnCardInHandBySide: "在手牌生成卡牌",
  ShuffleDeckBySide: "洗牌",
  SuppressUnit: "压制单位",
  TriggerDestruction: "触发亡语",
  TriggerDeployment: "触发部署",
  Branch: "条件分支",
  Sequence: "顺序执行",
  Boolean: "布尔变量",
  Integer: "整数变量",
  String: "文本变量",
  TakeControlOfEnemyUnit: "控制敌方单位",
  RemoveCardFromBoard: "移除战场卡牌",
  SetCountdown: "设置倒计时",
  RevealCard: "揭示卡牌",
  ResetUnitOperations: "重置行动",
  ShowNotification: "显示通知",
};
const FACTIONS = {
  Germany: "德国",
  Britain: "英国",
  Japan: "日本",
  Soviet: "苏联",
  USA: "美国",
  France: "法国",
  Italy: "意大利",
  Poland: "波兰",
  Finland: "芬兰",
  Anzac: "澳新军团",
  Allies: "盟军",
  Neutral: "中立",
  NotAvailable: "未指定",
};
let catalog = [],
  repository = null,
  enums = {},
  project,
  category = "all",
  selection = new Set(),
  selectedEdge = null,
  pending = null,
  history = [],
  future = [],
  toastTimer,
  saveTimer,
  drag = null,
  space = false,
  placement = null,
  pickerContext = null,
  classes = {};
const graph = () => project.graphs.find((g) => g.id === project.activeGraph),
  // Static catalog plus this project's variables, functions, macros and custom events,
  // limited to what the active graph may contain.
  fullCatalog = () =>
    [...catalog, ...dynamicDefinitions(project, graph())].filter((d) =>
      allowedInGraph(d, graph()),
    ),
  color = (d) => COLORS[nodeKind(d)] || "#2469a1",
  icon = (d) =>
    ({
      event: "ϟ",
      flow: "◇",
      variable: "≡",
      pure: "ƒ",
      getter: "●",
      setter: "●",
      cast: "↪",
      conversion: "⇄",
      function: "ƒ",
      functionEntry: "→",
      functionResult: "←",
      macroEntry: "⇥",
      macroExit: "⇤",
      customEvent: "ϟ",
      call: "ƒ",
      macro: "⧉",
    })[nodeKind(d)] || "ƒ",
  title = (n) =>
    n.title
      ? (nodeKind(n.definition) === "event" && !n.title.startsWith("事件")
          ? "事件 · "
          : "") + n.title
      : nodeChinese(n.definition);
// Primary node name for the current language; a custom title the user typed is always shown.
const nodeHeading = (n) =>
  langMode === "en" && !n.title ? nodeEnglish(n.definition) : title(n);
function toast(message, error = false) {
  $("toast").textContent = message;
  $("toast").classList.toggle("error", error);
  $("toast").hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => ($("toast").hidden = true), 4200);
}
function persist() {
  try {
    localStorage.setItem(STORAGE, JSON.stringify(project));
    $("save-state").textContent = "本地已保存";
  } catch {
    $("save-state").textContent = "存储空间不足，请导出";
  }
}
function scheduleSave() {
  $("save-state").textContent = "正在保存…";
  clearTimeout(saveTimer);
  saveTimer = setTimeout(persist, 300);
}
function checkpoint() {
  history.push(JSON.stringify(project));
  if (history.length > 70) history.shift();
  future = [];
}
function mutate(fn) {
  checkpoint();
  fn();
  syncDefinitions(project);
  pending = null;
  render();
  persist();
}
function undo() {
  if (!history.length) return;
  future.push(JSON.stringify(project));
  project = JSON.parse(history.pop());
  selection.clear();
  pending = null;
  selectedEdge = null;
  render();
  persist();
}
function redo() {
  if (!future.length) return;
  history.push(JSON.stringify(project));
  project = JSON.parse(future.pop());
  selection.clear();
  pending = null;
  selectedEdge = null;
  render();
  persist();
}
function openModal(name, body, subtitle = "") {
  $("modal").classList.remove("context-menu");
  $("modal").style.cssText = "";
  $("modal-content").innerHTML =
    `<div class="modal-head"><div><h2>${esc(name)}</h2>${subtitle ? `<p>${esc(subtitle)}</p>` : ""}</div><button class="icon-button" id="close-modal" aria-label="关闭">×</button></div><div class="modal-body">${body}</div>`;
  $("close-modal").onclick = () => $("modal").close();
  if (!$("modal").open) $("modal").showModal();
}
function render() {
  project.types = { ...classes, ...project.types };
  setTypeHierarchy(project.types);
  $("project-name").value = project.name;
  $("project-label").textContent = project.card.title;
  $("card-breadcrumb").textContent = project.card.title;
  $("graph-breadcrumb").textContent = graph().name;
  $("canvas-title").textContent = graph().name;
  $("graph-list").innerHTML = myBlueprintMarkup();
  $("tabs").innerHTML =
    project.graphs
      .map(
        (g) =>
          `<button class="tab ${g.id === project.activeGraph ? "active" : ""}" data-graph="${esc(g.id)}"><span class="graph-icon">${GRAPH_ICON[g.kind]}</span>${esc(g.name)}<span class="tab-dot"></span></button>`,
      )
      .join("") +
    '<button class="tab add-tab" id="tab-add" title="新建函数 / 宏 / 变量">＋</button>';
  $("canvas-subtitle").textContent = GRAPH_SUBTITLE[graph().kind];
  document
    .querySelectorAll("[data-graph]")
    .forEach((b) => (b.onclick = () => switchGraph(b.dataset.graph)));
  $("tab-add").onclick = newItemMenu;
  $("tabs")
    .querySelectorAll("[data-graph]")
    .forEach((b) => (b.ondblclick = () => editGraph(b.dataset.graph)));
  bindMyBlueprint();
  renderNodes();
  renderInspector();
  updateView();
  $("undo").disabled = !history.length;
  $("redo").disabled = !future.length;
  $("graph-stats").textContent =
    `${graph().nodes.length} 个节点 · ${graph().edges.length} 条连接`;
  $("empty-state").hidden = graph().nodes.length > 0;
  updateSelectionStatus();
  requestAnimationFrame(() => {
    drawEdges();
    drawMinimap();
  });
}
function switchGraph(id) {
  project.activeGraph = id;
  selection.clear();
  selectedEdge = null;
  pending = null;
  render();
  persist();
}
// ---------------------------------------------------------------- blueprint structure UI
const GRAPH_ICON = { event: "⌘", function: "ƒ", macro: "⧉" };
const GRAPH_SUBTITLE = {
  event: "Event Graph · 卡牌事件图",
  function: "Function · 函数",
  macro: "Macro · 宏",
};
const graphById = (id) => project.graphs.find((g) => g.id === id);
// Every top-level name already in use (variables, graphs, dispatchers, custom events, parent members).
const usedNames = ({ graph: g, variable: v, dispatcher: d, events = true } = {}) => [
  ...project.variables.filter((x) => x !== v).map((x) => x.name),
  ...project.graphs.filter((x) => x !== g).map((x) => x.name),
  ...project.dispatchers.filter((x) => x !== d).map((x) => x.name),
  ...(events
    ? project.graphs[0].nodes
        .filter((n) => n.definition.bp?.type === "customEvent")
        .map((n) => n.definition.name)
    : []),
  ...parentMembers(),
];
const classMap = () => ({ ...classes, ...project.types });
// Names already used by the parent class; a blueprint may not redefine them.
const parentMembers = () =>
  catalog.filter((d) => d.class === project.asset.parentClass).map((d) => d.name);
const bi2 = (zh, en) => `<span class="zh">${zh}</span><span class="en">${en}</span>`;

function myBlueprintMarkup() {
  const head = (zh, en, buttons = "") =>
    `<div class="bp-head"><span>${bi2(zh, en)}</span><span class="bp-actions">${buttons}</span></div>`;
  const add = (kind, title, icon = "＋") =>
    `<button class="tiny-button" data-new="${kind}" title="${title}">${icon}</button>`;
  const row = (g) =>
    `<div class="bp-row"><button class="graph-link ${g.id === project.activeGraph ? "active" : ""}" data-graph="${esc(g.id)}"><span class="graph-icon">${GRAPH_ICON[g.kind]}</span>${esc(g.name)}${g.override ? '<em class="bp-tag">override</em>' : ""}<span>${g.nodes.length}</span></button>${g.kind === "event" ? "" : `<button class="tiny-button" data-edit-graph="${esc(g.id)}" title="编辑">✎</button>`}</div>`;
  const events = project.graphs[0].nodes.filter(
    (n) => n.definition.bp?.type === "customEvent",
  );
  const local = (v) =>
    `<div class="bp-row"><button class="graph-link var-link" data-edit-graph="${esc(graph().id)}" title="${esc(v.type)}"><i class="var-dot" style="background:${pinColor(v.type)}"></i>${esc(v.name)}<span class="var-type">${esc(v.type)}</span></button><button class="tiny-button" data-add-var="getLocal:${esc(v.id)}" title="获取">G</button><button class="tiny-button" data-add-var="setLocal:${esc(v.id)}" title="设置">S</button></div>`;
  const dispatcher = (d) =>
    `<div class="bp-row"><button class="graph-link var-link" data-dispatcher="${esc(d.id)}" title="${esc(d.description || d.name)}"><span class="graph-icon">◉</span>${esc(d.name)}<span class="var-type">${d.params.length}</span></button><button class="tiny-button" data-add-dispatch="call:${esc(d.id)}" title="调用">C</button><button class="tiny-button" data-add-dispatch="bind:${esc(d.id)}" title="绑定事件">B</button><button class="tiny-button" data-add-dispatch="unbind:${esc(d.id)}" title="解除绑定">U</button></div>`;
  const variable = (v) =>
    `<div class="bp-row"><button class="graph-link var-link" data-var="${esc(v.id)}" title="${esc(v.tooltip || v.type)}"><i class="var-dot" style="background:${pinColor(v.type)}"></i>${esc(v.name)}<span class="var-type">${esc(v.type)}</span></button><button class="tiny-button" data-add-var="get:${esc(v.id)}" title="获取">G</button><button class="tiny-button" data-add-var="set:${esc(v.id)}" title="设置">S</button></div>`;
  return (
    head("图表", "Graphs") +
    row(project.graphs[0]) +
    events
      .map(
        (n) =>
          `<div class="bp-row bp-sub"><button class="graph-link" data-event-node="${esc(n.id)}"><span class="graph-icon">ϟ</span>${esc(n.definition.name)}</button></div>`,
      )
      .join("") +
    head(
      "函数",
      "Functions",
      add("function", "新建函数") + add("override", "重载父类函数", "⤴"),
    ) +
    project.graphs.filter((g) => g.kind === "function").map(row).join("") +
    head("宏", "Macros", add("macro", "新建宏")) +
    project.graphs.filter((g) => g.kind === "macro").map(row).join("") +
    head("变量", "Variables", add("variable", "新建变量")) +
    project.variables.map(variable).join("") +
    (graph().kind === "function"
      ? head("局部变量", "Local variables") +
        graph().locals.map(local).join("")
      : "") +
    head("事件分发器", "Event dispatchers", add("dispatcher", "新建事件分发器")) +
    project.dispatchers.map(dispatcher).join("")
  );
}
function bindMyBlueprint() {
  document
    .querySelectorAll("[data-new]")
    .forEach((b) => (b.onclick = () => newItem(b.dataset.new)));
  document
    .querySelectorAll("[data-edit-graph]")
    .forEach((b) => (b.onclick = () => editGraph(b.dataset.editGraph)));
  document
    .querySelectorAll("[data-var]")
    .forEach((b) => (b.onclick = () => variableDialog(b.dataset.var)));
  document
    .querySelectorAll("[data-dispatcher]")
    .forEach((b) => (b.onclick = () => dispatcherDialog(b.dataset.dispatcher)));
  document.querySelectorAll("[data-add-dispatch]").forEach(
    (b) =>
      (b.onclick = () => {
        const [action, id] = b.dataset.addDispatch.split(":");
        addBlueprintNode(`bp.dispatch.${action}.${id}`);
        if (window.innerWidth <= 800) setDrawer(false);
      }),
  );
  document.querySelectorAll("[data-add-var]").forEach(
    (b) =>
      (b.onclick = () => {
        const [kind, id] = b.dataset.addVar.split(":");
        addVariableNode(kind, id);
        if (window.innerWidth <= 800) setDrawer(false);
      }),
  );
  document.querySelectorAll("[data-event-node]").forEach(
    (b) =>
      (b.onclick = () => {
        project.activeGraph = project.graphs[0].id;
        selection = new Set([b.dataset.eventNode]);
        selectedEdge = null;
        render();
        persist();
      }),
  );
}
function addBlueprintNode(defId) {
  if (!fullCatalog().some((d) => d.id === defId))
    return toast("当前图表不能使用此节点", true);
  addNode(defId);
}
const addVariableNode = (kind, id) =>
  addBlueprintNode(`bp.${kind.toLowerCase()}.${id}`);
function newItem(kind) {
  if ($("modal").open) $("modal").close();
  if (kind === "function" || kind === "macro") functionDialog(null, kind);
  else if (kind === "override") overrideDialog();
  else if (kind === "dispatcher") dispatcherDialog(null);
  else variableDialog(null);
}
function newItemMenu() {
  const item = (kind, zh, en) =>
    `<button class="secondary-button new-item" data-new-item="${kind}">${bi2(zh, en)}</button>`;
  openModal(
    "新建",
    `<div class="new-menu">${item("function", "新建函数", "New function")}${item("override", "重载父类函数", "Override parent function")}${item("macro", "新建宏", "New macro")}${item("variable", "新建变量", "New variable")}${item("dispatcher", "新建事件分发器", "New event dispatcher")}</div><p class="detail-description">EventGraph 是唯一的事件图，不能再新建。更多逻辑请写在函数、宏或自定义事件里。</p>`,
  );
  $("modal-content")
    .querySelectorAll("[data-new-item]")
    .forEach((b) => (b.onclick = () => newItem(b.dataset.newItem)));
}
function editGraph(id) {
  const g = graphById(id);
  if (!g) return;
  if (g.kind === "event")
    return openModal(
      "EventGraph",
      "<p>EventGraph 是蓝图唯一的事件图，不能新建、重命名或删除。</p><p>事件（如“从手牌打出”）和自定义事件都放在这里；更多逻辑请新建函数或宏。</p>",
    );
  functionDialog(id, g.kind);
}

// ---- parameter lists (functions, macros, custom events)
function typeSelect(current, { allowExec = false, locked = false } = {}) {
  const base = elementType(current);
  const choices = [
    ...(allowExec ? ["exec"] : []),
    ...typeChoices(enums, classMap()),
  ];
  if (!choices.includes(base)) choices.push(base);
  return `<select class="param-type" ${locked ? "disabled" : ""}>${choices.map((t) => `<option ${t === base ? "selected" : ""}>${esc(t)}</option>`).join("")}</select><label class="param-array"><input type="checkbox" ${isArrayType(current) ? "checked" : ""} ${locked ? "disabled" : ""}> ${bi2("数组", "Array")}</label>`;
}
function paramRow(p, opts) {
  return `<div class="param-row" data-param="${esc(p.id)}"><input class="param-name" maxlength="64" value="${esc(p.name)}" placeholder="Name" ${opts.locked ? "disabled" : ""}>${typeSelect(p.type, opts)}${opts.withDefault ? defaultControl(p.type, p.default ?? defaultValueFor(p.type, enums), 'class="param-default"') : ""}${opts.locked ? "" : '<button type="button" class="tiny-button param-del" title="删除">×</button>'}</div>`;
}
function paramEditor(listId, zh, en, params, opts = {}) {
  return `<div class="field"><div class="detail-label">${bi2(zh, en)}</div><div id="${listId}" class="param-list" data-allow-exec="${opts.allowExec ? 1 : 0}" data-with-default="${opts.withDefault ? 1 : 0}">${params.map((p) => paramRow(p, opts)).join("")}</div>${opts.locked ? "" : `<button type="button" class="secondary-button param-add" data-list="${listId}">＋ ${bi2("添加参数", "Add parameter")}</button>`}</div>`;
}
function bindParamEditors() {
  const root = $("modal-content");
  root.onchange = (e) => {
    const row = e.target.closest(".param-row");
    if (!row || !row.querySelector(".param-default")) return;
    if (!e.target.matches(".param-type, .param-array input")) return;
    const base = row.querySelector(".param-type").value,
      type = row.querySelector(".param-array input").checked ? arrayOf(base) : base;
    row.querySelector(".param-default").outerHTML = defaultControl(
      type,
      defaultValueFor(type, enums),
      'class="param-default"',
    );
  };
  root.onclick = (e) => {
    const del = e.target.closest(".param-del");
    if (del) return del.closest(".param-row").remove();
    const add = e.target.closest(".param-add");
    if (!add) return;
    const list = $(add.dataset.list),
      n = list.querySelectorAll(".param-row").length + 1;
    list.insertAdjacentHTML(
      "beforeend",
      paramRow(newParam(list.dataset.withDefault === "1" ? `Local${n}` : `Param${n}`, "bool"), {
        allowExec: list.dataset.allowExec === "1",
        withDefault: list.dataset.withDefault === "1",
      }),
    );
  };
}
function readParams(listId, label) {
  const seen = [],
    out = [];
  for (const row of $(listId).querySelectorAll(".param-row")) {
    const name = row.querySelector(".param-name").value.trim(),
      error = checkName(name, seen);
    if (error) {
      toast(`${label}：${error}`, true);
      return null;
    }
    seen.push(name);
    const base = row.querySelector(".param-type").value,
      array = row.querySelector(".param-array input").checked && base !== "exec";
    const type = array ? arrayOf(base) : base,
      entry = { id: row.dataset.param, name, type };
    if ($(listId).dataset.withDefault === "1")
      entry.default = parseDefaultValue(type, row.querySelector(".param-default").value);
    out.push(entry);
  }
  return out;
}

// ---- functions and macros
function functionDialog(id, kind = "function") {
  const g = id ? graphById(id) : null;
  kind = g ? g.kind : kind;
  const locked = !!g?.override,
    isFn = kind === "function",
    draft = g || {
      description: "",
      pure: false,
      access: "Public",
      category: "Default",
      inputs: [],
      outputs: [],
    };
  const title = g
    ? isFn
      ? "编辑函数"
      : "编辑宏"
    : isFn
      ? "新建函数"
      : "新建宏";
  openModal(
    title,
    `<div class="field"><label class="detail-label" for="fn-name">名称</label><input id="fn-name" maxlength="64" value="${esc(g ? g.name : isFn ? "NewFunction" : "NewMacro")}" ${locked ? "disabled" : ""}><p class="detail-description">名称只能包含英文字母、数字和下划线，不能以数字开头（不支持中文）。</p></div><div class="field"><label class="detail-label" for="fn-desc">说明</label><textarea id="fn-desc" placeholder="可以用中文写说明">${esc(draft.description)}</textarea></div>${isFn ? `<div class="field-grid"><div class="field"><label class="detail-label" for="fn-access">访问</label><select id="fn-access">${["Public", "Protected", "Private"].map((v) => `<option ${v === draft.access ? "selected" : ""}>${v}</option>`).join("")}</select></div><div class="field"><label class="detail-label" for="fn-category">分类</label><input id="fn-category" maxlength="64" value="${esc(draft.category)}"></div></div>${locked ? "" : `<label class="check-row"><input type="checkbox" id="fn-pure" ${draft.pure ? "checked" : ""}> 纯函数（没有执行引脚）</label>`}` : ""}${paramEditor("fn-inputs", "输入参数", "Inputs", draft.inputs, { allowExec: !isFn, locked })}${paramEditor("fn-outputs", "输出参数", "Outputs", draft.outputs, { allowExec: !isFn, locked })}${isFn ? paramEditor("fn-locals", "局部变量", "Local variables", draft.locals || [], { withDefault: true }) : ""}<div class="modal-actions">${g ? '<button id="fn-delete" class="danger-button" style="width:auto;margin:0">删除</button>' : ""}<button id="fn-save" class="primary-button">${g ? "保存" : "创建"}</button></div>`,
  );
  bindParamEditors();
  $("fn-save").onclick = () => {
    const name = $("fn-name").value.trim();
    if (!locked) {
      const taken = [
        ...usedNames({ graph: g }),
      ];
      const error = checkName(name, taken);
      if (error) return toast(error, true);
    }
    const inputs = readParams("fn-inputs", "输入参数");
    if (!inputs) return;
    const outputs = readParams("fn-outputs", "输出参数");
    if (!outputs) return;
    const common = {
      description: $("fn-desc").value.trim(),
      inputs,
      outputs,
    };
    if (isFn) {
      const locals = readParams("fn-locals", "局部变量");
      if (!locals) return;
      const used = new Set([...inputs, ...outputs].map((p) => p.name.toLowerCase()));
      if (locals.some((l) => used.has(l.name.toLowerCase())))
        return toast("局部变量不能和输入、输出参数重名", true);
      common.locals = locals;
      common.access = $("fn-access").value;
      common.category = $("fn-category").value.trim() || "Default";
      common.pure = locked ? g.pure : $("fn-pure").checked;
    }
    mutate(() => {
      if (g) Object.assign(g, { name, ...common });
      else {
        const ng = isFn
          ? makeFunctionGraph(name, common)
          : makeMacroGraph(name, common);
        project.graphs.push(ng);
        project.activeGraph = ng.id;
        selection.clear();
      }
    });
    $("modal").close();
  };
  if ($("fn-delete"))
    $("fn-delete").onclick = () => {
      if (!confirm(tr(`删除“${g.name}”？调用它的节点也会被一并删除，可使用撤销恢复。`)))
        return;
      mutate(() => {
        project.graphs = project.graphs.filter((x) => x !== g);
        if (project.activeGraph === g.id)
          project.activeGraph = project.graphs[0].id;
        selection.clear();
      });
      $("modal").close();
    };
}
function overrideDialog() {
  const used = new Set(
      project.graphs.filter((g) => g.override).map((g) => g.override.function),
    ),
    list = overridableFunctions(catalog, project.asset.parentClass).filter(
      (d) => !used.has(d.name),
    );
  if (!list.length)
    return openModal("重载父类函数", "<p>没有可以重载的父类函数了。</p>");
  const sigText = (d) => {
    const s = overrideSignature(d);
    return `${s.pure ? "pure " : ""}${d.name}(${s.inputs.map((p) => `${p.type} ${p.name}`).join(", ")})${s.outputs.length ? ` -> (${s.outputs.map((p) => `${p.type} ${p.name}`).join(", ")})` : ""}`;
  };
  openModal(
    "重载父类函数",
    `<div class="field"><label class="detail-label" for="ovr-fn">${esc(project.asset.parentClass)}</label><select id="ovr-fn">${list.map((d) => `<option>${esc(d.name)}</option>`).join("")}</select></div><pre id="ovr-sig" class="detail-source"></pre><div class="modal-actions"><button id="ovr-create" class="primary-button">创建重载</button></div>`,
  );
  const show = () =>
    ($("ovr-sig").textContent = sigText(
      list.find((d) => d.name === $("ovr-fn").value),
    ));
  $("ovr-fn").onchange = show;
  show();
  $("ovr-create").onclick = () => {
    const def = list.find((d) => d.name === $("ovr-fn").value),
      sig = overrideSignature(def);
    mutate(() => {
      const g = makeFunctionGraph(def.name, {
        override: {
          class: def.class,
          function: def.name,
          source: def.source || null,
        },
        pure: sig.pure,
        inputs: sig.inputs,
        outputs: sig.outputs,
        description: `Override of ${def.class}::${def.name}`,
      });
      project.graphs.push(g);
      project.activeGraph = g.id;
      selection.clear();
    });
    $("modal").close();
  };
}

// ---- variables
// Input for a default value that matches the type: switch for bool, list for enums,
// number box for int/float, text otherwise. `attrs` is e.g. `id="var-default"`.
function defaultControl(type, value, attrs = 'id="var-default"') {
  const base = elementType(type),
    kind = pinKind(base);
  if (isArrayType(type)) return `<input ${attrs} value="${esc(value)}">`;
  const list = kind === "bool" ? ["false", "true"] : enums[base];
  if (list)
    return `<select ${attrs}>${list.map((v) => `<option ${String(value) === v ? "selected" : ""}>${esc(v)}</option>`).join("")}</select>`;
  if (kind === "int" || kind === "float")
    return `<input ${attrs} type="number" step="${kind === "int" ? 1 : "any"}" value="${esc(value)}">`;
  return `<input ${attrs} value="${esc(value)}">`;
}
function parseDefaultValue(type, raw) {
  const base = elementType(type),
    kind = pinKind(base);
  if (isArrayType(type)) return raw;
  if (kind === "bool") return raw === "true";
  if (kind === "int" || kind === "float") {
    const n = Number(raw);
    return Number.isFinite(n) ? (kind === "int" ? Math.trunc(n) : n) : 0;
  }
  return raw;
}
const readDefault = (type) => parseDefaultValue(type, $("var-default").value);
function variableDialog(id) {
  const v = id ? project.variables.find((x) => x.id === id) : null;
  let n = project.variables.length + 1,
    suggestion = `NewVar${n}`;
  while (project.variables.some((x) => x.name === suggestion))
    suggestion = `NewVar${++n}`;
  const draft = v || makeVariable(suggestion, "bool", enums);
  openModal(
    v ? "编辑变量" : "新建变量",
    `<div class="field"><label class="detail-label" for="var-name">名称</label><input id="var-name" maxlength="64" value="${esc(draft.name)}"><p class="detail-description">名称只能包含英文字母、数字和下划线，不能以数字开头（不支持中文）。</p></div><div class="field-grid"><div class="field"><div class="detail-label">${bi2("类型", "Type")}</div><div class="param-row">${typeSelect(draft.type)}</div></div></div><div class="field"><div class="detail-label">${bi2("默认值", "Default value")}</div><div id="var-default-wrap">${defaultControl(draft.type, draft.default)}</div></div><div class="field"><label class="detail-label" for="var-category">分类</label><input id="var-category" maxlength="64" value="${esc(draft.category)}"></div><div class="field"><label class="detail-label" for="var-tip">说明</label><textarea id="var-tip" placeholder="可以用中文写说明">${esc(draft.tooltip)}</textarea></div><label class="check-row"><input type="checkbox" id="var-editable" ${draft.instanceEditable ? "checked" : ""}> 实例可编辑</label><label class="check-row"><input type="checkbox" id="var-expose" ${draft.exposeOnSpawn ? "checked" : ""}> 生成时公开</label><div class="modal-actions">${v ? '<button id="var-delete" class="danger-button" style="width:auto;margin:0">删除</button>' : ""}<button id="var-save" class="primary-button">${v ? "保存" : "创建"}</button></div>`,
  );
  const typeNow = () => {
    const root = $("modal-content"),
      base = root.querySelector(".param-type").value;
    return root.querySelector(".param-array input").checked ? arrayOf(base) : base;
  };
  const refreshDefault = () => {
    const type = typeNow();
    $("var-default-wrap").innerHTML = defaultControl(type, defaultValueFor(type, enums));
  };
  $("modal-content").querySelector(".param-type").onchange = refreshDefault;
  $("modal-content").querySelector(".param-array input").onchange = refreshDefault;
  $("var-save").onclick = () => {
    const name = $("var-name").value.trim(),
      taken = usedNames({ variable: v }),
      error = checkName(name, taken);
    if (error) return toast(error, true);
    const type = typeNow(),
      values = {
        name,
        type,
        default: readDefault(type),
        category: $("var-category").value.trim() || "Default",
        tooltip: $("var-tip").value.trim(),
        instanceEditable: $("var-editable").checked,
        exposeOnSpawn: $("var-expose").checked,
      };
    mutate(() => {
      if (v) Object.assign(v, values);
      else project.variables.push({ ...draft, ...values });
    });
    $("modal").close();
  };
  if ($("var-delete"))
    $("var-delete").onclick = () => {
      if (!confirm(tr(`删除“${v.name}”？使用它的获取/设置节点也会被一并删除，可使用撤销恢复。`)))
        return;
      mutate(() => {
        project.variables = project.variables.filter((x) => x !== v);
      });
      $("modal").close();
    };
}

// ---- custom events
function customEventDialog(node, point) {
  const params = node ? node.definition.bp.params : [];
  openModal(
    node ? "编辑自定义事件" : "添加自定义事件",
    `<div class="field"><label class="detail-label" for="ce-name">名称</label><input id="ce-name" maxlength="64" value="${esc(node ? node.definition.name : "NewEvent")}"><p class="detail-description">名称只能包含英文字母、数字和下划线，不能以数字开头（不支持中文）。</p></div>${paramEditor("ce-params", "参数", "Parameters", params)}<div class="modal-actions"><button id="ce-save" class="primary-button">${node ? "保存" : "创建"}</button></div>`,
  );
  bindParamEditors();
  $("ce-save").onclick = () => {
    const name = $("ce-name").value.trim(),
      events = project.graphs[0].nodes.filter(
        (n) => n.definition.bp?.type === "customEvent" && n !== node,
      ),
      taken = [
        ...usedNames({ events: false }),
        ...events.map((n) => n.definition.name),
      ],
      error = checkName(name, taken);
    if (error) return toast(error, true);
    const list = readParams("ce-params", "参数");
    if (!list) return;
    mutate(() => {
      if (node) node.definition = customEventDefinition(name, list, node.definition);
      else {
        const at = point || freeSpotNearCenter();
        const n = makeNode(
          customEventDefinition(name, list),
          Math.round(at.x / 10) * 10,
          Math.round(at.y / 10) * 10,
        );
        graph().nodes.push(n);
        selection = new Set([n.id]);
        selectedEdge = null;
      }
    });
    $("modal").close();
  };
}

// ---- event dispatchers
function dispatcherDialog(id) {
  const d = id ? project.dispatchers.find((x) => x.id === id) : null;
  openModal(
    d ? "编辑事件分发器" : "新建事件分发器",
    `<div class="field"><label class="detail-label" for="ed-name">名称</label><input id="ed-name" maxlength="64" value="${esc(d ? d.name : `OnEvent${project.dispatchers.length + 1}`)}"><p class="detail-description">名称只能包含英文字母、数字和下划线，不能以数字开头（不支持中文）。</p></div><div class="field"><label class="detail-label" for="ed-desc">说明</label><textarea id="ed-desc" placeholder="可以用中文写说明">${esc(d?.description || "")}</textarea></div>${paramEditor("ed-params", "参数", "Parameters", d ? d.params : [])}<div class="modal-actions">${d ? '<button id="ed-delete" class="danger-button" style="width:auto;margin:0">删除</button>' : ""}<button id="ed-save" class="primary-button">${d ? "保存" : "创建"}</button></div>`,
  );
  bindParamEditors();
  $("ed-save").onclick = () => {
    const name = $("ed-name").value.trim(),
      error = checkName(name, usedNames({ dispatcher: d }));
    if (error) return toast(error, true);
    const params = readParams("ed-params", "参数");
    if (!params) return;
    const values = { name, params, description: $("ed-desc").value.trim() };
    mutate(() => {
      if (d) Object.assign(d, values);
      else project.dispatchers.push({ id: uid(), ...values });
    });
    $("modal").close();
  };
  if ($("ed-delete"))
    $("ed-delete").onclick = () => {
      if (!confirm(tr(`删除“${d.name}”？使用它的调用、绑定节点也会被一并删除，可使用撤销恢复。`)))
        return;
      mutate(() => {
        project.dispatchers = project.dispatchers.filter((x) => x !== d);
      });
      $("modal").close();
    };
}

// ---- inspector helpers
function assetSection() {
  const a = project.asset;
  return `<section class="detail-section"><h3 class="detail-heading">资源 <span>ASSET</span></h3>${field("资源名称", "asset.name", project.name)}<div class="field"><label class="detail-label">父类<input value="${esc(a.parentClass)}" disabled></label></div>${field("包路径", "asset.packagePath", a.packagePath)}<div class="field"><label class="detail-label">说明<textarea data-field="asset.description" placeholder="可以用中文写说明">${esc(a.description)}</textarea></label></div></section>`;
}
function graphSection() {
  const g = graph(),
    sig = (list) => list.map((p) => `${p.type} ${p.name}`).join(", ") || "—";
  if (g.kind === "event")
    return `<section class="detail-section"><h3 class="detail-heading">图表 <span>GRAPH</span></h3><p class="detail-description">EventGraph 是蓝图唯一的事件图，不能新建、重命名或删除。点击节点可编辑参数和注释。</p></section>`;
  return `<section class="detail-section"><h3 class="detail-heading">${g.kind === "macro" ? "宏" : "函数"} <span>${g.kind === "macro" ? "MACRO" : "FUNCTION"}</span></h3><div class="detail-source">${esc(g.name)}${g.override ? `\n(override ${esc(g.override.class)}::${esc(g.override.function)})` : ""}\n in: ${esc(sig(g.inputs))}\nout: ${esc(sig(g.outputs))}</div><button class="secondary-button" id="graph-settings" style="width:100%">编辑</button></section>`;
}
function bpNodeSection(n) {
  const t = n.definition.bp?.type;
  if (!t) return "";
  const label = {
    customEvent: "编辑自定义事件",
    entry: "编辑函数签名",
    result: "编辑函数签名",
    macroEntry: "编辑宏签名",
    macroExit: "编辑宏签名",
    call: "转到函数",
    macro: "转到宏",
    callEvent: "转到事件",
    get: "编辑变量",
    set: "编辑变量",
    getLocal: "编辑函数（局部变量）",
    setLocal: "编辑函数（局部变量）",
    dispatcher: "编辑事件分发器",
  }[t];
  return `<section class="detail-section"><h3 class="detail-heading">蓝图 <span>BLUEPRINT</span></h3><button class="secondary-button" id="bp-edit" style="width:100%">${label}</button></section>`;
}
function editBlueprintNode(n) {
  const bp = n.definition.bp;
  if (bp.type === "customEvent") return customEventDialog(n);
  if (["entry", "result", "macroEntry", "macroExit"].includes(bp.type))
    return functionDialog(bp.graphId, graphById(bp.graphId).kind);
  if (bp.type === "call" || bp.type === "macro") return switchGraph(bp.graphId);
  if (bp.type === "getLocal" || bp.type === "setLocal")
    return functionDialog(bp.graphId, "function");
  if (bp.type === "dispatcher") return dispatcherDialog(bp.dispatcherId);
  if (bp.type === "callEvent") {
    project.activeGraph = project.graphs[0].id;
    selection = new Set([bp.eventNodeId]);
    render();
    return persist();
  }
  return variableDialog(bp.variableId);
}
function libraryItem(d) {
  return `<button class="library-node" draggable="true" data-def="${esc(d.id)}" title="${esc(d.signature || nodeEnglish(d))}" style="--node-color:${color(d)}"><span class="lib-icon">${icon(d)}</span><span><strong>${esc(langMode === "zh" ? nodeChinese(d) : nodeEnglish(d))}</strong><small>${langMode === "both" ? esc(nodeChinese(d)) + " " : ""}<span class="lib-owner">${esc((d.class || "UE").replace(/^[UA](?=[A-Z][a-z])/, "").replace("Stub", ""))}</span></small></span></button>`;
}
function filterCatalog(q, cat = "all") {
  q = q.trim().toLowerCase();
  const items = fullCatalog().filter(
    (d) =>
      (cat === "all" || d.category === cat) &&
      `${nodeEnglish(d)} ${d.class} ${nodeChinese(d)} ${d.pins.map((p) => p.name).join(" ")}`
        .toLowerCase()
        .includes(q),
  );
  return items.sort(
    (a, b) => Number(!!LABELS[b.name]) - Number(!!LABELS[a.name]),
  );
}
function renderLibrary() {
  const results = filterCatalog($("node-search").value, category);
  $("category-list").innerHTML = Object.entries(CAT)
    .map(
      ([key, label]) =>
        `<button class="category ${category === key ? "active" : ""}" data-category="${key}">${label}</button>`,
    )
    .join("");
  $("category-list")
    .querySelectorAll("button")
    .forEach(
      (b) =>
        (b.onclick = () => {
          category = b.dataset.category;
          renderLibrary();
        }),
    );
  $("library-label").innerHTML = `${CAT[category]} · ${results.length}`;
  $("node-list").innerHTML = results.length
    ? results.slice(0, 120).map(libraryItem).join("") +
      (results.length > 120
        ? '<div class="no-results">仅显示前 120 项，输入关键词继续查找</div>'
        : "")
    : '<div class="no-results">没有匹配的节点</div>';
  bindLibrary($("node-list"));
}
function bindLibrary(container) {
  container.querySelectorAll("[data-def]").forEach((b) => {
    b.ondragstart = (e) => {
      e.dataTransfer.setData("application/x-kards-node", b.dataset.def);
      e.dataTransfer.effectAllowed = "copy";
    };
    b.ondblclick = () => addNode(b.dataset.def);
    b.onclick = () => {
      if (window.innerWidth <= 800) {
        addNode(b.dataset.def);
        setDrawer(false);
      } else $("selection-status").textContent = "双击添加节点，或拖入画布";
    };
  });
}
// Centre of the view, nudged diagonally so new nodes do not land exactly on existing ones.
function freeSpotNearCenter() {
  const at = centerPoint();
  at.x = Math.round(at.x / 10) * 10;
  at.y = Math.round(at.y / 10) * 10;
  while (
    graph().nodes.some(
      (n) => Math.abs(n.x - at.x) < 30 && Math.abs(n.y - at.y) < 30,
    )
  ) {
    at.x += 40;
    at.y += 40;
  }
  return at;
}
function centerPoint() {
  const r = $("viewport").getBoundingClientRect(),
    v = graph().view;
  return {
    x: (r.width / 2 - v.x) / v.zoom - 140,
    y: (r.height / 2 - v.y) / v.zoom - 70,
  };
}
function addNode(id, point, sourceEndpoint = null) {
  const def = fullCatalog().find((d) => d.id === id);
  if (!def) return;
  point = point || freeSpotNearCenter();
  if (def.action === "newCustomEvent") return customEventDialog(null, point);
  mutate(() => {
    const n = makeNode(
      def,
      Math.round(point.x / 10) * 10,
      Math.round(point.y / 10) * 10,
    );
    graph().nodes.push(n);
    if (sourceEndpoint) {
      const sourceNode = graph().nodes.find(
        (node) => node.id === sourceEndpoint.node,
      );
      const sourcePin = sourceNode?.definition.pins.find(
        (p) => p.id === sourceEndpoint.pin,
      );
      const targetPin = sourcePin && matchingPin(n.definition, sourcePin);
      if (targetPin) {
        connect(graph(), sourceEndpoint, { node: n.id, pin: targetPin.id });
        if (sourcePin.direction === "out" && sourcePin.type !== "exec") {
          const exec = sourceNode.definition.pins.find(
            (p) =>
              p.direction === "out" && p.type === "exec" && p.id !== "Failed",
          );
          const into = n.definition.pins.find(
            (p) => p.direction === "in" && p.type === "exec",
          );
          if (
            exec &&
            into &&
            !graph().edges.some(
              (e) => e.from.node === sourceNode.id && e.from.pin === exec.id,
            )
          )
            connect(
              graph(),
              { node: sourceNode.id, pin: exec.id },
              { node: n.id, pin: into.id },
            );
        }
      }
    }
    selection = new Set([n.id]);
    selectedEdge = null;
  });
}
function picker(point, sourceEndpoint = null) {
  placement = point || centerPoint();
  pickerContext = sourceEndpoint ? clone(sourceEndpoint) : null;
  const sourceNode =
    sourceEndpoint && graph().nodes.find((n) => n.id === sourceEndpoint.node);
  const sourcePin = sourceNode?.definition.pins.find(
    (p) => p.id === sourceEndpoint.pin,
  );
  openModal(
    sourcePin
      ? "此引脚的可用操作 / Actions for this Pin"
      : "添加节点 / All Actions",
    `${sourcePin ? `<div class="context-type"><i style="background:${pinColor(sourcePin.type)}"></i><strong>${esc(sourcePin.type)}</strong><span>${esc(translate(sourcePin.name))} · ${esc(sourcePin.name)}</span></div>` : ""}<input id="picker-search" class="modal-search" placeholder="搜索 / Search actions…" aria-label="搜索可添加节点"><div class="context-options"><label><input type="checkbox" id="context-sensitive" checked> 上下文关联 / Context Sensitive</label>${sourcePin ? '<select id="context-scope"><option value="all">所有兼容操作 / All Compatible</option><option value="members">属性和成员 / Members</option><option value="functions">成员函数 / Functions</option><option value="properties">读取/写入属性 / Properties</option><option value="casts">类型转换 / Cast & Convert</option></select>' : ""}</div><div id="picker-count"></div><div id="picker-results" class="picker-results"></div><div class="modal-actions"><button id="custom-node" class="secondary-button">＋ 自定义节点 / Custom Node</button></div>`,
    sourcePin
      ? "选择节点后自动连接此引脚 / Automatically connect compatible pins"
      : `${fullCatalog().length} 个节点 · KARDS + Unreal Blueprint`,
  );
  const update = () => {
    let matches = filterCatalog($("picker-search").value);
    if (sourcePin && $("context-sensitive").checked)
      matches = matches.filter((d) => matchingPin(d, sourcePin));
    const scope = $("context-scope")?.value;
    if (scope === "members")
      matches = matches.filter((d) => d.pins.some((p) => p.id === "target"));
    if (scope === "functions")
      matches = matches.filter(
        (d) => !d.property && d.pins.some((p) => p.id === "target"),
      );
    if (scope === "properties") matches = matches.filter((d) => d.property);
    if (scope === "casts")
      matches = matches.filter((d) =>
        ["cast", "conversion"].includes(nodeKind(d)),
      );
    if (sourcePin) {
      const rank = (d) =>
        matchingPin(d, sourcePin)?.id === "target"
          ? d.property
            ? 2
            : 3
          : nodeKind(d) === "cast"
            ? 1
            : 0;
      matches.sort((a, b) => rank(b) - rank(a));
    }
    $("picker-count").textContent = `${matches.length} 个操作 / Actions`;
    $("picker-results").innerHTML =
      matches.slice(0, 100).map(libraryItem).join("") ||
      '<div class="no-results">无兼容操作 / No compatible actions</div>';
    $("picker-results")
      .querySelectorAll("[data-def]")
      .forEach((b) => {
        b.draggable = false;
        b.onclick = () => {
          if (Date.now() < ignoreBackdropUntil) return;
          $("modal").close();
          addNode(b.dataset.def, placement, pickerContext);
        };
      });
  };
  $("picker-search").oninput = update;
  $("context-sensitive").onchange = update;
  if ($("context-scope")) $("context-scope").onchange = update;
  $("picker-search").onkeydown = (e) => {
    if (e.key === "Enter") $("picker-results").querySelector("button")?.click();
  };
  $("custom-node").onclick = customNode;
  update();
  const r = viewport.getBoundingClientRect(),
    view = graph().view,
    w = Math.min(490, window.innerWidth - 24),
    h = Math.min(620, window.innerHeight - 32);
  const screenX = r.left + placement.x * view.zoom + view.x,
    screenY = r.top + placement.y * view.zoom + view.y;
  $("modal").classList.add("context-menu");
  $("modal").style.cssText =
    `margin:0;width:${w}px;max-height:${h}px;left:${Math.max(12, Math.min(screenX, window.innerWidth - w - 12))}px;top:${Math.max(12, Math.min(screenY, window.innerHeight - h - 12))}px;`;
  $("picker-search").focus();
}
function customNode() {
  openModal(
    "自定义节点",
    `<div class="field"><label class="detail-label" for="custom-name">节点名称</label><input id="custom-name" value="CustomAction" maxlength="100"></div><div class="field"><label class="detail-label" for="custom-category">节点类型</label><select id="custom-category"><option value="custom">动作（含执行端口）</option><option value="events">事件（执行输出）</option><option value="variables">纯数据节点</option></select></div><div class="field"><label class="detail-label" for="custom-pins">数据端口：每行「方向 类型 名称」</label><textarea id="custom-pins" class="custom-ports" spellcheck="false">in int32 Amount\nout bool Success</textarea></div><p>方向为 in / out。支持 bool、int32、float、FString、UBaseCardObject* 及 KARDS 枚举。相同类型才能连线。</p><div class="modal-actions"><button id="create-custom" class="primary-button">创建节点</button></div>`,
  );
  $("create-custom").onclick = () => {
    const name = $("custom-name").value.trim(),
      cat = $("custom-category").value;
    if (!name) return toast("请输入节点名称", true);
    let pins = [];
    if (cat === "custom")
      pins.push({ id: "exec-in", name: "执行", type: "exec", direction: "in" });
    if (cat !== "variables")
      pins.push({
        id: "exec-out",
        name: "然后",
        type: "exec",
        direction: "out",
      });
    const lines = $("custom-pins")
      .value.split("\n")
      .filter((l) => l.trim());
    for (const line of lines) {
      const m = line.trim().match(/^(in|out)\s+(\S+)\s+(\w+)$/);
      if (!m || pins.some((p) => p.id === m?.[3]) || m[2] === "exec")
        return toast("端口格式错误、名称重复或使用了保留类型 exec", true);
      pins.push({ id: m[3], name: m[3], type: m[2], direction: m[1] });
    }
    if (pins.length > 100) return toast("自定义节点最多支持 100 个端口", true);
    const def = {
      id: "custom." + uid(),
      name,
      class: "Custom",
      category: cat,
      pure: cat === "variables",
      pins,
    };
    mutate(() => {
      const n = makeNode(def, placement?.x || 100, placement?.y || 150);
      graph().nodes.push(n);
      selection = new Set([n.id]);
    });
    $("modal").close();
  };
}
function pinMarkup(n, p) {
  const connected = graph().edges.some(
    (e) =>
      (e.from.node === n.id && e.from.pin === p.id) ||
      (e.to.node === n.id && e.to.pin === p.id),
  );
  const val = n.values[p.id] ?? defaultValue(p.type, enums);
  const button = `<button class="pin ${p.type === "exec" ? "exec" : ""} ${pinKind(p.type) === "array" ? "array" : ""} ${connected ? "connected" : ""} ${pending?.node === n.id && pending.pin === p.id ? "pending" : ""}" style="--pin-color:${pinColor(p.type)}" data-node="${esc(n.id)}" data-pin="${esc(p.id)}" title="${esc(p.name)} · ${esc(p.type)} · ${p.direction === "in" ? "输入" : "输出"}" aria-label="${esc(p.name)} ${p.direction === "in" ? "输入" : "输出"}端口"></button>`;
  const cn = translate(p.name),
    raw = pinEnglish(p);
  const label = `<span class="pin-label"><span class="pin-cn zh">${esc(cn)}</span><span class="pin-en en">${esc(raw)}</span></span>`;
  const showValue =
    !connected &&
    p.type !== "exec" &&
    pinKind(p.type) !== "delegate" &&
    !(p.id === "target" && val === "Self") &&
    (p.direction === "in" ||
      (n.definition.category === "variables" && n.definition.kind === "variable"));
  return `<div class="pin-row ${p.direction === "out" ? "output" : ""}">${p.direction === "in" ? button : ""}${label}${showValue ? (typeof val === "boolean" ? `<button class="pin-value pin-toggle" data-toggle="${esc(p.id)}" data-toggle-node="${esc(n.id)}" title="点击切换 true / false">${val ? "true" : "false"}</button>` : `<span class="pin-value">${esc(val)}</span>`) : ""}${p.direction === "out" ? button : ""}</div>`;
}
function nodeBody(n) {
  const ins = n.definition.pins.filter((p) => p.direction === "in"),
    outs = n.definition.pins.filter((p) => p.direction === "out");
  return Array.from(
    { length: Math.max(ins.length, outs.length) },
    (_, i) =>
      `<div class="pin-pair">${ins[i] ? pinMarkup(n, ins[i]) : '<div class="pin-row"></div>'}${outs[i] ? pinMarkup(n, outs[i]) : '<div class="pin-row output"></div>'}</div>`,
  ).join("");
}
// Visual size class: simple data nodes (Get Self, Get Card ID, literals) stay small, while
// busy nodes (Change Attack and friends) get larger ports and rows that are easier to wire.
function nodeTier(d) {
  const ins = d.pins.filter((p) => p.direction === "in").length,
    outs = d.pins.filter((p) => p.direction === "out").length,
    data = d.pins.filter((p) => p.type !== "exec" && p.id !== "target").length,
    hasExec = d.pins.some((p) => p.type === "exec"),
    rows = Math.max(ins, outs);
  if (rows >= 6 || d.pins.length >= 9) return "lg";
  if (!hasExec && data <= 2) return "xs";
  if ((!hasExec && data <= 4) || (hasExec && rows <= 2)) return "sm";
  return "md";
}
// Nodes size themselves to their content, so redraw wires whenever one changes size.
let relayoutFrame = 0;
const nodeObserver = new ResizeObserver(() => {
  cancelAnimationFrame(relayoutFrame);
  relayoutFrame = requestAnimationFrame(() => {
    drawEdges();
    drawMinimap();
  });
});
function renderNodes() {
  $("nodes").innerHTML = graph()
    .nodes.map(
      (n) =>
        `<article class="node ue-${nodeKind(n.definition)} tier-${nodeTier(n.definition)} ${selection.has(n.id) ? "selected" : ""}" data-node-id="${esc(n.id)}" style="left:${n.x}px;top:${n.y}px;--node-color:${color(n.definition)}">${n.comment ? `<div class="node-comment">${esc(n.comment)}</div>` : ""}<div class="node-head" title="${esc(kindLabels[nodeKind(n.definition)] || "FUNCTION")} · ${esc((n.definition.class || "UE").replace(/^[UA](?=[A-Z][a-z])/, ""))}"><div class="node-heading"><span>${icon(n.definition)}</span><strong>${esc(nodeHeading(n))}</strong></div>${langMode === "both" ? `<div class="node-subtitle">${esc(nodeEnglish(n.definition))}</div>` : ""}</div><div class="node-kind-label">${esc(kindLabels[nodeKind(n.definition)] || "FUNCTION")}<span>${esc((n.definition.class || "UE").replace(/^[UA](?=[A-Z][a-z])/, ""))}</span></div><div class="node-body">${nodeBody(n)}</div></article>`,
    )
    .join("");
  nodeObserver.disconnect();
  $("nodes")
    .querySelectorAll(".node")
    .forEach((el) => nodeObserver.observe(el));
}
function updateSelectionStatus() {
  const bar = $("selection-bar");
  if (bar) {
    bar.hidden = !(selection.size || selectedEdge);
    $("bar-duplicate").hidden = !selection.size;
  }
  $("selection-status").textContent = pending
    ? "选择一个兼容端口完成连线 · Esc 取消"
    : selectedEdge
      ? "已选中连接 · Delete 删除"
      : selection.size
        ? `已选中 ${selection.size} 个节点`
        : "就绪";
}
function selectNode(id, add = false) {
  if (!add) selection.clear();
  if (add && selection.has(id)) selection.delete(id);
  else selection.add(id);
  selectedEdge = null;
  document
    .querySelectorAll(".node")
    .forEach((n) =>
      n.classList.toggle("selected", selection.has(n.dataset.nodeId)),
    );
  renderInspector();
  updateSelectionStatus();
}
function field(label, key, value, options) {
  let input;
  if (options) {
    const opts = [
      ...new Set([
        ...options.map((o) => (typeof o === "string" ? o : o.value)),
        String(value),
      ]),
    ];
    input = `<select data-field="${esc(key)}">${opts.map((v) => `<option value="${esc(v)}" ${String(value) === v ? "selected" : ""}>${esc(options.find((o) => typeof o === "object" && o.value === v)?.label || v)}</option>`).join("")}</select>`;
  } else
    input = `<input data-field="${esc(key)}" value="${esc(value)}" ${typeof value === "number" ? 'type="number"' : ""}>`;
  return `<div class="field"><label class="detail-label">${esc(label)}${input}</label></div>`;
}
function renderInspector() {
  let body = "";
  const n = graph().nodes.find((n) => selection.has(n.id));
  if (selectedEdge) {
    body =
      '<section class="detail-section"><h3 class="detail-heading">连接详情</h3><p class="detail-description">选中连线后，可按 Delete 删除。每个输入端口只接受一条连接，新的连接会替换原连接。</p><button class="danger-button" id="delete-selection">删除连接</button></section>';
  } else if (n) {
    const d = n.definition;
    body = `<section class="detail-section"><h3 class="detail-heading">节点 <span>${selection.size > 1 ? selection.size + " SELECTED" : "NODE"}</span></h3><span class="node-type-badge">${esc(kindLabels[nodeKind(d)] || "自定义")} / ${esc(d.class || "Custom")}</span>${field("显示名称", "title", n.title || nodeChinese(d))}<div class="field"><label class="detail-label">节点注释<textarea data-field="comment" placeholder="描述这段逻辑的意图…">${esc(n.comment)}</textarea></label></div><div class="field-grid">${field("位置 X", "x", n.x)}${field("位置 Y", "y", n.y)}</div></section><section class="detail-section"><h3 class="detail-heading">默认参数 <span>DEFAULTS</span></h3>`;
    const inputs = d.pins.filter(
      (p) =>
        p.type !== "exec" &&
        pinKind(p.type) !== "delegate" &&
        (p.direction === "in" || (d.category === "variables" && d.kind === "variable")),
    );
    body += inputs.length
      ? inputs
          .map((p) => {
            const val = n.values[p.id] ?? defaultValue(p.type, enums),
              isConnected = graph().edges.some(
                (e) => e.to.node === n.id && e.to.pin === p.id,
              );
            const options =
              p.type === "bool" ? ["false", "true"] : enums[p.type];
            return `<div class="field"><label class="detail-label">${esc(translate(p.name))} / ${esc(p.name)} <span style="opacity:.5">${esc(p.type)}</span>${options ? `<select data-value="${esc(p.id)}" ${isConnected ? "disabled" : ""}>${[...new Set([...options, String(val)])].map((v) => `<option ${String(v) === String(val) ? "selected" : ""}>${esc(v)}</option>`).join("")}</select>` : `<input data-value="${esc(p.id)}" value="${esc(val)}" ${["int", "float"].includes(pinKind(p.type)) ? 'type="number" step="any"' : ""} ${isConnected ? "disabled" : ""}>`}${isConnected ? '<span class="detail-description">由连接提供值 · 请在来源节点上修改</span>' : ""}</label></div>`;
          })
          .join("")
      : '<p class="detail-description">此节点没有可编辑的默认输入。</p>';
    body += `</section>${bpNodeSection(n)}<section class="detail-section"><h3 class="detail-heading">源代码 <span>NATIVE</span></h3><div class="detail-source">${esc(d.signature || d.name)}${d.source ? `<p><a href="https://github.com/ghhhhughg638/KardsSource/blob/main/${encodeURI(d.source)}" target="_blank" rel="noreferrer">${esc(d.source)} ↗</a></p>` : "<p>Studio 自定义节点 · 仅用于可视化设计</p>"}</div><button class="secondary-button" id="duplicate" style="width:100%;margin-top:10px">复制节点 <kbd>Ctrl D</kbd></button><button class="danger-button" id="delete-selection">删除选中节点</button></section>`;
  } else {
    const card = project.card;
    body = `${assetSection()}<section class="detail-section"><h3 class="detail-heading">卡牌信息 <span>CARD DATA</span></h3><div class="card-preview"><div class="card-emblem">${esc(card.cost)}</div><div><small>${esc(card.faction.toUpperCase())} / ${esc(card.type.toUpperCase())}</small><h3>${esc(card.title)}</h3><p>${esc(FACTIONS[card.faction] || card.faction)} · ${esc(card.rarity)}</p></div></div>${field("卡牌名称", "card.title", card.title)}${field(
      "所属阵营",
      "card.faction",
      card.faction,
      (enums.EFactionEnum || Object.keys(FACTIONS)).map((value) => ({
        value,
        label: FACTIONS[value] || value,
      })),
    )}${field("卡牌类型", "card.type", card.type, enums.ETypeEnum || ["infantry", "tank", "order"])}${field("稀有度", "card.rarity", card.rarity, enums.ERarityEnum || ["Common", "Uncommon", "Rare", "Unique"])}<div class="field-grid">${field("攻击力", "card.attack", card.attack)}${field("防御力", "card.defense", card.defense)}</div><div style="height:12px"></div>${field("指挥点消耗", "card.cost", card.cost)}<div class="field"><label class="detail-label">卡牌效果<textarea data-field="card.text">${esc(card.text)}</textarea></label></div></section>${graphSection()}<section class="detail-section"><h3 class="detail-heading">文件格式 <span>v1.0</span></h3><p class="detail-description">.kardsbp.json 包含资源信息、变量、函数、宏、EventGraph、节点定义、参数和连线。导入后可继续编辑。</p></section>`;
  }
  $("inspector-content").innerHTML = body;
  $("inspector-content")
    .querySelectorAll("[data-field]")
    .forEach(
      (input) =>
        (input.onchange = () => {
          const key = input.dataset.field;
          let val = input.type === "number" ? Number(input.value) : input.value;
          if (typeof val === "number" && !Number.isFinite(val)) return;
          if (key === "asset.name") {
            const error = checkName(val.trim());
            if (error) {
              toast(error, true);
              return renderInspector();
            }
            val = val.trim();
          }
          mutate(() => {
            if (key.startsWith("card.")) project.card[key.slice(5)] = val;
            else if (key === "asset.name") project.name = val;
            else if (key.startsWith("asset.")) project.asset[key.slice(6)] = val;
            else if (n) n[key] = val;
          });
        }),
    );
  $("inspector-content")
    .querySelectorAll("[data-value]")
    .forEach(
      (input) =>
        (input.onchange = () => {
          const p = n.definition.pins.find((p) => p.id === input.dataset.value);
          let val =
            p.type === "bool"
              ? input.value === "true"
              : ["int", "float"].includes(pinKind(p.type))
                ? Number(input.value)
                : input.value;
          if (typeof val === "number" && !Number.isFinite(val)) return;
          mutate(() => (n.values[p.id] = val));
        }),
    );
  if ($("delete-selection")) $("delete-selection").onclick = deleteSelection;
  if ($("duplicate")) $("duplicate").onclick = duplicate;
  if ($("graph-settings"))
    $("graph-settings").onclick = () => editGraph(project.activeGraph);
  if ($("bp-edit")) $("bp-edit").onclick = () => editBlueprintNode(n);
}
function pinPosition(endpoint) {
  const node = $("nodes").querySelector(
    `[data-node-id="${CSS.escape(endpoint.node)}"]`,
  );
  const pin = node?.querySelector(`[data-pin="${CSS.escape(endpoint.pin)}"]`);
  if (!pin) return null;
  const r = pin.getBoundingClientRect(),
    v = $("viewport").getBoundingClientRect(),
    view = graph().view;
  return {
    x: (r.x + r.width / 2 - v.x - view.x) / view.zoom,
    y: (r.y + r.height / 2 - v.y - view.y) / view.zoom,
  };
}
function curve(a, b) {
  const dx = Math.max(65, Math.abs(b.x - a.x) * 0.5);
  return `M ${a.x} ${a.y} C ${a.x + dx} ${a.y}, ${b.x - dx} ${b.y}, ${b.x} ${b.y}`;
}
function drawEdges(pointer) {
  let html = "";
  for (const e of graph().edges) {
    const a = pinPosition(e.from),
      b = pinPosition(e.to);
    if (!a || !b) continue;
    const pin = graph()
      .nodes.find((n) => n.id === e.from.node)
      ?.definition.pins.find((p) => p.id === e.from.pin);
    html += `<path class="edge ${selectedEdge === e.id ? "selected" : ""}" data-edge="${esc(e.id)}" d="${curve(a, b)}" stroke="${pinColor(pin.type)}"/>`;
  }
  if (pending && pointer) {
    const p = pinPosition(pending),
      def = graph()
        .nodes.find((n) => n.id === pending.node)
        ?.definition.pins.find((p) => p.id === pending.pin);
    if (p)
      html += `<path class="edge ghost" d="${def.direction === "out" ? curve(p, pointer) : curve(pointer, p)}" stroke="${pinColor(def.type)}"/>`;
  }
  $("edges").innerHTML = html;
}
function updateView() {
  const v = graph().view;
  $("world").style.transform = `translate(${v.x}px,${v.y}px) scale(${v.zoom})`;
  $("viewport").style.backgroundSize =
    `${16 * v.zoom}px ${16 * v.zoom}px, ${16 * v.zoom}px ${16 * v.zoom}px, ${128 * v.zoom}px ${128 * v.zoom}px, ${128 * v.zoom}px ${128 * v.zoom}px`;
  $("viewport").style.backgroundPosition = `${v.x}px ${v.y}px`;
  $("zoom-label").textContent = `${Math.round(v.zoom * 100)}%`;
}
function worldPoint(e) {
  const r = $("viewport").getBoundingClientRect(),
    v = graph().view;
  return {
    x: (e.clientX - r.left - v.x) / v.zoom,
    y: (e.clientY - r.top - v.y) / v.zoom,
  };
}
function zoomAt(factor, cx, cy) {
  const v = graph().view,
    r = $("viewport").getBoundingClientRect();
  cx ??= r.width / 2;
  cy ??= r.height / 2;
  const z = Math.max(0.2, Math.min(2, v.zoom * factor));
  v.x = cx - ((cx - v.x) * z) / v.zoom;
  v.y = cy - ((cy - v.y) * z) / v.zoom;
  v.zoom = z;
  updateView();
  drawMinimap();
  scheduleSave();
}
const nodeEl = (n) =>
  document.querySelector(`[data-node-id="${CSS.escape(n.id)}"]`);
const nodeWidth = (n) => nodeEl(n)?.offsetWidth || 280;
function bounds() {
  if (!graph().nodes.length) return { x: 0, y: 0, w: 900, h: 600 };
  const ns = graph().nodes;
  const x = Math.min(...ns.map((n) => n.x)) - 30,
    y = Math.min(...ns.map((n) => n.y)) - 55;
  return {
    x,
    y,
    w: Math.max(...ns.map((n) => n.x + nodeWidth(n))) - x + 30,
    h:
      Math.max(
        ...ns.map(
          (n) =>
            n.y +
            (document.querySelector(`[data-node-id="${CSS.escape(n.id)}"]`)
              ?.offsetHeight || 180),
        ),
      ) -
      y +
      35,
  };
}
function fit() {
  if (!graph().nodes.length) {
    graph().view = { x: 40, y: 0, zoom: 1 };
    updateView();
    return;
  }
  const narrow = window.innerWidth <= 800,
    b = bounds(),
    r = $("viewport").getBoundingClientRect(),
    z = Math.max(
      0.2,
      Math.min(
        1.15,
        (r.width - (narrow ? 24 : 70)) / b.w,
        (r.height - (narrow ? 80 : 145)) / b.h,
      ),
    );
  graph().view = {
    x: (r.width - b.w * z) / 2 - b.x * z,
    y: (r.height - b.h * z) / 2 - b.y * z + 15,
    zoom: z,
  };
  updateView();
  drawMinimap();
  scheduleSave();
}
let miniBounds;
function drawMinimap() {
  const b = bounds();
  miniBounds = { x: b.x - 50, y: b.y - 50, w: b.w + 100, h: b.h + 100 };
  const m = miniBounds,
    v = graph().view,
    r = $("viewport").getBoundingClientRect();
  $("minimap").setAttribute("viewBox", `${m.x} ${m.y} ${m.w} ${m.h}`);
  $("minimap").innerHTML =
    graph()
      .nodes.map(
        (n) =>
          `<rect x="${n.x}" y="${n.y}" width="${nodeWidth(n)}" height="${document.querySelector(`[data-node-id="${CSS.escape(n.id)}"]`)?.offsetHeight || 150}" rx="5" fill="${color(n.definition)}" fill-opacity=".3" stroke="${color(n.definition)}" stroke-width="2"/>`,
      )
      .join("") +
    `<rect x="${-v.x / v.zoom}" y="${-v.y / v.zoom}" width="${r.width / v.zoom}" height="${r.height / v.zoom}" fill="#dddddd" fill-opacity=".03" stroke="#c7c7c7" stroke-width="3"/>`;
}
function deleteSelection() {
  if (!selection.size && !selectedEdge) return;
  mutate(() => {
    const doomed = new Set(
      graph()
        .nodes.filter((n) => selection.has(n.id) && !n.definition.fixed)
        .map((n) => n.id),
    );
    graph().nodes = graph().nodes.filter((n) => !doomed.has(n.id));
    graph().edges = graph().edges.filter(
      (e) =>
        !doomed.has(e.from.node) &&
        !doomed.has(e.to.node) &&
        e.id !== selectedEdge,
    );
    selection.clear();
    selectedEdge = null;
  });
}
function duplicate() {
  if (!selection.size) return;
  mutate(() => {
    const copied = graph().nodes.filter(
        (n) =>
          selection.has(n.id) &&
          !n.definition.fixed &&
          n.definition.kind !== "customEvent",
      ),
      ids = new Map();
    for (const n of copied) {
      const copy = clone(n);
      copy.id = uid();
      copy.x += 40;
      copy.y += 40;
      ids.set(n.id, copy.id);
      graph().nodes.push(copy);
    }
    const edges = graph().edges.filter(
      (e) => ids.has(e.from.node) && ids.has(e.to.node),
    );
    graph().edges.push(
      ...edges.map((e) => ({
        id: uid(),
        from: { node: ids.get(e.from.node), pin: e.from.pin },
        to: { node: ids.get(e.to.node), pin: e.to.pin },
      })),
    );
    selection = new Set(ids.values());
  });
}
function tryConnect(a, b) {
  try {
    checkpoint();
    if (!connect(graph(), a, b)) history.pop();
    pending = null;
    render();
    persist();
  } catch (e) {
    history.pop();
    pending = null;
    renderNodes();
    drawEdges();
    toast(e.message, true);
    updateSelectionStatus();
  }
}
const viewport = $("viewport");
// Two-finger pinch: zoom around the midpoint and pan with it.
const touches = new Map();
let pinch = null;
const touchPair = () => {
  const [a, b] = [...touches.values()];
  return {
    d: Math.hypot(a.x - b.x, a.y - b.y) || 1,
    x: (a.x + b.x) / 2,
    y: (a.y + b.y) / 2,
  };
};
viewport.addEventListener(
  "pointerdown",
  (e) => {
    if (e.pointerType !== "touch") return;
    touches.set(e.pointerId, { x: e.clientX, y: e.clientY });
    if (touches.size === 2) {
      drag = null;
      pending = null;
      viewport.classList.remove("panning");
      pinch = touchPair();
      drawEdges();
    }
    if (touches.size >= 2) e.stopImmediatePropagation();
  },
  true,
);
viewport.addEventListener(
  "pointermove",
  (e) => {
    if (!touches.has(e.pointerId)) return;
    touches.set(e.pointerId, { x: e.clientX, y: e.clientY });
    if (!pinch || touches.size !== 2) return;
    e.stopImmediatePropagation();
    const now = touchPair(),
      r = viewport.getBoundingClientRect();
    zoomAt(now.d / pinch.d, now.x - r.left, now.y - r.top);
    graph().view.x += now.x - pinch.x;
    graph().view.y += now.y - pinch.y;
    updateView();
    drawMinimap();
    pinch = now;
  },
  true,
);
// Long-press on empty canvas opens the node picker (touch devices have no right-click).
let pressTimer = 0,
  ignoreBackdropUntil = 0;
const cancelPress = () => clearTimeout(pressTimer);
viewport.addEventListener("pointerdown", (e) => {
  cancelPress();
  if (
    e.pointerType !== "touch" ||
    touches.size !== 1 ||
    e.target.closest(".node,.pin,[data-edge],.canvas-bottom,.minimap-wrap,.empty-state,.selection-bar")
  )
    return;
  const { clientX, clientY } = e;
  pressTimer = setTimeout(() => {
    if (touches.size !== 1) return;
    drag = null;
    viewport.classList.remove("panning");
    // The finger is still down: its release must not count as a click outside the picker.
    ignoreBackdropUntil = Infinity;
    picker(worldPoint({ clientX, clientY }), pending);
  }, 550);
});
viewport.addEventListener("pointermove", (e) => {
  if (e.pointerType === "touch" && pressTimer && drag?.type === "pan") {
    if (Math.hypot(e.clientX - drag.startX, e.clientY - drag.startY) > 8)
      cancelPress();
  }
});
const releasePress = () => {
  cancelPress();
  if (ignoreBackdropUntil === Infinity) ignoreBackdropUntil = Date.now() + 500;
};
window.addEventListener("pointerup", releasePress, true);
window.addEventListener("pointercancel", releasePress, true);
const endTouch = (e) => {
  if (!touches.delete(e.pointerId)) return;
  if (touches.size < 2) pinch = null;
};
window.addEventListener("pointerup", endTouch, true);
window.addEventListener("pointercancel", endTouch, true);
viewport.addEventListener("pointerdown", (e) => {
  if (e.button === 2) return;
  if (e.target.closest(".canvas-bottom,.minimap-wrap,.empty-state,.selection-bar"))
    return;
  const toggle = e.target.closest("[data-toggle]");
  if (toggle && e.button === 0) {
    e.preventDefault();
    const n = graph().nodes.find((n) => n.id === toggle.dataset.toggleNode);
    if (n) {
      const id = toggle.dataset.toggle;
      mutate(() => (n.values[id] = !n.values[id]));
    }
    return;
  }
  const pin = e.target.closest(".pin");
  if (pin && e.button === 0) {
    e.preventDefault();
    const endpoint = { node: pin.dataset.node, pin: pin.dataset.pin };
    if (
      pending &&
      (pending.node !== endpoint.node || pending.pin !== endpoint.pin)
    ) {
      tryConnect(pending, endpoint);
      return;
    }
    pending = endpoint;
    drag = { type: "wire", startX: e.clientX, startY: e.clientY };
    renderNodes();
    drawEdges(worldPoint(e));
    updateSelectionStatus();
    return;
  }
  const edge = e.target.closest("[data-edge]");
  if (edge && e.button === 0 && !space) {
    selection.clear();
    selectedEdge = edge.dataset.edge;
    renderNodes();
    renderInspector();
    drawEdges();
    updateSelectionStatus();
    return;
  }
  const node = e.target.closest(".node");
  if (node && e.button === 0 && !space) {
    if (!selection.has(node.dataset.nodeId) || e.shiftKey)
      selectNode(node.dataset.nodeId, e.shiftKey);
    if (e.target.closest(".node-head")) {
      drag = {
        type: "nodes",
        startX: e.clientX,
        startY: e.clientY,
        positions: graph()
          .nodes.filter((n) => selection.has(n.id))
          .map((n) => ({ id: n.id, x: n.x, y: n.y })),
        changed: false,
      };
      viewport.setPointerCapture(e.pointerId);
    }
    return;
  }
  if (e.button === 0 || e.button === 1) {
    if (e.button === 0 && !space) {
      selection.clear();
      selectedEdge = null;
      pending = null;
      renderNodes();
      renderInspector();
      drawEdges();
      updateSelectionStatus();
    }
    drag = {
      type: "pan",
      startX: e.clientX,
      startY: e.clientY,
      x: graph().view.x,
      y: graph().view.y,
    };
    viewport.classList.add("panning");
    viewport.setPointerCapture(e.pointerId);
    e.preventDefault();
  }
});
viewport.addEventListener("pointermove", (e) => {
  if (pending) drawEdges(worldPoint(e));
  if (!drag) return;
  const dx = e.clientX - drag.startX,
    dy = e.clientY - drag.startY;
  if (drag.type === "pan") {
    graph().view.x = drag.x + dx;
    graph().view.y = drag.y + dy;
    updateView();
    drawMinimap();
  } else if (
    drag.type === "nodes" &&
    (drag.changed || Math.abs(dx) + Math.abs(dy) > 3)
  ) {
    if (!drag.changed) {
      checkpoint();
      drag.changed = true;
    }
    for (const p of drag.positions) {
      const n = graph().nodes.find((n) => n.id === p.id);
      n.x = p.x + dx / graph().view.zoom;
      n.y = p.y + dy / graph().view.zoom;
      const el = document.querySelector(`[data-node-id="${CSS.escape(n.id)}"]`);
      el.style.left = n.x + "px";
      el.style.top = n.y + "px";
    }
    drawEdges();
    drawMinimap();
  }
});
window.addEventListener("pointerup", (e) => {
  if (!drag) return;
  if (drag.type === "wire") {
    const hit = document
      .elementFromPoint(e.clientX, e.clientY)
      ?.closest(".pin");
    if (
      hit &&
      pending &&
      (hit.dataset.node !== pending.node || hit.dataset.pin !== pending.pin)
    )
      tryConnect(pending, { node: hit.dataset.node, pin: hit.dataset.pin });
    else if (
      pending &&
      !hit &&
      Math.hypot(e.clientX - drag.startX, e.clientY - drag.startY) > 7 &&
      viewport.contains(document.elementFromPoint(e.clientX, e.clientY)) &&
      !document
        .elementFromPoint(e.clientX, e.clientY)
        ?.closest(".node,.minimap-wrap,.canvas-bottom")
    )
      picker(worldPoint(e), pending);
  } else if (drag.type === "nodes" && drag.changed) {
    for (const p of drag.positions) {
      const n = graph().nodes.find((n) => n.id === p.id);
      n.x = Math.round(n.x / 10) * 10;
      n.y = Math.round(n.y / 10) * 10;
    }
    render();
    persist();
  } else if (drag.type === "pan") scheduleSave();
  drag = null;
  viewport.classList.remove("panning");
});
viewport.addEventListener(
  "wheel",
  (e) => {
    if (e.target.closest(".node-body") && e.shiftKey) return;
    e.preventDefault();
    const r = viewport.getBoundingClientRect();
    zoomAt(Math.exp(-e.deltaY * 0.0012), e.clientX - r.left, e.clientY - r.top);
  },
  { passive: false },
);
viewport.addEventListener("contextmenu", (e) => {
  if (e.target.closest(".minimap-wrap")) return;
  e.preventDefault();
  picker(worldPoint(e), pending);
});
viewport.addEventListener("dblclick", (e) => {
  if (!e.target.closest(".node,.minimap-wrap,.canvas-bottom,.empty-state"))
    picker(worldPoint(e), pending);
});
viewport.addEventListener("dragover", (e) => {
  e.preventDefault();
  e.dataTransfer.dropEffect = "copy";
});
viewport.addEventListener("drop", (e) => {
  e.preventDefault();
  const id = e.dataTransfer.getData("application/x-kards-node");
  if (id) addNode(id, worldPoint(e));
});
$("minimap").onclick = (e) => {
  const pt = $("minimap").createSVGPoint();
  pt.x = e.clientX;
  pt.y = e.clientY;
  const p = pt.matrixTransform($("minimap").getScreenCTM().inverse()),
    r = viewport.getBoundingClientRect(),
    v = graph().view;
  v.x = r.width / 2 - p.x * v.zoom;
  v.y = r.height / 2 - p.y * v.zoom;
  updateView();
  drawMinimap();
  scheduleSave();
};
function download(filename, text, type) {
  const url = URL.createObjectURL(new Blob([text], { type })),
    a = document.createElement("a");
  a.href = url;
  a.download = filename;
  document.body.append(a);
  a.click();
  a.remove();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}
function exportProject() {
  persist();
  const data = buildExport(project, {
    classes: classMap(),
    enums,
    repository,
  });
  download(
    (project.name || "blueprint").replace(/[\\/:*?"<>|]/g, "_") +
      ".kardsbp.json",
    JSON.stringify(data, null, 2),
    "application/json",
  );
  toast("已导出蓝图，可随时导入继续编辑");
}
// SVG exports use English for everything the editor provides (node, pin and category names),
// and exactly what the user typed for everything they can customise (card title, graph name,
// node titles, comments, default values).
const pinEnglish = (p) =>
  p.name === "执行" ? "Exec" : p.name === "然后" ? "Then" : p.name;
const kindEnglish = (n) =>
  `${(kindLabels[nodeKind(n.definition)] || "FUNCTION").split(" · ")[0]} · ${(n.definition.class || "UE").replace(/^[UA](?=[A-Z][a-z])/, "")}`;
function exportSVG() {
  if (!graph().nodes.length) return toast("请先添加节点");
  const b = bounds();
  const header = 60;
  let svg = `<svg xmlns="http://www.w3.org/2000/svg" viewBox="${b.x} ${b.y - header} ${b.w} ${b.h + header}" width="${b.w}" height="${b.h + header}"><rect x="${b.x}" y="${b.y - header}" width="${b.w}" height="${b.h + header}" fill="#252525"/><text x="${b.x + 20}" y="${b.y - 25}" fill="#d4d4d4" font-family="sans-serif" font-size="16">${esc(project.card.title)} · ${esc(graph().name)}</text><g fill="none" stroke-width="2.3">`;
  for (const e of graph().edges) {
    const a = pinPosition(e.from),
      p = pinPosition(e.to),
      node = graph().nodes.find((n) => n.id === e.from.node),
      pin = node.definition.pins.find((p) => p.id === e.from.pin);
    if (a && p)
      svg += `<path d="${curve(a, p)}" stroke="${pinColor(pin.type)}"/>`;
  }
  svg += "</g>";
  for (const n of graph().nodes) {
    const el = document.querySelector(`[data-node-id="${CSS.escape(n.id)}"]`),
      h = el.offsetHeight,
      w = el.offsetWidth;
    svg += `<g font-family="monospace"><rect x="${n.x}" y="${n.y}" width="${w}" height="${h}" rx="6" fill="#171717" stroke="${color(n.definition)}"/><rect x="${n.x + 1}" y="${n.y + 1}" width="${w - 2}" height="57" rx="5" fill="${color(n.definition)}"/><text x="${n.x + 12}" y="${n.y + 24}" fill="#e0e9e2" font-size="12">${esc(n.title || nodeEnglish(n.definition))}</text><text x="${n.x + 12}" y="${n.y + 43}" fill="#eeeeee" font-size="9">${esc(n.title ? nodeEnglish(n.definition) : kindEnglish(n))}</text>`;
    if (n.comment)
      svg += `<text x="${n.x}" y="${n.y - 12}" fill="#a4b69d" font-size="10">${esc(n.comment)}</text>`;
    for (const p of n.definition.pins) {
      const pos = pinPosition({ node: n.id, pin: p.id }),
        out = p.direction === "out";
      if (!pos) continue;
      const val = n.values[p.id] ?? defaultValue(p.type, enums);
      const hasValue =
        p.type !== "exec" &&
        !graph().edges.some((e) => e.to.node === n.id && e.to.pin === p.id) &&
        (p.direction === "in" ||
          (n.definition.category === "variables" && n.definition.kind === "variable"));
      svg += `<circle cx="${pos.x}" cy="${pos.y}" r="4" fill="#171717" stroke="${pinColor(p.type)}" stroke-width="2"/><text x="${pos.x + (out ? -12 : 12)}" y="${pos.y + 3}" fill="#b1c4b8" font-size="9" text-anchor="${out ? "end" : "start"}">${esc(pinEnglish(p))}${hasValue ? " = " + esc(val) : ""}</text>`;
    }
    svg += "</g>";
  }
  svg += "</svg>";
  download(
    (graph().name || "graph").replace(/[\\/:*?"<>|]/g, "_") + ".svg",
    svg,
    "image/svg+xml",
  );
  toast("已导出可独立打开的 SVG 图像");
}
function newProject() {
  openModal(
    "新建蓝图项目",
    `<div class="field"><label class="detail-label" for="new-name">资源名称</label><input id="new-name" value="BP_NewCard" maxlength="64"><p class="detail-description">名称只能包含英文字母、数字和下划线，不能以数字开头（不支持中文）。</p></div><p>当前项目会保留在撤销记录中。建议先导出文件，方便以后继续编辑。</p><div class="modal-actions"><button id="save-first" class="secondary-button">导出当前项目</button><button id="create-project" class="primary-button">创建项目</button></div>`,
  );
  $("save-first").onclick = exportProject;
  $("create-project").onclick = () => {
    const name = $("new-name").value.trim(),
      error = checkName(name);
    if (error) return toast(error, true);
    mutate(() => {
      project = makeProject();
      project.name = name;
      selection.clear();
      selectedEdge = null;
    });
    $("modal").close();
  };
}
$("file-input").onchange = async (e) => {
  const file = e.target.files[0];
  if (!file) return;
  try {
    if (file.size > 12 * 1024 * 1024) throw new Error("文件超过 12 MB 限制");
    const parsed = JSON.parse(await file.text());
    setTypeHierarchy({ ...classes, ...parsed.types });
    const imported = validateProject(parsed);
    mutate(() => {
      project = imported;
      selection.clear();
      selectedEdge = null;
    });
    toast(`已导入 ${project.name} · ${project.graphs.length} 个图表`);
  } catch (error) {
    setTypeHierarchy({ ...classes, ...project.types });
    toast(
      error instanceof SyntaxError ? "JSON 格式错误，无法导入" : error.message,
      true,
    );
  } finally {
    e.target.value = "";
  }
};
function validate() {
  try {
    validateProject(clone(project));
    const g = graph(),
      unconnected = g.nodes.filter((n) =>
        n.definition.pins.some(
          (p) =>
            p.type === "exec" &&
            p.direction === "in" &&
            !g.edges.some((e) => e.to.node === n.id && e.to.pin === p.id),
        ),
      );
    openModal(
      "蓝图检查",
      `<div class="validation-item">✓ 文件结构、端口类型和连接引用有效。</div><div class="validation-item">${g.nodes.length} 个节点 · ${g.edges.length} 条连接 · ${unconnected.length} 个动作节点没有执行输入</div>${unconnected.length ? `<p>尚未接入执行流：${unconnected.map((n) => esc(nodeHeading(n))).join(langMode === "en" ? ", " : "、")}。可以保留它们作为设计草稿。</p>` : ""}<p>检查范围为蓝图文档结构。此编辑器不运行蓝图，也不验证游戏运行结果。</p>`,
    );
  } catch (e) {
    toast(e.message, true);
  }
}
function help() {
  openModal(
    "设计你的下一张卡牌",
    `<div class="help-grid"><span>添加节点</span><kbd>Tab / 右键</kbd><span>平移画布</span><kbd>拖动空白 / 空格 + 拖动</kbd><span>缩放画布</span><kbd>滚轮 / ＋ −</kbd><span>适应所有节点</span><kbd>F</kbd><span>多选节点</span><kbd>Shift + 单击</kbd><span>全选 / 复制 / 删除</span><kbd>Ctrl A / Ctrl D / Delete</kbd><span>撤销 / 重做</span><kbd>Ctrl Z / Ctrl Shift Z</kbd><span>保存到本地 / 导出文件</span><kbd>Ctrl S / Ctrl Shift S</kbd><span>取消连线 / 取消选择</span><kbd>Esc</kbd></div><p>从一个端口拖向另一个端口，或依次点击两个端口。只有类型相同的输出与输入可以连接。点击连线后按 Delete 可断开。</p><p>修改会自动保存在当前浏览器。使用「导出蓝图」获得文件；更换设备或清除浏览器数据前请先导出。支持 macOS 的 ⌘ 快捷键。</p><div class="format-note">format: "${FORMAT}"\nversion: ${VERSION}\ngraphs: [{ nodes, edges, view }]</div>`,
  );
}
$("node-search").oninput = renderLibrary;
$("add-node").onclick = () => picker();
$("empty-add").onclick = () => picker();
$("add-graph").onclick = newItemMenu;
$("new-project").onclick = newProject;
$("import").onclick = () => $("file-input").click();
$("save").onclick = () => {
  persist();
  toast("项目已保存到当前浏览器");
};
$("export").onclick = exportProject;
$("export-svg").onclick = exportSVG;
$("undo").onclick = undo;
$("redo").onclick = redo;
$("fit").onclick = fit;
$("zoom-in").onclick = () => zoomAt(1.15);
$("zoom-out").onclick = () => zoomAt(1 / 1.15);
$("zoom-label").onclick = () => zoomAt(1 / graph().view.zoom);
// Re-download every site file from the server (bypassing the HTTP cache), then reload.
async function forceRefresh() {
  const button = $("force-refresh");
  button.disabled = true;
  button.textContent = "⟳ …";
  try {
    persist();
    const files = [
      "./",
      "./style.css",
      "./polish.css",
      "./app.js",
      "./model.js",
      "./ue-nodes.js",
      "./i18n.js",
      "./catalog.json",
    ];
    await Promise.all(
      files.map((f) =>
        fetch(f, { cache: "reload" }).then((r) => {
          if (!r.ok) throw new Error(`${f} ${r.status}`);
        }),
      ),
    );
    if ("caches" in window)
      await Promise.all((await caches.keys()).map((k) => caches.delete(k)));
    toast("已从 GitHub 读取最新版本，正在刷新…");
    setTimeout(() => location.reload(), 400);
  } catch (error) {
    button.disabled = false;
    button.innerHTML = `⟳ ${bi("刷新", "Refresh")}`;
    toast(`刷新失败：${error.message}。请检查网络后重试。`, true);
  }
}
$("force-refresh").onclick = forceRefresh;
$("help").onclick = help;
$("validate").onclick = validate;
$("toggle-inspector").onclick = () => {
  const panel = $("inspector");
  if (window.innerWidth <= 1000)
    setInspectorOpen(!panel.classList.contains("mobile-open"));
  else panel.hidden = !panel.hidden;
  requestAnimationFrame(() => {
    drawEdges();
    drawMinimap();
  });
};
function setDrawer(open) {
  $("left-panel").classList.toggle("open", open);
  $("drawer-backdrop").classList.toggle("open", open);
  if (open) $("inspector").classList.remove("mobile-open");
}
function setInspectorOpen(open) {
  $("inspector").classList.toggle("mobile-open", open);
  $("drawer-backdrop").classList.toggle("open", open);
}
$("bar-details").onclick = () => setInspectorOpen(true);
$("bar-duplicate").onclick = duplicate;
$("bar-delete").onclick = deleteSelection;
$("toggle-library").onclick = () =>
  setDrawer(!$("left-panel").classList.contains("open"));
$("drawer-backdrop").onclick = () => {
  setDrawer(false);
  setInspectorOpen(false);
};
$("graph-list").addEventListener("click", () => {
  if (window.innerWidth <= 800) setDrawer(false);
});
$("project-name").onchange = (e) => {
  const name = e.target.value.trim(),
    error = checkName(name);
  if (error) {
    toast(error, true);
    e.target.value = project.name;
  } else mutate(() => (project.name = name));
};
$("modal").onclick = (e) => {
  if (Date.now() < ignoreBackdropUntil) return;
  if (e.target === $("modal")) {
    const r = $("modal").getBoundingClientRect();
    if (
      e.clientX < r.x ||
      e.clientX > r.right ||
      e.clientY < r.y ||
      e.clientY > r.bottom
    )
      $("modal").close();
  }
};
window.addEventListener("keydown", (e) => {
  const typing =
    /INPUT|TEXTAREA|SELECT/.test(e.target.tagName) ||
    e.target.isContentEditable;
  if (e.key === "Escape") {
    pending = null;
    selection.clear();
    selectedEdge = null;
    if (!$("modal").open) {
      renderNodes();
      renderInspector();
      drawEdges();
      updateSelectionStatus();
    }
    return;
  }
  if ($("modal").open) return;
  const mod = e.ctrlKey || e.metaKey;
  if (mod && e.key.toLowerCase() === "s") {
    e.preventDefault();
    if (e.shiftKey) exportProject();
    else {
      persist();
      toast("已保存到当前浏览器");
    }
    return;
  }
  if (typing) return;
  if (e.key === " ") {
    e.preventDefault();
    space = true;
  }
  if (e.key === "Tab") {
    e.preventDefault();
    picker();
  }
  if (e.key === "/") {
    e.preventDefault();
    $("node-search").focus();
  }
  if (e.key.toLowerCase() === "f" && !mod) fit();
  if (e.key === "Delete" || e.key === "Backspace") {
    e.preventDefault();
    deleteSelection();
  }
  if (mod && e.key.toLowerCase() === "z") {
    e.preventDefault();
    e.shiftKey ? redo() : undo();
  }
  if (mod && e.key.toLowerCase() === "y") {
    e.preventDefault();
    redo();
  }
  if (mod && e.key.toLowerCase() === "d") {
    e.preventDefault();
    duplicate();
  }
  if (mod && e.key.toLowerCase() === "a") {
    e.preventDefault();
    selection = new Set(graph().nodes.map((n) => n.id));
    renderNodes();
    renderInspector();
    drawEdges();
    updateSelectionStatus();
  }
});
window.addEventListener("keyup", (e) => {
  if (e.key === " ") space = false;
});
window.addEventListener("blur", () => {
  space = false;
});
window.addEventListener("resize", () => {
  drawEdges();
  drawMinimap();
});
window.addEventListener("beforeunload", () => {
  if (project) persist();
});
async function boot() {
  try {
    const response = await fetch("./catalog.json");
    if (!response.ok) throw new Error("节点库载入失败");
    const data = await response.json();
    classes = { ...engineClasses, ...data.classes };
    setTypeHierarchy(classes);
    catalog = [
      ...builtins,
      ...typeNodes(classes, data.properties || []),
      ...data.nodes,
    ];
    enums = data.enums;
    repository = data.repository || null;
    $("catalog-count").textContent = catalog.length;
    $("source-status").textContent =
      `${data.nodes.length} 函数 · ${Object.keys(data.classes || {}).length} 类 · ${builtins.length} UE 节点`;
    let restored = false;
    try {
      const saved = localStorage.getItem(STORAGE);
      if (saved) {
        const parsed = JSON.parse(saved);
        setTypeHierarchy({ ...classes, ...parsed.types });
        project = validateProject(parsed);
        restored = true;
      }
    } catch {
      toast("本地草稿无法读取，已新建空白项目。可导入备份文件恢复。", true);
    }
    project ??= makeProject();
    project.types = { ...classes, ...project.types };
    renderLibrary();
    render();
    if (!restored) requestAnimationFrame(fit);
    persist();
  } catch (e) {
    $("source-status").textContent = "节点库载入失败";
    openModal(
      "无法加载节点库",
      `<p>${esc(e.message)}。请刷新页面，或检查 catalog.json 是否和站点一起发布。</p><button class="primary-button" onclick="location.reload()">重新加载</button>`,
    );
  }
}
$("modal").addEventListener("close", () => {
  pickerContext = null;
  pending = null;
  if (project) {
    renderNodes();
    drawEdges();
    updateSelectionStatus();
  }
});
$("new-project").innerHTML = bi("新建", "New");
$("import").innerHTML = bi("导入", "Import");
$("export").innerHTML = `<span>↗</span>${bi("导出", "Export")}`;
$("add-node").innerHTML = `＋ ${bi("节点", "Node")}`;
$("validate").innerHTML = bi("检查", "Check");
$("lang-select").value = langMode;
setUiLanguage(langMode);
$("lang-select").onchange = (e) => {
  langMode = e.target.value;
  document.documentElement.dataset.lang = langMode;
  try {
    localStorage.setItem(LANG_KEY, langMode);
  } catch {
    /* storage unavailable: the choice lasts for this visit only */
  }
  renderLibrary();
  render();
  setUiLanguage(langMode);
};
boot();
