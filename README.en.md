# KardsSource

## Credits

- Editor, node library, workspace, performance and UI work, update mechanism and later fixes: **Claude** (Anthropic).
- Original-card Blueprint data conversion and the first viewer: **Codex** (OpenAI). Since 2.17.0 Claude has aligned the viewer's UI and node styling with the editor.

Commits carry `Co-Authored-By: Claude`, and every changelog entry names its author.

## Original-card reference library — Codex

Open **Original Cards** from the editor header to browse all 2,019 non-DIY card Blueprint exports, 5,585 functions, and 80,736 top-level source instructions. The library supports filtering, function navigation, typed enum controls, compact expressions and advanced pins, source/default inspection, and editable workspace copies. Cooked data does not contain original editor positions: the logical layout is reconstructed. Implementation and conversion by **Codex**. See [the reference guide](docs/REFERENCE_LIBRARY.md).

Reconstructed Unreal Engine 5.6 C++ source tree for the KARDS client, plus **KARDS Blueprint Studio**, a browser-based blueprint editor (see `README.md` for full Studio documentation, in Chinese).

## Layout
- `kards/` – main game module (cards, gameplay effects, enums, many `*Stub` / `*EditorShim` placeholder classes; files marked `// STUB:` contain no real logic).
- `KardsCore/`, `KardsSubsystems/`, `BlueprintJson/` – supporting modules.
- `KardsLocalizationTools/` – editor commandlets for asset/localization reconstruction.
- `docs/` – static Blueprint Studio site (deployed to GitHub Pages).
- `tools/` – node catalog generator and tests.

There is no `.uproject` in this repo, so the C++ modules cannot be built standalone.

## Studio development
```
npm install
npm test                 # unit tests
npm run lint             # ESLint
npm run catalog          # regenerate docs/catalog.json from C++ headers
python3 -m http.server 4173 --directory docs
```
CI (`.github/workflows/ci.yml`) runs tests, lint and verifies `docs/catalog.json` is up to date.
