#!/usr/bin/env python3
"""Reconstruct the original card reference corpus from FModel Kismet exports.

No editor layout is present in cooked packages. Preserve every instruction and
its address, typed properties, function signatures and class defaults; the web
viewer derives a compact graph without pretending to recover original positions.
Usage: python tools/generate-reference-library.py --exports <.../kards/Content>
"""
import argparse
import hashlib
import json
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CALLS = {"VirtualFunction", "LocalVirtualFunction", "FinalFunction", "LocalFinalFunction", "CallMath", "CallMulticastDelegate"}
VARIABLES = {"LocalVariable", "LocalOutVariable", "InstanceVariable", "DefaultVariable"}
SUPPORTED = set("LocalVariable InstanceVariable Context Let IntConst CallMath LetBool FinalFunction LocalVirtualFunction False StringConst LocalOutVariable Jump Return EndOfScript Nothing JumpIfNot LetValueOnPersistentFrame ObjectConst PopExecutionFlow PopExecutionFlowIfNot ByteConst LocalFinalFunction True Self PushExecutionFlow LetObj ComputedJump NameConst TextConst FloatConst StructMemberContext SetArray VirtualFunction StructConst NoObject SwitchValue DoubleConst UnicodeStringConst ArrayGetByRef DynamicCast Cast BindDelegate AddMulticastDelegate VectorConst".split())
CPP_CLASSES = {n: "A" + n for n in ["Actor", "Pawn", "Character", "PlayerController", "Controller", "PlayerState", "GameStateBase", "GameModeBase", "HUD", "StaticMeshActor"]}


def object_name(v):
    text = v.get("ObjectName", "") if isinstance(v, dict) else str(v or "")
    return text.split("'")[1] if "'" in text else text


def typename(p):
    if not isinstance(p, dict):
        return "unknown"
    if p.get("Enum"):
        return object_name(p["Enum"]).split("::")[0]
    t = p.get("Type", "")
    if t == "ArrayProperty":
        return "TArray<" + typename(p.get("Inner")) + ">"
    if t == "SetProperty":
        return "TSet<" + typename(p.get("ElementProp")) + ">"
    if t == "MapProperty":
        return "TMap<" + typename(p.get("KeyProp")) + "," + typename(p.get("ValueProp")) + ">"
    if t == "StructProperty":
        n = object_name(p.get("Struct"))
        return n if n.startswith("F") else "F" + n
    if t in {"ObjectProperty", "SoftObjectProperty", "ClassProperty", "SoftClassProperty", "InterfaceProperty"}:
        n = object_name(p.get("MetaClass") or p.get("PropertyClass") or p.get("InterfaceClass")) or "UObject"
        if not re.match(r"^[UA][A-Z]", n):
            n = CPP_CLASSES.get(n, "U" + n)
        return ("TSubclassOf<" + n + ">") if "ClassProperty" in t else n + "*"
    return {"BoolProperty": "bool", "IntProperty": "int32", "Int64Property": "int64", "UInt32Property": "uint32", "ByteProperty": "uint8", "FloatProperty": "float", "DoubleProperty": "double", "StrProperty": "FString", "NameProperty": "FName", "TextProperty": "FText", "MulticastInlineDelegateProperty": "Delegate", "DelegateProperty": "Delegate"}.get(t, "unknown")


def flags(p, name):
    return name in re.split(r"[\s|,]+", p.get("PropertyFlags", ""))


def text_value(v):
    if isinstance(v, str):
        return v
    if isinstance(v, dict):
        value = v.get("LocalizedString") or v.get("SourceString") or v.get("CultureInvariantString") or v.get("Key") or v.get("Value") or ""
        return text_value(value) if isinstance(value, dict) else str(value)
    return ""


def prop(p):
    return {"name": p["Name"], "type": typename(p), "direction": "out" if flags(p, "OutParm") and not flags(p, "ConstParm") else "in", **({"return": True} if flags(p, "ReturnParm") else {})}


def signature(f, owner):
    return {"name": f["Name"], "owner": owner, "params": [prop(p) for p in f.get("ChildProperties", []) if flags(p, "Parm")], "pure": "FUNC_BlueprintPure" in f.get("FunctionFlags", ""), "event": "FUNC_Event" in f.get("FunctionFlags", ""), "flags": f.get("FunctionFlags", "")}


def generate(source, engine=None):
    catalog = json.loads((ROOT / "docs/catalog.json").read_text(encoding="utf-8-sig"))
    for cls, info in catalog["classes"].items():
        for n in [cls, info.get("parent", "")]:
            if re.match(r"^[UA][A-Z]", n):
                CPP_CLASSES[n[1:]] = n
    reflected_classes = dict(catalog["classes"])
    skirmish_path = source / "Blueprints/BP_SkirmishMode.json"
    if skirmish_path.exists():
        skirmish = next((e for e in json.loads(skirmish_path.read_text(encoding="utf-8-sig")) if e.get("Type") == "BlueprintGeneratedClass"), None)
        if skirmish and object_name(skirmish.get("SuperStruct")) == "Actor":
            reflected_classes["U" + skirmish["Name"]] = {"parent": "AActor", "source": "Exports/Blueprints/BP_SkirmishMode.json:SuperStruct"}
    (ROOT / "docs/reference-types.js").write_text("// Generated from catalog and source class reflection by generate-reference-library.py.\nexport const referenceTypes = " + json.dumps(reflected_classes, ensure_ascii=False, indent=2) + ";\n", encoding="utf-8")
    enums = dict(catalog["enums"])
    enum_values = {k: {str(i): v for i, v in enumerate(vs)} for k, vs in enums.items()}
    enum_labels = {}
    # Explicit enum values must not be confused with positions in a dropdown.
    for header in ROOT.glob("*/Public/*.h"):
        raw = header.read_text(encoding="utf-8-sig")
        for match in re.finditer(r"enum\s+class\s+(\w+)[^{]*\{([^}]+)", raw):
            counter = 0
            values = {}
            for item in match[2].split(","):
                item = re.sub(r"//[^\n]*", "", item).strip()
                m = re.match(r"(\w+)(?:\s*=\s*(0[xX][0-9a-fA-F]+|\d+))?", item)
                if not m:
                    continue
                if m[2]:
                    counter = int(m[2], 0)
                if "_MAX" not in m[1] and "UMETA(Hidden)" not in item:
                    values[str(counter)] = m[1]
                counter += 1
            if match[1] in enums:
                enum_values[match[1]] = values
    enum_file = source / "Structs/CardMoveReason.json"
    if enum_file.exists():
        e = json.loads(enum_file.read_text(encoding="utf-8-sig"))[0]
        values = {str(v): n.split("::")[-1] for n, v in e.get("Names", {}).items() if not n.endswith("_MAX")}
        enums[e["Name"]] = [n for _, n in sorted(values.items(), key=lambda pair: int(pair[0]))]
        enum_values[e["Name"]] = values
        enum_labels[e["Name"]] = {x["Key"]: text_value(x["Value"]) for x in e.get("Properties", {}).get("DisplayNameMap", [])}
    signatures = {}
    by_name = {}
    parents = {}
    property_types = {}
    for p in catalog.get("properties", []):
        owner = re.sub(r"^[UA](?=[A-Z])", "", p["class"])
        property_types.setdefault(owner, {})[p["name"].lower()] = p["type"]
    for d in catalog["nodes"]:
        owner = re.sub(r"^[UA](?=[A-Z])", "", d["class"])
        params = [{"name": p["id"], "type": p["type"], "direction": p["direction"], **({"return": True} if p["id"] == "return" else {})} for p in d["pins"] if p["type"] != "exec" and p["id"] != "target"]
        s = {"name": d["name"], "owner": owner, "params": params, "pure": d["pure"], "event": d["event"], "catalogId": d["id"]}
        signatures[owner + "." + d["name"]] = s
        by_name.setdefault(d["name"], []).append(s)
    # Engine reflection declarations provide real argument names/directions too.
    # Their dummy Array/Map/Set element types are inferred from each call site.
    if engine and engine.exists():
        headers = list((engine / "Engine/Classes/Kismet").glob("*.h"))
        headers += list((engine / "GameplayTags/Classes").glob("*Library.h"))
        for header in headers:
            raw = header.read_text(encoding="utf-8-sig", errors="replace")
            cls = re.search(r"class\s+(?:\w+_API\s+)?U(\w+)\s*:", raw)
            if not cls:
                continue
            for m in re.finditer(r"\bUFUNCTION\s*\(", raw):
                start = raw.index("(", m.start()); depth = 1; end = start + 1
                while end < len(raw) and depth:
                    depth += (raw[end] == "(") - (raw[end] == ")"); end += 1
                semi = raw.find(";", end)
                if semi < 0:
                    continue
                flags_text = raw[start:end]
                decl = re.sub(r"//[^\n]*", "", raw[end:semi])
                decl = re.sub(r"\b(?:static|virtual|\w+_API)\b", "", decl).strip()
                match = re.match(r"([\w:<>, *&]+?)\s+(\w+)\s*\((.*)\)\s*(?:const)?$", decl, re.S)
                if not match:
                    continue
                ret, name, args = match.groups(); parts = []; level = 0; begin = 0
                for i, char in enumerate(args):
                    level += (char in "<(") - (char in ">)")
                    if char == "," and level == 0:
                        parts.append(args[begin:i]); begin = i + 1
                if args[begin:].strip():
                    parts.append(args[begin:])
                params = []
                for part in parts:
                    ref_input = bool(re.search(r"UPARAM\s*\(\s*ref\s*\)", part, re.I))
                    part = re.sub(r"UPARAM\s*\([^)]*\)", "", part).split("=")[0].strip()
                    p = re.match(r"(.+?)\b(\w+)$", part, re.S)
                    if not p:
                        continue
                    pt, pn = p.groups(); pt = pt.strip()
                    params.append({"name": pn, "type": "unknown" if cls[1] in {"KismetArrayLibrary", "BlueprintMapLibrary", "BlueprintSetLibrary"} else pt.replace("const ", "").replace("&", "").strip(), "direction": "out" if "&" in pt and not pt.startswith("const ") and not ref_input else "in"})
                if ret.strip() != "void":
                    params.append({"name": "ReturnValue", "type": ret.strip(), "direction": "out", "return": True})
                signatures[cls[1] + "." + name] = {"name": name, "owner": cls[1], "params": params, "pure": "BlueprintPure" in flags_text, "event": False, "source": "Engine/Source/Runtime/" + header.relative_to(engine).as_posix()}
    external_loaded = set()
    ops = Counter()
    raw_ops = Counter()
    normalized_ops = Counter()
    unresolved = Counter()

    def get_signature(ref, name, local):
        raw = object_name(ref)
        owner = raw.split(":")[0] if ":" in raw else local
        key = owner + "." + name
        path = ref.get("ObjectPath", "") if isinstance(ref, dict) else ""
        if key not in signatures and path.startswith("kards/Content/"):
            relative = path[len("kards/Content/"):].split(".")[0]
            if relative not in external_loaded:
                external_loaded.add(relative)
                p = source / (relative + ".json")
                if p.exists():
                    es = json.loads(p.read_text(encoding="utf-8-sig"))
                    for f in es:
                        if f.get("Type") == "Function":
                            signatures[owner + "." + f["Name"]] = signature(f, owner)
        if key not in signatures:
            choices = by_name.get(name, [])
            if len(choices) == 1:
                key = choices[0]["owner"] + "." + name
        if key not in signatures and parents.get(owner):
            parent_key = parents[owner] + "." + name
            if parent_key in signatures:
                key = parent_key
        return key

    def variable(e, scope="local"):
        v = e.get("Variable", e)
        p = v.get("Property", {})
        path = v.get("Path", [])
        name = p.get("Name") or (".".join(path) if isinstance(path, list) else path) or "?"
        owner = object_name(v.get("ResolvedOwner") or v.get("Owner"))
        resolved_type = typename(p)
        if resolved_type == "unknown":
            resolved_type = property_types.get(owner, {}).get(name.lower(), "unknown")
        return {"op": "Variable", "name": name, "type": resolved_type, "scope": scope}

    def normalize(e, local):
        if e is None or not isinstance(e, dict):
            return e
        op = e.get("Inst", "").removeprefix("EX_")
        if not op:
            return variable(e) if "Property" in e else e
        ops[op] += 1
        if op not in SUPPORTED:
            unresolved[op] += 1
        n = {"op": op}
        if "StatementIndex" in e:
            n["at"] = e["StatementIndex"]
        if op in VARIABLES:
            return {**n, **variable(e, "member" if op in {"InstanceVariable", "DefaultVariable"} else "local"), "sourceOp": op}
        if op in CALLS:
            ref = e.get("Function", {})
            name = object_name(ref).split(":")[-1] if not isinstance(ref, str) else ref
            if not name:
                name = e.get("FunctionName", "UnknownFunction")
            key = get_signature(ref, name, local)
            args = [normalize(x, local) for x in e.get("Parameters", [])]
            if key not in signatures:
                choices = by_name.get(name, [])
                def score(s):
                    ps = [p for p in s["params"] if not p.get("return")]
                    if len(ps) != len(args):
                        return -1
                    return sum(5 if p["type"] == a.get("type") else 1 if a.get("op") in {"Self", "ByteConst", "IntConst"} else 0 for p, a in zip(ps, args))
                ranked = sorted(((score(s), s) for s in choices), key=lambda pair: pair[0], reverse=True)
                if ranked and ranked[0][0] > 0 and (len(ranked) == 1 or ranked[0][0] > ranked[1][0]):
                    best = ranked[0][1]; key = best["owner"] + "." + name
            if key not in signatures:
                # Do not fabricate declarations absent from the available SDK.
                # Preserve reflected call-site types and label unnamed inputs by
                # position; the inspector exposes this weaker evidence explicitly.
                primitive = {"Self": "UBaseCardObject*", "NameConst": "FName", "StringConst": "FString", "IntConst": "int32", "ByteConst": "uint8", "True": "bool", "False": "bool"}
                params = []
                for i, arg in enumerate(args):
                    out = arg.get("op") == "Variable" and arg.get("name", "").startswith("CallFunc_" + name + "_")
                    params.append({"name": arg["name"].removeprefix("CallFunc_" + name + "_") if out else "Arg" + str(i), "type": arg.get("type") or primitive.get(arg.get("op"), "unknown"), "direction": "out" if out else "in"})
                returns = {ps["callfunc_" + name.lower() + "_returnvalue"] for ps in property_types.values() if "callfunc_" + name.lower() + "_returnvalue" in ps}
                if len(returns) == 1:
                    params.append({"name": "ReturnValue", "type": next(iter(returns)), "direction": "out", "return": True})
                signatures[key] = {"name": name, "owner": key.rsplit(".", 1)[0], "params": params, "pure": False, "event": False, "evidence": "Reflected call-site types; input parameter names unavailable (Arg0, Arg1, ...)."}
            n.update(name=name, signature=key, args=args)
            return n
        if op in {"Context", "Context_FailSilent", "ClassContext"}:
            target = e.get("ObjectExpression", {})
            target_class = target.get("Variable", {}).get("Property", {}).get("PropertyClass", {})
            target_owner = object_name(target_class)
            if not target_owner and target.get("Inst") in {"EX_InstanceVariable", "EX_LocalVariable"}:
                target_owner = re.sub(r"^[UA](?=[A-Z])", "", variable(target).get("type", "").rstrip("*"))
                if target_owner == "unknown":
                    target_owner = ""
            if target.get("Inst") == "EX_ObjectConst":
                target_owner = object_name(target.get("Value")).removeprefix("Default__")
            context = e.get("ContextExpression", {})
            if target_class.get("ObjectPath", "").startswith("kards/Content/") and isinstance(context.get("Function"), str):
                get_signature({"ObjectName": "Function'" + target_owner + ":" + context["Function"] + "'", "ObjectPath": target_class["ObjectPath"]}, context["Function"], target_owner)
            return {**n, "object": normalize(target, local), "value": normalize(context, target_owner or local)}
        if op in {"Let", "LetBool", "LetObj"}:
            return {**n, "target": normalize(e.get("Variable", e.get("VariableExpression")), local), "value": normalize(e.get("Expression", e.get("AssignmentExpression")), local)}
        if op == "LetValueOnPersistentFrame":
            return {**n, "target": variable(e["DestinationProperty"], "frame"), "value": normalize(e.get("AssignmentExpression"), local)}
        if op in {"Jump", "JumpIfNot"}:
            n["to"] = e["CodeOffset"]
        if op in {"JumpIfNot", "PopExecutionFlowIfNot"}:
            n["condition"] = normalize(e["BooleanExpression"], local)
        if op == "PushExecutionFlow":
            n["to"] = e["PushingAddress"]
        if op == "Return":
            n["value"] = normalize(e.get("Expression", e.get("ReturnExpression")), local)
        if op == "ComputedJump":
            n["value"] = normalize(e.get("CodeOffsetExpression", e.get("OffsetExpression", e.get("Expression"))), local)
        if op == "ObjectConst":
            n.update(value=object_name(e.get("Value")), path=e.get("Value", {}).get("ObjectPath", "") if isinstance(e.get("Value"), dict) else "")
            if isinstance(e.get("Value"), dict) and e["Value"].get("ObjectName", "").split("'")[0] in {"Class", "BlueprintGeneratedClass", "WidgetBlueprintGeneratedClass"}:
                n["type"] = "UClass*"
        elif op in {"TextConst"}:
            n.update(value=text_value(e.get("Value", e.get("TextLiteral", e))), type="FText")
            def literal(v):
                if isinstance(v, dict):
                    return normalize(v, local) if "Inst" in v else {k: literal(x) for k, x in v.items()}
                if isinstance(v, list):
                    return [literal(x) for x in v]
                return v
            n["literal"] = literal(e.get("Value", {}))
        elif op in {"NameConst", "StringConst", "UnicodeStringConst", "IntConst", "ByteConst", "FloatConst", "DoubleConst", "VectorConst"}:
            n["value"] = e.get("Value", e.get("Values"))
        elif op == "StructMemberContext":
            n.update(object=normalize(e.get("StructExpression"), local), member=normalize(e.get("StructMemberExpression", e.get("StructMember")), local))
        elif op == "StructConst":
            n.update(type=object_name(e.get("Struct")), values=[normalize(x, local) for x in e.get("Properties", e.get("Value", []))])
        elif op == "SetArray":
            n.update(target=normalize(e.get("AssigningProperty"), local), values=[normalize(x, local) for x in e.get("Elements", [])])
        elif op in {"Cast", "DynamicCast"}:
            n.update(type=object_name(e.get("ClassPtr", e.get("Class", ""))) or e.get("ConversionType", "Cast"), value=normalize(e.get("Target", e.get("Expression")), local))
        elif op == "ArrayGetByRef":
            n.update(array=normalize(e.get("ArrayVariable", e.get("ArrayTerm")), local), index=normalize(e.get("ArrayIndex", e.get("IndexTerm")), local))
        elif op == "SwitchValue":
            n.update(index=normalize(e.get("IndexTerm"), local), cases=[{"match": normalize(c.get("CaseIndexValueTerm"), local), "value": normalize(c.get("CaseTerm"), local)} for c in e.get("Cases", [])], default=normalize(e.get("DefaultTerm"), local))
        elif op in {"BindDelegate", "AddMulticastDelegate"}:
            n.update(fields={k: normalize(v, local) if isinstance(v, dict) else v for k, v in e.items() if k not in {"Inst", "StatementIndex", "ObjectPath"}})
        return n

    dest = ROOT / "docs/reference-data"
    (dest / "cards").mkdir(parents=True, exist_ok=True)
    records = []
    inventory = []
    for p in sorted((source / "Blueprints/Cards").rglob("card_*.json")):
        if "DIY" in p.parts:
            continue
        raw = p.read_bytes()
        exports = json.loads(raw.decode("utf-8-sig"))
        c = next((e for e in exports if e.get("Type") == "BlueprintGeneratedClass"), None)
        if not c:
            continue
        relative = p.relative_to(source).as_posix()
        ident = hashlib.sha256(relative.encode()).hexdigest()[:16]
        owner = c["Name"]
        parents[owner] = object_name(c.get("SuperStruct"))
        property_types[owner] = {x["Name"].lower(): typename(x) for x in c.get("ChildProperties", [])}
        fs = [e for e in exports if e.get("Type") == "Function"]
        for f in fs:
            signatures[owner + "." + f["Name"]] = signature(f, owner)
            property_types[owner + ":" + f["Name"]] = {p["Name"].lower(): typename(p) for p in f.get("ChildProperties", [])}
        cdo = next((e.get("Properties", {}) for e in exports if e.get("Name", "").startswith("Default__")), {})
        scalar = lambda k, default: cdo.get(k, default)
        enum = lambda k, default: str(cdo.get(k, default)).split("::")[-1]
        card = {"title": text_value(cdo.get("title")) or p.stem, "text": text_value(cdo.get("Text")), "flavorText": text_value(cdo.get("flavorText")), "faction": enum("faction", "NotAvailable"), "type": enum("Type", "NotAvailable"), "rarity": enum("rarity", "NotAvailable"), "cost": scalar("kredits", 0), "operationCost": scalar("operationCost", 0), "attack": scalar("attack", 0), "defense": scalar("defense", 0)}
        functions = [{**signature(f, owner), "locals": [prop(x) for x in f.get("ChildProperties", []) if not flags(x, "Parm") and not x["Name"].startswith("UberGraph")], "body": [normalize(s, owner) for s in f.get("ScriptBytecode", [])]} for f in fs]
        def count_ops(v, output, normalized=False):
            if isinstance(v, dict):
                op = v.get("sourceOp") if normalized and v.get("op") == "Variable" else v.get("op") if normalized else v.get("Inst", "").removeprefix("EX_")
                if op:
                    output[op] += 1
                for x in v.values():
                    count_ops(x, output, normalized)
            elif isinstance(v, list):
                for x in v:
                    count_ops(x, output, normalized)
        for f in fs:
            count_ops(f.get("ScriptBytecode", []), raw_ops)
        for f in functions:
            count_ops(f["body"], normalized_ops, True)
        payload = {"format": "kards-original-reference", "version": 1, "id": ident, "name": p.stem, "package": "/Game/" + relative[:-5], "parent": "UBaseCardObject", "card": card, "defaults": cdo, "variables": [prop(x) for x in c.get("ChildProperties", []) if x["Name"] != "UberGraphFrame"], "functions": functions, "provenance": {"source": relative, "sha256": hashlib.sha256(raw).hexdigest(), "method": "FModel cooked Kismet export; reconstructed layout", "author": "Codex"}}
        (dest / "cards" / (ident + ".json")).write_text(json.dumps(payload, ensure_ascii=False, separators=(",", ":")) + "\n", encoding="utf-8")
        def names(v):
            if isinstance(v, dict):
                if v.get("op") in CALLS:
                    yield v["name"]
                for x in v.values():
                    yield from names(x)
            elif isinstance(v, list):
                for x in v:
                    yield from names(x)
        records.append({"id": ident, "name": p.stem, "package": payload["package"], **card, "functionNames": sorted(set(f["name"] for f in functions) | set(names(functions))), "functionCount": len(functions), "statementCount": sum(len(f["body"]) for f in functions), "dataOnly": not any(f["body"] for f in functions)})
        inventory.append({"file": relative, "sha256": payload["provenance"]["sha256"]})
    # Publish only signatures used by the corpus, not local implementation bodies.
    used = set()
    def collect(v):
        if isinstance(v, dict):
            if "signature" in v:
                used.add(v["signature"])
            for x in v.values():
                collect(x)
        elif isinstance(v, list):
            for x in v:
                collect(x)
    for p in (dest / "cards").glob("*.json"):
        collect(json.loads(p.read_text(encoding="utf-8")))
    manifest = {"format": "kards-original-library", "version": 1, "author": "Codex", "sourceDescription": "Original non-DIY card Blueprint exports, including campaigns, tutorials and side-effect holders", "layout": "Reconstructed from bytecode; original editor positions are unavailable", "counts": {"cards": len(records), "functions": sum(c["functionCount"] for c in records), "statements": sum(c["statementCount"] for c in records), "unsupportedOpcodes": sum(unresolved.values())}, "enums": enums, "enumValues": enum_values, "enumLabels": enum_labels, "signatures": {k: signatures[k] for k in sorted(used) if k in signatures}, "cards": records}
    (dest / "index.json").write_text(json.dumps(manifest, ensure_ascii=False, separators=(",", ":")) + "\n", encoding="utf-8")
    (dest / "inventory.json").write_text(json.dumps({"author": "Codex", "cards": inventory, "opcodes": dict(ops), "rawOpcodes": dict(raw_ops), "normalizedOpcodes": dict(normalized_ops), "unsupported": dict(unresolved)}, ensure_ascii=False, separators=(",", ":")) + "\n", encoding="utf-8")
    (dest / "signature-gaps.json").write_text(json.dumps({"note": "Call arguments remain typed and inspectable even when a declaration is absent from available headers.", "keys": sorted(used - signatures.keys())}, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({**manifest["counts"], "signatures": len(manifest["signatures"]), "unresolvedSignatureKeys": len(used - signatures.keys()), "bytes": sum(p.stat().st_size for p in dest.rglob("*.json"))}, ensure_ascii=False))
    if unresolved:
        raise SystemExit("Uncovered opcodes: " + str(dict(unresolved)))
    if raw_ops != normalized_ops:
        raise SystemExit("Instruction preservation mismatch: " + str(raw_ops - normalized_ops) + " / " + str(normalized_ops - raw_ops))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exports", required=True, type=Path)
    parser.add_argument("--engine-runtime", type=Path)
    args = parser.parse_args()
    generate(args.exports, args.engine_runtime)
