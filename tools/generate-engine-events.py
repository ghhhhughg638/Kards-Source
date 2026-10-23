"""Generate real Blueprint override/event signatures from selected UE headers."""
import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADERS = {
    "AActor": "Runtime/Engine/Classes/GameFramework/Actor.h",
    "UActorComponent": "Runtime/Engine/Classes/Components/ActorComponent.h",
    "UUserWidget": "Runtime/UMG/Public/Blueprint/UserWidget.h",
    "UGameInstance": "Runtime/Engine/Classes/Engine/GameInstance.h",
    "APlayerController": "Runtime/Engine/Classes/GameFramework/PlayerController.h",
    "APawn": "Runtime/Engine/Classes/GameFramework/Pawn.h",
    "ACharacter": "Runtime/Engine/Classes/GameFramework/Character.h",
    "AGameModeBase": "Runtime/Engine/Classes/GameFramework/GameModeBase.h",
}


def end_parenthesis(text, start):
    depth, quoted, escaped = 0, False, False
    for i in range(start, len(text)):
        c = text[i]
        if c == '"' and not escaped:
            quoted = not quoted
        if not quoted:
            depth += (c == "(") - (c == ")")
            if depth == 0:
                return i
        escaped = c == "\\" and not escaped
    raise ValueError("Unclosed UFUNCTION")


def params(text):
    depth, start = 0, 0
    for i, char in enumerate(text):
        depth += char in "<(["
        depth -= char in ">)]"
        if char == "," and depth == 0:
            yield text[start:i]
            start = i + 1
    if text[start:].strip():
        yield text[start:]


def generate(source):
    nodes = []
    enums = {}
    enum_paths = {"EEndPlayReason": "Runtime/Engine/Classes/Engine/EngineTypes.h", "EMovementMode": "Runtime/Engine/Classes/Engine/EngineTypes.h", "ETravelFailure": "Runtime/Engine/Classes/Engine/EngineBaseTypes.h", "ENetworkFailure": "Runtime/Net/Core/Public/Net/Core/Connection/NetEnums.h", "ETouchIndex": "Runtime/InputCore/Classes/InputCoreTypes.h"}
    for name, path in enum_paths.items():
        text = (source / path).read_text(encoding="utf-8-sig")
        text = re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.S)
        match = re.search(rf"namespace\s+{name}\s*\{{\s*enum\s+Type[^{{]*\{{(.*?)\}}", text, re.S) or re.search(rf"enum\s+{name}\b[^{{]*\{{(.*?)\}}", text, re.S)
        if not match:
            raise ValueError(f"Missing enum definition: {name}")
        enums[name] = [re.match(r"\s*(\w+)", entry)[1] for entry in match[1].split(",") if re.match(r"\s*(\w+)", entry) and "Hidden" not in entry and not re.match(r"\s*\w*MAX\b", entry)]
    for cls, relative in HEADERS.items():
        raw = (source / relative).read_text(encoding="utf-8-sig")
        raw = re.sub(r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*.*?\*/', lambda m: m[0] if m[0].startswith('"') else "\n", raw, flags=re.S)
        for match in re.finditer(r"\bUFUNCTION\s*\(", raw):
            end = end_parenthesis(raw, raw.index("(", match.start()))
            flags = raw[match.start():end + 1]
            if not any(f in flags for f in ("BlueprintImplementableEvent", "BlueprintNativeEvent")):
                continue
            declaration = re.sub(r"//[^\n]*", "", raw[end + 1:raw.index(";", end)]).strip()
            declaration = re.sub(r"\b(?:virtual|UE_DEPRECATED|ENGINE_API|UMG_API)\b", "", declaration).strip()
            function = re.match(r"([\w:<>, *&]+?)\s+(\w+)\s*\((.*)\)\s*(?:const\s*)?(?:override\s*)?$", declaration, re.S)
            if not function:
                raise ValueError(f"Unsupported callback declaration in {relative}: {declaration}")
            ret, name, arguments = function.groups()
            parsed = []
            for argument in params(arguments):
                ref_input = bool(re.search(r"UPARAM\s*\(\s*ref\s*\)", argument, re.I))
                argument = re.sub(r"UPARAM\s*\([^)]*\)", "", argument).split("=")[0].strip()
                item = re.match(r"(.+?)[ \t]*\b(\w+)$", argument, re.S)
                if not item:
                    raise ValueError(argument)
                type_name, param = item.groups()
                output = "&" in type_name and not type_name.startswith("const ") and not ref_input
                type_name = type_name.replace("const ", "").replace("&", "").strip()
                if type_name.endswith("::Type") and type_name[:-6] in enums:
                    type_name = type_name[:-6]
                parsed.append((param, type_name, output))
            pure = "BlueprintPure" in flags and not re.search(r"BlueprintPure\s*=\s*false", flags)
            event = ret.strip() == "void" and not any(out for _, _, out in parsed)
            pins = []
            if not pure:
                if not event:
                    pins.append({"id": "exec-in", "name": "Exec", "type": "exec", "direction": "in"})
                pins.append({"id": "exec-out", "name": "Then", "type": "exec", "direction": "out"})
            if not event:
                pins.append({"id": "target", "name": "Target", "type": cls + "*", "direction": "in"})
            pins.extend({"id": p, "name": p, "type": t, "direction": "out" if event or out else "in"} for p, t, out in parsed)
            if ret.strip() != "void":
                pins.append({"id": "return", "name": "Return Value", "type": ret.strip(), "direction": "out"})
            nodes.append({"id": f"engine.{cls}.{name}", "name": name, "class": cls, "event": event, "pure": pure, "kind": "event" if event else "pure" if pure else "function", "category": "events" if event else "functions", "nativeEvent": "BlueprintNativeEvent" in flags, "implementable": "BlueprintImplementableEvent" in flags, "source": "Engine/Source/" + relative, "signature": re.sub(r"\s+", " ", declaration), "pins": pins})
    (ROOT / "docs/engine-events.json").write_text(json.dumps({"engine": "5.6", "enums": enums, "nodes": nodes}, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"members": len(nodes), "events": sum(n["event"] for n in nodes), "functions": sum(not n["event"] for n in nodes)}))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine-source", type=Path, required=True)
    generate(parser.parse_args().engine_source)
