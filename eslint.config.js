import js from "@eslint/js";
import globals from "globals";

export default [
  js.configs.recommended,
  {
    files: ["docs/*.js"],
    languageOptions: { globals: globals.browser, sourceType: "module" },
  },
  {
    files: ["tools/*.mjs"],
    languageOptions: { globals: globals.node, sourceType: "module" },
  },
];
