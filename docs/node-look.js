// How a node looks, shared by the editor (app.js) and the original-card viewer (reference.js) so a
// node is drawn the same way in both places.
import { nodeKind, kindLabels } from "./i18n.js";

const ICONS = {
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
};
export const nodeIcon = (d) => ICONS[nodeKind(d)] || "ƒ";

// Visual size class: simple data nodes (Get Self, Get Card ID, literals) stay small, while
// busy nodes (Change Attack and friends) get larger ports and rows that are easier to wire.
export function nodeTier(d) {
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
export const pinEnglish = (p) => (p.name === "执行" ? "Exec" : p.name === "然后" ? "Then" : p.name);
export const kindEnglish = (n) =>
  `${(kindLabels[nodeKind(n.definition)] || "FUNCTION").split(" · ")[0]} · ${(n.definition.class || "UE").replace(/^[UA](?=[A-Z][a-z])/, "")}`;
