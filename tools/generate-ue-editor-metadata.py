"""Read UE editor colors and function-node metadata from installed engine headers."""
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def generate(engine):
    functions = {}
    for name in ("KismetMathLibrary", "KismetStringLibrary"):
        path = engine / f"Runtime/Engine/Classes/Kismet/{name}.h"
        raw = path.read_text(encoding="utf-8-sig")
        for match in re.finditer(r"UFUNCTION\((.*?)\)\s*static\s+[\w:<>*&\s]+?\s+(\w+)\s*\(", raw, re.S):
            flags, function = match.groups()
            title = re.search(r'CompactNodeTitle\s*=\s*"([^"]*)"', flags)
            associative = bool(re.search(r'CommutativeAssociativeBinaryOperator\s*=\s*"true"', flags))
            if title or associative:
                functions[function] = {"compactTitle": title[1] if title else "", "associative": associative}
    raw = (engine / "Editor/GraphEditor/Private/GraphEditorSettings.cpp").read_text(encoding="utf-8-sig")
    colors = {}
    for match in re.finditer(r"(\w+(?:PinTypeColor|NodeTitleColor))\s*=\s*FLinearColor\(([^)]+)\)", raw):
        channels = [float(v.strip().rstrip("f")) for v in match[2].split(",")[:3]]
        srgb = lambda v: round(255 * (12.92 * v if v <= .0031308 else 1.055 * v ** (1 / 2.4) - .055))
        colors[match[1]] = "#" + "".join(f"{srgb(v):02x}" for v in channels)
    output = "// Generated from UE 5.6 Kismet headers and GraphEditorSettings.cpp.\n"
    output += "export const ueFunctionMetadata = " + json.dumps(dict(sorted(functions.items())), indent=2) + ";\n"
    output += "export const ueEditorColors = " + json.dumps(colors, indent=2) + ";\n"
    (ROOT / "docs/ue-editor-metadata.js").write_text(output, encoding="utf-8")
    print(json.dumps({"functions": len(functions), "associative": sum(v["associative"] for v in functions.values()), "colors": len(colors)}))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine-source", required=True, type=Path)
    generate(parser.parse_args().engine_source)
