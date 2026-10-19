# KardsSource

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
