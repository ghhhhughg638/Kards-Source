// Same clipboard format in the editor and reference viewer. By Codex.
import { getDefinition } from "./blueprint.js";

export const CLIP_KEY = "kards-blueprint-studio.clipboard";
export const CLIP_TAG = "KARDS-BP-NODES:";
export function nodeClip(project, graph, selected) {
  const nodes = structuredClone(graph.nodes.filter((n) => selected.has(n.id) && (!n.definition.fixed || n.definition.bp?.type === "result") && !["entry", "macroEntry", "macroExit"].includes(n.definition.bp?.type)));
  const ids = new Set(nodes.map((n) => n.id));
  const edges = structuredClone(graph.edges.filter((e) => ids.has(e.from.node) && ids.has(e.to.node)));
  // Bind genuine card-owned member getters. Inherited properties and source
  // local/frame reads retain their reflected reference definition.
  for (const n of nodes) {
    const expression = n.definition.reference?.expression;
    const v = expression?.op === "Variable" && expression.scope === "member" && project.variables.find((v) => v.name === expression.name && v.type === expression.type);
    if (!v) continue;
    const old = n.definition.pins.find((p) => p.direction === "out" && p.type !== "exec");
    n.definition = getDefinition(v);
    for (const e of edges) if (e.from.node === n.id && e.from.pin === old?.id) e.from.pin = "value";
  }
  const graphs = [], visited = new Set(), variables = new Set();
  const collect = (list) => {
    for (const n of list) {
      const bp = n.definition.bp;
      if (bp?.variableId && ["get", "set"].includes(bp.type)) variables.add(bp.variableId);
      if (!["call", "macro"].includes(bp?.type) || visited.has(bp.graphId)) continue;
      const dependency = project.graphs.find((g) => g.id === bp.graphId);
      if (dependency) { visited.add(dependency.id); graphs.push(structuredClone(dependency)); collect(dependency.nodes); }
    }
  };
  collect(nodes);
  return { format: "kards-bp-nodes", version: 1, nodes, edges, enums: structuredClone(project.enums || {}), types: structuredClone(project.types || {}), variables: structuredClone(project.variables.filter((v) => variables.has(v.id))), graphs, source: { projectId: project.id, asset: project.name, graph: graph.name } };
}

export function importClipDependencies(project, clip, uid) {
  const vars = new Map(), graphs = new Map(), addedGraphs = new Set();
  project.enums = { ...clip.enums, ...project.enums };
  project.types = { ...clip.types, ...project.types };
  for (const v of clip.variables || []) {
    let into = project.variables.find((x) => x.name === v.name && x.type === v.type);
    if (!into) {
      into = structuredClone(v); into.id = uid();
      for (let i = 2; project.variables.some((x) => x.name === into.name); i++) into.name = `${v.name.slice(0, 60)}_${i}`;
      project.variables.push(into);
    }
    vars.set(v.id, into.id);
  }
  for (const g of clip.graphs || []) {
    if (clip.source?.projectId === project.id && project.graphs.some((x) => x.id === g.id)) { graphs.set(g.id, g.id); continue; }
    const existingOverride = g.override && project.graphs.find((x) => x.override?.function === g.override.function && x.override.class === g.override.class);
    if (existingOverride) { graphs.set(g.id, existingOverride.id); continue; }
    const into = structuredClone(g); into.id = uid();
    for (let i = 2; project.graphs.some((x) => x.name === into.name); i++) into.name = `${g.name.slice(0, 60)}_${i}`;
    graphs.set(g.id, into.id); project.graphs.push(into); addedGraphs.add(into.id);
  }
  const remap = (n) => {
    const bp = n.definition.bp;
    if (vars.has(bp?.variableId) && ["get", "set"].includes(bp.type)) bp.variableId = vars.get(bp.variableId);
    if (graphs.has(bp?.graphId)) bp.graphId = graphs.get(bp.graphId);
  };
  for (const g of project.graphs.filter((g) => addedGraphs.has(g.id))) {
    const ids = new Map(g.nodes.map((n) => [n.id, uid()]));
    for (const n of g.nodes) { const id = n.id; remap(n); n.id = ids.get(id); }
    for (const e of g.edges) { e.id = uid(); e.from.node = ids.get(e.from.node); e.to.node = ids.get(e.to.node); }
  }
  for (const n of clip.nodes) remap(n);
}
