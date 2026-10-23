"""Extract reference-card Chinese text from the game's own UE locres files."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAGIC = bytes.fromhex("0e147475674a03fc4a15909dc3377f1b")


def read_locres(path):
    data = path.read_bytes()
    position = 0

    def read(fmt):
        nonlocal position
        size = struct.calcsize(fmt)
        result = struct.unpack_from(fmt, data, position)[0]
        position += size
        return result

    def string():
        nonlocal position
        count = read("<i")
        size = abs(count) * (2 if count < 0 else 1)
        if size > len(data) - position:
            raise ValueError(f"Invalid FString in {path}")
        raw = data[position:position + size]
        position += size
        return raw.decode("utf-16-le" if count < 0 else "utf-8").rstrip("\0")

    if data[:16] != MAGIC:
        raise ValueError(f"Unsupported legacy resource: {path}")
    position = 16
    version = read("<B")
    if version not in (1, 2, 3):
        raise ValueError(f"Unsupported locres version {version}")
    table_at = read("<q")
    entries_at = position
    position = table_at
    strings = []
    for _ in range(read("<i")):
        strings.append(string())
        if version >= 2:
            read("<i")  # string reference count
    position = entries_at
    expected = read("<I") if version >= 2 else None

    def key():
        if version >= 2:
            read("<I")
        return string()

    entries = {}
    for _ in range(read("<I")):
        namespace = key()
        for _ in range(read("<I")):
            name, source_hash, index = key(), read("<I"), read("<i")
            entries[(namespace, name)] = (source_hash, strings[index])
    if expected is not None and len(entries) != expected:
        raise ValueError(f"Entry count mismatch: {path}")
    return entries


def generate(exports):
    index = json.loads((ROOT / "docs/reference-data/index.json").read_text(encoding="utf-8"))
    directory = exports / "Localization/Game"
    cultures = [culture for culture in ("zh-Hans", "zh-CN") if (directory / culture / "Game.locres").exists()]
    resources = {culture: read_locres(directory / culture / "Game.locres") for culture in cultures}
    english = read_locres(directory / "en/Game.locres")
    source_matches = {}
    for key, (source_hash, original) in english.items():
        for culture in cultures:
            match = resources[culture].get(key)
            if match and match[0] == source_hash and match[1]:
                source_matches.setdefault(original, set()).add(match[1])
                break
    overrides = json.loads((ROOT / "tools/reference-translation-overrides.json").read_text(encoding="utf-8"))
    cards, missing, mismatches = {}, [], []
    counts = {"title": 0, "text": 0}
    methods = {"game": 0, "exactSource": 0, "reference": 0}
    for meta in index["cards"]:
        payload = json.loads((ROOT / f"docs/reference-data/cards/{meta['id']}.json").read_text(encoding="utf-8"))
        translated = {"origins": {}}
        for field, property_name in (("title", "title"), ("text", "Text")):
            value = payload["defaults"].get(property_name)
            if not isinstance(value, dict):
                continue
            key = (value.get("Namespace", ""), value.get("Key", ""))
            original = value.get("SourceString", value.get("LocalizedString", value.get("CultureInvariantString", "")))
            for culture in cultures:
                match = resources[culture].get(key)
                if not match or not match[1]:
                    continue
                en = english.get(key)
                if en and (en[0] != match[0] or en[1] != original):
                    mismatches.append({"card": meta["name"], "field": field, "namespace": key[0], "key": key[1]})
                    continue
                translated[field] = match[1]
                translated["origins"][field] = "game"
                methods["game"] += 1
                counts[field] += 1
                break
            if field not in translated and len(source_matches.get(original, set())) == 1:
                translated[field] = next(iter(source_matches[original]))
                translated["origins"][field] = "exactSource"
                methods["exactSource"] += 1
                counts[field] += 1
            override = overrides.get(meta["name"], {})
            if field not in translated and field in override:
                if override.get("english" + field[0].upper() + field[1:]) != original:
                    raise ValueError(f"Outdated reference translation: {meta['name']} {field}")
                translated[field] = override[field]
                translated["origins"][field] = "reference"
                methods["reference"] += 1
                counts[field] += 1
        if translated["origins"]:
            cards[meta["id"]] = translated
        absent = [field for field in ("title", "text") if meta.get(field) and field not in translated]
        if absent:
            missing.append({"id": meta["id"], "name": meta["name"], "fields": absent})
    result = {"language": "zh-Hans", "source": "Game localization by namespace/key and matching source text; supplemental reference translations are marked per field", "counts": counts, "methods": methods, "cards": cards}
    dest = ROOT / "docs/reference-data/zh-Hans.json"
    dest.write_text(json.dumps(result, ensure_ascii=False, separators=(",", ":")) + "\n", encoding="utf-8")
    report = {"counts": counts, "methods": methods, "cards": len(cards), "total": len(index["cards"]), "missing": missing, "sourceMismatches": mismatches, "resources": [{"path": f"Localization/Game/{c}/Game.locres", "sha256": hashlib.sha256((directory / c / 'Game.locres').read_bytes()).hexdigest()} for c in cultures]}
    (ROOT / "docs/reference-data/translation-coverage.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"cards": len(cards), "total": len(index["cards"]), **counts, "missingCards": len(missing), "sourceMismatches": len(mismatches)}, ensure_ascii=False))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exports", type=Path, required=True)
    generate(parser.parse_args().exports)

