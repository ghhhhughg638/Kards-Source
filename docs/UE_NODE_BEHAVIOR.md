# UE node editing · Codex

The editor and reference view share node capabilities and colors in `ue-node-features.js`.

## Variable pins

- Associative operators use `CommutativeAssociativeBinaryOperator` metadata from UE headers. AND, OR, NAND, supported addition/multiplication, Min/Max and Append follow that metadata. Subtract, Divide, NOT and fixed function calls cannot add arbitrary inputs.
- Operands retain at least two inputs, up to 26. Sequence and MultiGate retain at least one execution output. Make Array can remove every item. Integer Select retains two options; Boolean Select stays binary.
- Integer, Name and String switches can add/remove cases. Enum switch/select pins follow the enum definition instead of accepting arbitrary cases.
- Add buttons and individual remove buttons work on touch screens. Right-click a pin in the main editor to remove it or disconnect its wires.
- Removing a pin removes only its own values, folded expressions and incident wires. Identifiers on remaining pins stay stable. Changes participate in the main editor's undo/redo, workspace storage and v2 export/import. Wildcard templates are updated with their visible pins.
- Modified nodes retain their UE editor node class and ordered variable-pin IDs. This remains a visual reference/design format; it does not claim Unreal compilation or game execution.

## Presentation

`ue-editor-metadata.js` is generated from UE 5.6 `GraphEditorSettings.cpp`, `KismetMathLibrary.h` and `KismetStringLibrary.h`. Linear color values are converted to sRGB for the browser. Boolean pins/wires are maroon, bytes/enums dark teal, and object, class, numeric and struct pins have separate colors. Container pins use the element/key color. Pure-function header colors are separate from data-type colors; compact AND/OR and arithmetic nodes use a dark body without the regular function header.

Regenerate with:

```sh
python tools/generate-ue-editor-metadata.py --engine-source /path/to/UE_5.6/Engine/Source
```
