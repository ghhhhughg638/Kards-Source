// A workspace holds several cards (each one a Blueprint project) in the browser's storage.
// Every card has its own storage entry; a small index remembers the order, the active card
// and enough metadata (name, title, faction) to list cards without parsing them.
export const WORKSPACE_KEY = "kards-blueprint-studio.workspace";
export const CARD_PREFIX = "kards-blueprint-studio.card.";
// The single-project key used before workspaces existed.
export const LEGACY_KEY = "kards-blueprint-studio.v1";

export function createWorkspace(storage) {
  let index = { order: [], active: null, meta: {} };
  const cardKey = (id) => CARD_PREFIX + id;
  const writeIndex = () => storage.setItem(WORKSPACE_KEY, JSON.stringify(index));
  const metaOf = (p) => ({
    name: p.name,
    title: p.card?.title ?? "",
    faction: p.card?.faction ?? "",
    type: p.card?.type ?? "",
    updated: Date.now(),
  });
  const api = {
    // Reads the index; the first time, adopts the pre-workspace single project.
    load() {
      try {
        const raw = JSON.parse(storage.getItem(WORKSPACE_KEY) || "null");
        if (raw && Array.isArray(raw.order))
          index = {
            order: raw.order.filter((id) => typeof id === "string" && storage.getItem(cardKey(id)) !== null),
            active: typeof raw.active === "string" ? raw.active : null,
            meta: raw.meta && typeof raw.meta === "object" ? raw.meta : {},
          };
      } catch {
        index = { order: [], active: null, meta: {} };
      }
      if (!index.order.length) {
        try {
          const legacy = JSON.parse(storage.getItem(LEGACY_KEY) || "null");
          if (legacy && typeof legacy.id === "string") {
            api.save(legacy);
            storage.removeItem(LEGACY_KEY);
          }
        } catch {
          /* unreadable legacy draft: start empty */
        }
      }
      if (!index.order.includes(index.active)) index.active = index.order[0] ?? null;
      return api;
    },
    ids: () => [...index.order],
    active: () => index.active,
    has: (id) => index.order.includes(id),
    meta: (id) => index.meta[id] || { name: id, title: "", faction: "", type: "", updated: 0 },
    list: () => index.order.map((id) => ({ id, ...api.meta(id) })),
    read(id) {
      try {
        return JSON.parse(storage.getItem(cardKey(id)));
      } catch {
        return null;
      }
    },
    // Saves the card and makes it the active one. Throws if storage is full.
    save(project, { activate = true } = {}) {
      storage.setItem(cardKey(project.id), JSON.stringify(project));
      if (!index.order.includes(project.id)) index.order.push(project.id);
      index.meta[project.id] = metaOf(project);
      if (activate) index.active = project.id;
      writeIndex();
    },
    setActive(id) {
      if (!index.order.includes(id)) return;
      index.active = id;
      writeIndex();
    },
    remove(id) {
      const at = index.order.indexOf(id);
      if (at < 0) return;
      storage.removeItem(cardKey(id));
      index.order.splice(at, 1);
      delete index.meta[id];
      if (index.active === id) index.active = index.order[Math.min(at, index.order.length - 1)] ?? null;
      writeIndex();
    },
    // Asset names are unique across the workspace (case-insensitive), like files in one folder.
    nameTaken(name, exceptId = null) {
      const lower = name.toLowerCase();
      return index.order.some((id) => id !== exceptId && String(api.meta(id).name).toLowerCase() === lower);
    },
    uniqueName(name, exceptId = null) {
      if (!api.nameTaken(name, exceptId)) return name;
      const base = name.replace(/_\d+$/, "");
      for (let n = 2; ; n++) {
        const candidate = `${base}_${n}`.slice(0, 64);
        if (!api.nameTaken(candidate, exceptId)) return candidate;
      }
    },
  };
  return api;
}
