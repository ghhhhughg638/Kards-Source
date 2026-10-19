import {
  FORMAT,
  VERSION,
  uid,
  clone,
  builtins,
  makeNode,
  makeGraph,
  makeProject,
  pinKind,
  defaultValue,
  connect,
  validateProject,
  createDemo,
  setTypeHierarchy,
  matchingPin,
} from "./model.js";
import { typeNodes, engineClasses } from "./ue-nodes.js";
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
const STORAGE = "kards-blueprint-studio.v1",
  CAT = {
    all: "全部 All",
    events: "事件 Events",
    combat: "战斗",
    cards: "卡牌",
    effects: "效果",
    queries: "查询",
    match: "对局",
    flow: "流程 Flow",
    variables: "变量 Variables",
    utilities: "工具 Utilities",
    properties: "属性 Properties",
    math: "运算 Math",
    arrays: "数组 Array",
    conversion: "转换 Cast",
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
};
const PINCOLORS = {
  exec: "#eeeeee",
  object: "#00a9ef",
  int: "#30e7bd",
  bool: "#d41119",
  string: "#ec3fc4",
  enum: "#03c5b1",
  float: "#9fdb47",
  array: "#e69030",
  name: "#bf83ef",
  text: "#ec96b4",
  vector: "#e3bd20",
  rotator: "#aaa0eb",
};
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
    })[nodeKind(d)] || "ƒ",
  title = (n) =>
    n.title
      ? (nodeKind(n.definition) === "event" && !n.title.startsWith("事件")
          ? "事件 · "
          : "") + n.title
      : nodeChinese(n.definition);
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
  $("graph-list").innerHTML = project.graphs
    .map(
      (g) =>
        `<button class="graph-link ${g.id === project.activeGraph ? "active" : ""}" data-graph="${esc(g.id)}"><span class="graph-icon">⌘</span>${esc(g.name)}<span>${g.nodes.length}</span></button>`,
    )
    .join("");
  $("tabs").innerHTML =
    project.graphs
      .map(
        (g) =>
          `<button class="tab ${g.id === project.activeGraph ? "active" : ""}" data-graph="${esc(g.id)}"><span class="graph-icon">⌘</span>${esc(g.name)}<span class="tab-dot"></span></button>`,
      )
      .join("") +
    '<button class="tab add-tab" id="tab-add" title="新建事件图">＋</button>';
  document
    .querySelectorAll("[data-graph]")
    .forEach((b) => (b.onclick = () => switchGraph(b.dataset.graph)));
  $("tab-add").onclick = addGraph;
  $("tabs")
    .querySelectorAll("[data-graph]")
    .forEach((b) => (b.ondblclick = () => editGraph(b.dataset.graph)));
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
function addGraph() {
  openModal(
    "新建事件图",
    '<div class="field"><label class="detail-label" for="graph-name">事件图名称</label><input id="graph-name" maxlength="80" value="新事件图"></div><div class="modal-actions"><button id="create-graph" class="primary-button">创建事件图</button></div>',
  );
  $("create-graph").onclick = () => {
    const name = $("graph-name").value.trim();
    if (!name) return;
    mutate(() => {
      const g = makeGraph(name);
      project.graphs.push(g);
      project.activeGraph = g.id;
      selection.clear();
    });
    $("modal").close();
  };
  $("graph-name").select();
}
function editGraph(id) {
  const g = project.graphs.find((x) => x.id === id);
  openModal(
    "事件图设置",
    `<div class="field"><label class="detail-label" for="graph-name">名称</label><input id="graph-name" maxlength="80" value="${esc(g.name)}"></div><div class="modal-actions">${project.graphs.length > 1 ? '<button id="delete-graph" class="danger-button" style="width:auto;margin:0">删除事件图</button>' : ""}<button id="rename-graph" class="primary-button">保存</button></div>`,
  );
  $("rename-graph").onclick = () => {
    const name = $("graph-name").value.trim();
    if (name) {
      mutate(() => (g.name = name));
      $("modal").close();
    }
  };
  if ($("delete-graph"))
    $("delete-graph").onclick = () => {
      if (!confirm(`删除“${g.name}”及其所有节点？可使用撤销恢复。`)) return;
      mutate(() => {
        project.graphs = project.graphs.filter((x) => x !== g);
        if (project.activeGraph === id)
          project.activeGraph = project.graphs[0].id;
        selection.clear();
      });
      $("modal").close();
    };
}
function libraryItem(d) {
  return `<button class="library-node" draggable="true" data-def="${esc(d.id)}" title="${esc(d.signature || nodeEnglish(d))}" style="--node-color:${color(d)}"><span class="lib-icon">${icon(d)}</span><span><strong>${esc(nodeEnglish(d))}</strong><small>${esc(nodeChinese(d))} <span class="lib-owner">${esc((d.class || "UE").replace(/^[UA](?=[A-Z][a-z])/, "").replace("Stub", ""))}</span></small></span></button>`;
}
function filterCatalog(q, cat = "all") {
  q = q.trim().toLowerCase();
  const items = catalog.filter(
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
  $("library-label").textContent = `${CAT[category]} · ${results.length}`;
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
function centerPoint() {
  const r = $("viewport").getBoundingClientRect(),
    v = graph().view;
  return {
    x: (r.width / 2 - v.x) / v.zoom - 140,
    y: (r.height / 2 - v.y) / v.zoom - 70,
  };
}
function addNode(id, point, sourceEndpoint = null) {
  const def = catalog.find((d) => d.id === id);
  if (!def) return;
  point = point || centerPoint();
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
    `${sourcePin ? `<div class="context-type"><i style="background:${PINCOLORS[pinKind(sourcePin.type)]}"></i><strong>${esc(sourcePin.type)}</strong><span>${esc(translate(sourcePin.name))} · ${esc(sourcePin.name)}</span></div>` : ""}<input id="picker-search" class="modal-search" placeholder="搜索 / Search actions…" aria-label="搜索可添加节点"><div class="context-options"><label><input type="checkbox" id="context-sensitive" checked> 上下文关联 / Context Sensitive</label>${sourcePin ? '<select id="context-scope"><option value="all">所有兼容操作 / All Compatible</option><option value="members">属性和成员 / Members</option><option value="functions">成员函数 / Functions</option><option value="properties">读取/写入属性 / Properties</option><option value="casts">类型转换 / Cast & Convert</option></select>' : ""}</div><div id="picker-count"></div><div id="picker-results" class="picker-results"></div><div class="modal-actions"><button id="custom-node" class="secondary-button">＋ 自定义节点 / Custom Node</button></div>`,
    sourcePin
      ? "选择节点后自动连接此引脚 / Automatically connect compatible pins"
      : `${catalog.length} 个节点 · KARDS + Unreal Blueprint`,
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
          addNode(b.dataset.def, placement, pickerContext);
          $("modal").close();
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
  const button = `<button class="pin ${p.type === "exec" ? "exec" : ""} ${connected ? "connected" : ""} ${pending?.node === n.id && pending.pin === p.id ? "pending" : ""}" style="--pin-color:${PINCOLORS[pinKind(p.type)]}" data-node="${esc(n.id)}" data-pin="${esc(p.id)}" title="${esc(p.name)} · ${esc(p.type)} · ${p.direction === "in" ? "输入" : "输出"}" aria-label="${esc(p.name)} ${p.direction === "in" ? "输入" : "输出"}端口"></button>`;
  const cn = translate(p.name),
    raw = p.name === "执行" ? "Exec" : p.name === "然后" ? "Then" : p.name;
  const label = `<span class="pin-label"><span class="pin-cn">${esc(cn)}</span><span class="pin-en">${esc(raw)}</span></span>`;
  const showValue =
    !connected &&
    p.type !== "exec" &&
    (p.direction === "in" || n.definition.category === "variables");
  return `<div class="pin-row ${p.direction === "out" ? "output" : ""}">${p.direction === "in" ? button : ""}${label}${showValue ? `<span class="pin-value">${esc(typeof val === "boolean" ? (val ? "true" : "false") : val)}</span>` : ""}${p.direction === "out" ? button : ""}</div>`;
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
function renderNodes() {
  $("nodes").innerHTML = graph()
    .nodes.map(
      (n) =>
        `<article class="node ue-${nodeKind(n.definition)} ${selection.has(n.id) ? "selected" : ""}" data-node-id="${esc(n.id)}" style="left:${n.x}px;top:${n.y}px;--node-color:${color(n.definition)}">${n.comment ? `<div class="node-comment">${esc(n.comment)}</div>` : ""}<div class="node-head"><div class="node-heading"><span>${icon(n.definition)}</span><strong>${esc(title(n))}</strong></div><div class="node-subtitle">${esc(nodeEnglish(n.definition))}</div></div><div class="node-kind-label">${esc(kindLabels[nodeKind(n.definition)] || "FUNCTION")}<span>${esc((n.definition.class || "UE").replace(/^[UA](?=[A-Z][a-z])/, ""))}</span></div><div class="node-body">${nodeBody(n)}</div></article>`,
    )
    .join("");
}
function updateSelectionStatus() {
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
        (p.direction === "in" || d.category === "variables"),
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
            return `<div class="field"><label class="detail-label">${esc(translate(p.name))} / ${esc(p.name)} <span style="opacity:.5">${esc(p.type)}</span>${options ? `<select data-value="${esc(p.id)}" ${isConnected ? "disabled" : ""}>${[...new Set([...options, String(val)])].map((v) => `<option ${String(v) === String(val) ? "selected" : ""}>${esc(v)}</option>`).join("")}</select>` : `<input data-value="${esc(p.id)}" value="${esc(val)}" ${["int", "float"].includes(pinKind(p.type)) ? 'type="number" step="any"' : ""} ${isConnected ? "disabled" : ""}>`}${isConnected ? '<span class="detail-description">由连接提供值</span>' : ""}</label></div>`;
          })
          .join("")
      : '<p class="detail-description">此节点没有可编辑的默认输入。</p>';
    body += `</section><section class="detail-section"><h3 class="detail-heading">源代码 <span>NATIVE</span></h3><div class="detail-source">${esc(d.signature || d.name)}${d.source ? `<p><a href="https://github.com/ghhhhughg638/Kards-Source/blob/main/${encodeURI(d.source)}" target="_blank" rel="noreferrer">${esc(d.source)} ↗</a></p>` : "<p>Studio 自定义节点 · 仅用于可视化设计</p>"}</div><button class="secondary-button" id="duplicate" style="width:100%;margin-top:10px">复制节点 <kbd>Ctrl D</kbd></button><button class="danger-button" id="delete-selection">删除选中节点</button></section>`;
  } else {
    const card = project.card;
    body = `<section class="detail-section"><h3 class="detail-heading">卡牌信息 <span>CARD DATA</span></h3><div class="card-preview"><div class="card-emblem">${esc(card.cost)}</div><div><small>${esc(card.faction.toUpperCase())} / ${esc(card.type.toUpperCase())}</small><h3>${esc(card.title)}</h3><p>${esc(FACTIONS[card.faction] || card.faction)} · ${esc(card.rarity)}</p></div></div>${field("卡牌名称", "card.title", card.title)}${field(
      "所属阵营",
      "card.faction",
      card.faction,
      (enums.EFactionEnum || Object.keys(FACTIONS)).map((value) => ({
        value,
        label: FACTIONS[value] || value,
      })),
    )}${field("卡牌类型", "card.type", card.type, enums.ETypeEnum || ["infantry", "tank", "order"])}${field("稀有度", "card.rarity", card.rarity, enums.ERarityEnum || ["Common", "Uncommon", "Rare", "Unique"])}<div class="field-grid">${field("攻击力", "card.attack", card.attack)}${field("防御力", "card.defense", card.defense)}</div><div style="height:12px"></div>${field("指挥点消耗", "card.cost", card.cost)}<div class="field"><label class="detail-label">卡牌效果<textarea data-field="card.text">${esc(card.text)}</textarea></label></div></section><section class="detail-section"><h3 class="detail-heading">事件图 <span>GRAPH</span></h3>${field("名称", "graph.name", graph().name)}<p class="detail-description">点击节点可编辑参数和注释。双击顶部标签可重命名或删除事件图。</p><button class="secondary-button" id="graph-settings" style="width:100%">管理当前事件图</button></section><section class="detail-section"><h3 class="detail-heading">文件格式 <span>v1.0</span></h3><p class="detail-description">.kardsbp.json 包含卡牌信息、多个事件图、节点定义、参数和连线。导入后可继续编辑。</p></section>`;
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
          mutate(() => {
            if (key.startsWith("card.")) project.card[key.slice(5)] = val;
            else if (key === "graph.name") graph().name = val || "事件图";
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
    html += `<path class="edge ${selectedEdge === e.id ? "selected" : ""}" data-edge="${esc(e.id)}" d="${curve(a, b)}" stroke="${PINCOLORS[pinKind(pin.type)]}"/>`;
  }
  if (pending && pointer) {
    const p = pinPosition(pending),
      def = graph()
        .nodes.find((n) => n.id === pending.node)
        ?.definition.pins.find((p) => p.id === pending.pin);
    if (p)
      html += `<path class="edge ghost" d="${def.direction === "out" ? curve(p, pointer) : curve(pointer, p)}" stroke="${PINCOLORS[pinKind(def.type)]}"/>`;
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
function bounds() {
  if (!graph().nodes.length) return { x: 0, y: 0, w: 900, h: 600 };
  const ns = graph().nodes;
  const x = Math.min(...ns.map((n) => n.x)) - 30,
    y = Math.min(...ns.map((n) => n.y)) - 55;
  return {
    x,
    y,
    w: Math.max(...ns.map((n) => n.x + 280)) - x + 30,
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
  const b = bounds(),
    r = $("viewport").getBoundingClientRect(),
    z = Math.max(
      0.2,
      Math.min(1.15, (r.width - 70) / b.w, (r.height - 145) / b.h),
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
          `<rect x="${n.x}" y="${n.y}" width="280" height="${document.querySelector(`[data-node-id="${CSS.escape(n.id)}"]`)?.offsetHeight || 150}" rx="5" fill="${color(n.definition)}" fill-opacity=".3" stroke="${color(n.definition)}" stroke-width="2"/>`,
      )
      .join("") +
    `<rect x="${-v.x / v.zoom}" y="${-v.y / v.zoom}" width="${r.width / v.zoom}" height="${r.height / v.zoom}" fill="#dddddd" fill-opacity=".03" stroke="#c7c7c7" stroke-width="3"/>`;
}
function deleteSelection() {
  if (!selection.size && !selectedEdge) return;
  mutate(() => {
    graph().nodes = graph().nodes.filter((n) => !selection.has(n.id));
    graph().edges = graph().edges.filter(
      (e) =>
        !selection.has(e.from.node) &&
        !selection.has(e.to.node) &&
        e.id !== selectedEdge,
    );
    selection.clear();
    selectedEdge = null;
  });
}
function duplicate() {
  if (!selection.size) return;
  mutate(() => {
    const copied = graph().nodes.filter((n) => selection.has(n.id)),
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
viewport.addEventListener("pointerdown", (e) => {
  if (e.button === 2) return;
  if (e.target.closest(".canvas-bottom,.minimap-wrap,.empty-state")) return;
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
  download(
    (project.name || "blueprint").replace(/[\\/:*?"<>|]/g, "_") +
      ".kardsbp.json",
    JSON.stringify(
      { ...project, exportedAt: new Date().toISOString() },
      null,
      2,
    ),
    "application/json",
  );
  toast("已导出蓝图，可随时导入继续编辑");
}
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
      svg += `<path d="${curve(a, p)}" stroke="${PINCOLORS[pinKind(pin.type)]}"/>`;
  }
  svg += "</g>";
  for (const n of graph().nodes) {
    const el = document.querySelector(`[data-node-id="${CSS.escape(n.id)}"]`),
      h = el.offsetHeight;
    svg += `<g font-family="monospace"><rect x="${n.x}" y="${n.y}" width="280" height="${h}" rx="6" fill="#171717" stroke="${color(n.definition)}"/><rect x="${n.x + 1}" y="${n.y + 1}" width="278" height="57" rx="5" fill="${color(n.definition)}"/><text x="${n.x + 12}" y="${n.y + 24}" fill="#e0e9e2" font-size="12">${esc(title(n))}</text><text x="${n.x + 12}" y="${n.y + 43}" fill="#eeeeee" font-size="9">${esc(nodeEnglish(n.definition))}</text>`;
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
        (p.direction === "in" || n.definition.category === "variables");
      svg += `<circle cx="${pos.x}" cy="${pos.y}" r="4" fill="#171717" stroke="${PINCOLORS[pinKind(p.type)]}" stroke-width="2"/><text x="${pos.x + (out ? -12 : 12)}" y="${pos.y + 3}" fill="#b1c4b8" font-size="9" text-anchor="${out ? "end" : "start"}">${esc(translate(p.name))} / ${esc(p.name)}${hasValue ? " = " + esc(val) : ""}</text>`;
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
    `<div class="field"><label class="detail-label" for="new-name">项目名称</label><input id="new-name" value="我的 KARDS 蓝图" maxlength="100"></div><div class="field"><label class="detail-label" for="template">起始模板</label><select id="template"><option value="empty">空白卡牌蓝图</option><option value="demo">示例：部署与战术分支</option></select></div><p>当前项目会保留在撤销记录中。建议先导出文件，方便以后继续编辑。</p><div class="modal-actions"><button id="save-first" class="secondary-button">导出当前项目</button><button id="create-project" class="primary-button">创建项目</button></div>`,
  );
  $("save-first").onclick = exportProject;
  $("create-project").onclick = () => {
    const name = $("new-name").value.trim() || "未命名卡牌",
      demo = $("template").value === "demo";
    mutate(() => {
      project = demo ? createDemo(catalog) : makeProject();
      project.name = name;
      if (!demo) project.card.title = name;
      selection.clear();
      selectedEdge = null;
    });
    $("modal").close();
    if (demo) requestAnimationFrame(fit);
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
    toast(`已导入 ${project.name} · ${project.graphs.length} 个事件图`);
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
      `<div class="validation-item">✓ 文件结构、端口类型和连接引用有效。</div><div class="validation-item">${g.nodes.length} 个节点 · ${g.edges.length} 条连接 · ${unconnected.length} 个动作节点没有执行输入</div>${unconnected.length ? `<p>尚未接入执行流：${unconnected.map((n) => esc(title(n))).join("、")}。可以保留它们作为设计草稿。</p>` : ""}<p>检查范围为蓝图文档结构。此编辑器不运行蓝图，也不验证游戏运行结果。</p>`,
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
$("add-graph").onclick = addGraph;
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
$("help").onclick = help;
$("validate").onclick = validate;
$("toggle-inspector").onclick = () => {
  const panel = $("inspector");
  if (window.innerWidth <= 1000) panel.classList.toggle("mobile-open");
  else panel.hidden = !panel.hidden;
  requestAnimationFrame(() => {
    drawEdges();
    drawMinimap();
  });
};
function setDrawer(open) {
  $("left-panel").classList.toggle("open", open);
  $("drawer-backdrop").classList.toggle("open", open);
}
$("toggle-library").onclick = () =>
  setDrawer(!$("left-panel").classList.contains("open"));
$("drawer-backdrop").onclick = () => setDrawer(false);
$("graph-list").addEventListener("click", () => {
  if (window.innerWidth <= 800) setDrawer(false);
});
$("project-name").onchange = (e) => {
  const name = e.target.value.trim().slice(0, 200);
  if (name) mutate(() => (project.name = name));
  else e.target.value = project.name;
};
$("modal").onclick = (e) => {
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
      toast("本地草稿无法读取，已打开示例。可导入备份文件恢复。", true);
    }
    project ??= createDemo(catalog);
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
$("new-project").textContent = "新建 New";
$("import").textContent = "导入 Import";
$("export").innerHTML = "<span>↗</span>导出 Export";
$("add-node").textContent = "＋ 节点 Node";
$("validate").textContent = "检查 Check";
boot();
