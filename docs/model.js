export const FORMAT = "kards-blueprint";
export const VERSION = 1;
export const uid = () => crypto.randomUUID();
export const clone = (value) =>
  typeof structuredClone === "function"
    ? structuredClone(value)
    : JSON.parse(JSON.stringify(value));
import { ueBuiltins, engineClasses } from "./ue-nodes.js";
export const builtins = ueBuiltins;
let typeHierarchy = { ...engineClasses };
export function setTypeHierarchy(classes = {}) {
  typeHierarchy = { ...engineClasses, ...classes };
}
export function compatible(a, b) {
  if (a === b) return true;
  if (
    !a.endsWith("*") ||
    !b.endsWith("*") ||
    a.includes("<") ||
    b.includes("<")
  )
    return false;
  let current = a.slice(0, -1),
    wanted = b.slice(0, -1);
  const seen = new Set();
  while (current && !seen.has(current)) {
    if (current === wanted) return true;
    seen.add(current);
    current = typeHierarchy[current]?.parent;
  }
  return false;
}
export function matchingPin(definition, source) {
  const options = definition.pins.filter(
    (p) =>
      p.direction !== source.direction &&
      (source.direction === "out"
        ? compatible(source.type, p.type)
        : compatible(p.type, source.type)),
  );
  return (
    options.find((p) => p.id === "target") ||
    options.find((p) => p.type === source.type) ||
    options[0]
  );
}
export function makeNode(def, x, y) {
  return {
    id: uid(),
    definition: clone(def),
    x,
    y,
    title: "",
    comment: "",
    values: {},
  };
}
export function makeGraph(name = "事件图") {
  return {
    id: uid(),
    name,
    nodes: [],
    edges: [],
    view: { x: 40, y: 0, zoom: 1 },
  };
}
export function makeProject() {
  const g = makeGraph();
  return {
    format: FORMAT,
    version: VERSION,
    id: uid(),
    name: "未命名卡牌",
    card: {
      title: "未命名卡牌",
      faction: "Soviet",
      type: "infantry",
      rarity: "Common",
      cost: 3,
      attack: 3,
      defense: 4,
      text: "在这里描述卡牌效果。",
    },
    graphs: [g],
    activeGraph: g.id,
  };
}
export function pinKind(type) {
  if (type === "exec") return "exec";
  if (type.includes("TArray") || type.includes("TSet")) return "array";
  if (type.includes("bool")) return "bool";
  if (/^(int|uint)/.test(type)) return "int";
  if (/float|double/.test(type)) return "float";
  if (type === "FName") return "name";
  if (type === "FText") return "text";
  if (type === "FVector" || type === "FVector2D") return "vector";
  if (type === "FRotator") return "rotator";
  if (type === "FString") return "string";
  if (/^E[A-Z]/.test(type) || type.startsWith("Enum")) return "enum";
  return "object";
}
export function defaultValue(type, enums = {}) {
  if (enums[type]) return enums[type][0] || "";
  const k = pinKind(type);
  return k === "bool"
    ? false
    : k === "int" || k === "float"
      ? 0
      : k === "object"
        ? "Self"
        : k === "array"
          ? "[]"
          : "";
}
export function connect(graph, a, b) {
  const na = graph.nodes.find((n) => n.id === a.node),
    nb = graph.nodes.find((n) => n.id === b.node);
  if (!na || !nb || na === nb) throw new Error("请连接两个不同的节点");
  const pa = na.definition.pins.find((p) => p.id === a.pin),
    pb = nb.definition.pins.find((p) => p.id === b.pin);
  if (!pa || !pb || pa.direction === pb.direction)
    throw new Error("请将输出端口连接到输入端口");
  const [from, to, po, pi] =
    pa.direction === "out" ? [a, b, pa, pb] : [b, a, pb, pa];
  if (!compatible(po.type, pi.type))
    throw new Error(`端口类型不匹配：${po.type} → ${pi.type}`);
  if (
    graph.edges.some(
      (e) =>
        e.from.node === from.node &&
        e.from.pin === from.pin &&
        e.to.node === to.node &&
        e.to.pin === to.pin,
    )
  )
    return false;
  graph.edges = graph.edges.filter(
    (e) =>
      !(e.to.node === to.node && e.to.pin === to.pin) &&
      !(
        po.type === "exec" &&
        e.from.node === from.node &&
        e.from.pin === from.pin
      ),
  );
  graph.edges.push({ id: uid(), from: clone(from), to: clone(to) });
  return true;
}
export function validateProject(input) {
  if (!input || input.format !== FORMAT || input.version !== VERSION)
    throw new Error("无法识别文件。请导入版本 1 的 .kardsbp.json 蓝图文件。");
  if (
    typeof input.name !== "string" ||
    input.name.length > 200 ||
    !Array.isArray(input.graphs) ||
    !input.graphs.length ||
    input.graphs.length > 100
  )
    throw new Error("项目名称或事件图列表无效");
  if (
    !input.card ||
    ["title", "faction", "type", "rarity", "text"].some(
      (k) => typeof input.card[k] !== "string",
    ) ||
    ["cost", "attack", "defense"].some(
      (k) =>
        typeof input.card[k] !== "number" || !Number.isFinite(input.card[k]),
    )
  )
    throw new Error("卡牌信息不完整");
  const graphIds = new Set();
  for (const g of input.graphs) {
    if (
      typeof g.id !== "string" ||
      graphIds.has(g.id) ||
      typeof g.name !== "string" ||
      !Array.isArray(g.nodes) ||
      !Array.isArray(g.edges) ||
      g.nodes.length > 2000 ||
      g.edges.length > 10000
    )
      throw new Error("事件图结构无效或超出大小限制");
    graphIds.add(g.id);
    const nodes = new Map();
    for (const n of g.nodes) {
      if (
        typeof n.id !== "string" ||
        nodes.has(n.id) ||
        !Number.isFinite(n.x) ||
        !Number.isFinite(n.y) ||
        Math.abs(n.x) > 1e7 ||
        Math.abs(n.y) > 1e7
      )
        throw new Error("节点标识或位置无效");
      const d = n.definition;
      if (
        !d ||
        typeof d.name !== "string" ||
        !Array.isArray(d.pins) ||
        d.pins.length > 150 ||
        typeof d.category !== "string"
      )
        throw new Error("节点定义缺失");
      const pins = new Set();
      for (const p of d.pins) {
        if (
          typeof p.id !== "string" ||
          pins.has(p.id) ||
          typeof p.name !== "string" ||
          typeof p.type !== "string" ||
          !["in", "out"].includes(p.direction)
        )
          throw new Error("端口定义无效");
        pins.add(p.id);
      }
      if (
        n.values !== undefined &&
        (!n.values ||
          typeof n.values !== "object" ||
          Array.isArray(n.values) ||
          Object.values(n.values).some(
            (v) => !["string", "boolean", "number"].includes(typeof v),
          ))
      )
        throw new Error("节点参数值无效");
      n.values = n.values || {};
      n.title = typeof n.title === "string" ? n.title : "";
      n.comment = typeof n.comment === "string" ? n.comment : "";
      nodes.set(n.id, n);
    }
    const edgeIds = new Set(),
      inputs = new Set(),
      execOutputs = new Set();
    for (const e of g.edges) {
      if (typeof e.id !== "string" || edgeIds.has(e.id))
        throw new Error("连接标识重复");
      edgeIds.add(e.id);
      const out = nodes
        .get(e.from?.node)
        ?.definition.pins.find(
          (p) => p.id === e.from.pin && p.direction === "out",
        );
      const into = nodes
        .get(e.to?.node)
        ?.definition.pins.find(
          (p) => p.id === e.to.pin && p.direction === "in",
        );
      if (
        !out ||
        !into ||
        !compatible(out.type, into.type) ||
        e.from.node === e.to.node
      )
        throw new Error("文件含有不存在或类型不匹配的连接");
      const key = JSON.stringify(e.to),
        outkey = JSON.stringify(e.from);
      if (inputs.has(key) || (out.type === "exec" && execOutputs.has(outkey)))
        throw new Error("文件中有重复的输入或执行连接");
      inputs.add(key);
      if (out.type === "exec") execOutputs.add(outkey);
    }
    if (
      !g.view ||
      !Number.isFinite(g.view.x) ||
      !Number.isFinite(g.view.y) ||
      !Number.isFinite(g.view.zoom)
    )
      g.view = { x: 40, y: 0, zoom: 1 };
    g.view.zoom = Math.max(0.2, Math.min(2, g.view.zoom));
  }
  if (!graphIds.has(input.activeGraph)) input.activeGraph = input.graphs[0].id;
  return input;
}
