// Browser-side definitions of common Unreal Blueprint nodes. These are visual schemas.
const pin = (id, type, direction = "in", name = id) => ({
  id,
  name,
  type,
  direction,
});
const execIn = () => pin("exec-in", "exec", "in", "Exec");
const execOut = (id = "exec-out", name = "Then") =>
  pin(id, "exec", "out", name);
const node = (name, label, pins, options = {}) => ({
  id: "ue." + name,
  name,
  label,
  class: "UE",
  category: "flow",
  kind: "flow",
  pins,
  ...options,
});
const pure = (name, label, pins, options = {}) =>
  node(name, label, pins, {
    category: "math",
    kind: "pure",
    pure: true,
    ...options,
  });
export const ueBuiltins = [
  node(
    "Branch",
    "分支",
    [
      execIn(),
      pin("Condition", "bool"),
      execOut("True", "True"),
      execOut("False", "False"),
    ],
    { id: "studio.Branch" },
  ),
  node(
    "Sequence",
    "序列",
    [execIn(), execOut("Then0", "Then 0"), execOut("Then1", "Then 1")],
    { id: "studio.Sequence" },
  ),
  ...[
    ["Boolean", "布尔变量", "bool"],
    ["Integer", "整数变量", "int32"],
    ["String", "字符串变量", "FString"],
  ].map(([name, label, type]) =>
    pure(name, label, [pin("Value", type, "out")], {
      id: "studio." + name,
      category: "variables",
      kind: "variable",
    }),
  ),
  node("DoOnce", "执行一次", [
    execIn(),
    pin("Reset", "exec"),
    pin("StartClosed", "bool"),
    execOut("Completed", "Completed"),
  ]),
  node("DoN", "执行 N 次", [
    execIn(),
    pin("Reset", "exec"),
    pin("N", "int32"),
    execOut(),
    pin("Counter", "int32", "out"),
  ]),
  node("Gate", "门", [
    pin("Enter", "exec"),
    pin("Open", "exec"),
    pin("Close", "exec"),
    pin("Toggle", "exec"),
    pin("StartClosed", "bool"),
    execOut("Exit", "Exit"),
  ]),
  node("FlipFlop", "交替执行", [
    execIn(),
    execOut("A", "A"),
    execOut("B", "B"),
    pin("IsA", "bool", "out"),
  ]),
  node("ForLoop", "For 循环", [
    execIn(),
    pin("FirstIndex", "int32"),
    pin("LastIndex", "int32"),
    execOut("LoopBody", "Loop Body"),
    pin("Index", "int32", "out"),
    execOut("Completed", "Completed"),
  ]),
  node("ForLoopWithBreak", "可中断 For 循环", [
    execIn(),
    pin("FirstIndex", "int32"),
    pin("LastIndex", "int32"),
    pin("Break", "exec"),
    execOut("LoopBody", "Loop Body"),
    pin("Index", "int32", "out"),
    execOut("Completed", "Completed"),
  ]),
  node("WhileLoop", "While 循环", [
    execIn(),
    pin("Condition", "bool"),
    execOut("LoopBody", "Loop Body"),
    execOut("Completed", "Completed"),
  ]),
  node(
    "Delay",
    "延迟",
    [execIn(), pin("Duration", "float"), execOut("Completed", "Completed")],
    { kind: "function", category: "utilities" },
  ),
  node(
    "RetriggerableDelay",
    "可重触发延迟",
    [execIn(), pin("Duration", "float"), execOut("Completed", "Completed")],
    { kind: "function", category: "utilities" },
  ),
  node("SwitchOnInt", "按整数切换", [
    execIn(),
    pin("Selection", "int32"),
    execOut("0", "0"),
    execOut("1", "1"),
    execOut("2", "2"),
    execOut("Default", "Default"),
  ]),
  node(
    "PrintString",
    "打印字符串",
    [
      execIn(),
      pin("InString", "FString"),
      pin("PrintToScreen", "bool"),
      pin("Duration", "float"),
      execOut(),
    ],
    { kind: "function", category: "utilities" },
  ),
  pure(
    "IsValid",
    "是否有效",
    [pin("Object", "UObject*"), pin("return", "bool", "out", "Return Value")],
    { category: "queries" },
  ),
  node("IsValidExec", "是否有效（执行）", [
    execIn(),
    pin("Object", "UObject*"),
    execOut("IsValid", "Is Valid"),
    execOut("IsNotValid", "Is Not Valid"),
  ]),
  pure("Self", "自身引用", [pin("Self", "UBaseCardObject*", "out")], {
    category: "variables",
    kind: "variable",
  }),
  pure("ObjectReference", "对象引用", [pin("Value", "UObject*", "out")], {
    category: "variables",
    kind: "variable",
  }),
  pure("Float", "浮点变量", [pin("Value", "float", "out")], {
    category: "variables",
    kind: "variable",
  }),
  pure("Name", "名称变量", [pin("Value", "FName", "out")], {
    category: "variables",
    kind: "variable",
  }),
  pure("Text", "文本变量", [pin("Value", "FText", "out")], {
    category: "variables",
    kind: "variable",
  }),
  pure(
    "GetDisplayName",
    "获取显示名称",
    [
      pin("Object", "UObject*"),
      pin("return", "FString", "out", "Return Value"),
    ],
    { category: "queries" },
  ),
  pure(
    "EqualObject",
    "对象相等",
    [
      pin("A", "UObject*"),
      pin("B", "UObject*"),
      pin("return", "bool", "out", "Return Value"),
    ],
    { category: "math" },
  ),
  pure("MakeVector", "构造向量", [
    pin("X", "float"),
    pin("Y", "float"),
    pin("Z", "float"),
    pin("return", "FVector", "out", "Return Value"),
  ]),
  pure("BreakVector", "拆分向量", [
    pin("Vector", "FVector"),
    pin("X", "float", "out"),
    pin("Y", "float", "out"),
    pin("Z", "float", "out"),
  ]),
  ...[
    ["Add", "加"],
    ["Subtract", "减"],
    ["Multiply", "乘"],
    ["Divide", "除"],
    ["Min", "最小值"],
    ["Max", "最大值"],
  ].flatMap(([op, label]) =>
    ["int32", "float"].map((type) =>
      pure(
        `${op}_${type}`,
        `${label}（${type === "int32" ? "整数" : "浮点"}）`,
        [
          pin("A", type),
          pin("B", type),
          pin("return", type, "out", "Return Value"),
        ],
      ),
    ),
  ),
  ...[
    ["Greater", "大于"],
    ["Less", "小于"],
    ["Equal", "等于"],
    ["NotEqual", "不等于"],
    ["GreaterEqual", "大于等于"],
    ["LessEqual", "小于等于"],
  ].flatMap(([op, label]) =>
    ["int32", "float"].map((type) =>
      pure(
        `${op}_${type}`,
        `${label}（${type === "int32" ? "整数" : "浮点"}）`,
        [
          pin("A", type),
          pin("B", type),
          pin("return", "bool", "out", "Return Value"),
        ],
      ),
    ),
  ),
  ...["int32", "float"].map((type) =>
    pure("Clamp_" + type, "限制范围", [
      pin("Value", type),
      pin("Min", type),
      pin("Max", type),
      pin("return", type, "out", "Return Value"),
    ]),
  ),
  ...[
    ["AND", "与"],
    ["OR", "或"],
    ["XOR", "异或"],
  ].map(([name, label]) =>
    pure(name, label, [
      pin("A", "bool"),
      pin("B", "bool"),
      pin("return", "bool", "out", "Return Value"),
    ]),
  ),
  pure("NOT", "非", [
    pin("A", "bool"),
    pin("return", "bool", "out", "Return Value"),
  ]),
  ...["int32", "float", "FString", "UBaseCardObject*"].map((type) =>
    pure(
      "Select_" + type,
      "选择",
      [
        pin("Index", "bool"),
        pin("A", type),
        pin("B", type),
        pin("return", type, "out", "Return Value"),
      ],
      { category: "flow" },
    ),
  ),
  pure(
    "AppendString",
    "追加字符串",
    [
      pin("A", "FString"),
      pin("B", "FString"),
      pin("return", "FString", "out", "Return Value"),
    ],
    { category: "utilities" },
  ),
  ...[
    ["int32", "float"],
    ["float", "int32"],
    ["int32", "FString"],
    ["float", "FString"],
    ["bool", "FString"],
    ["FName", "FString"],
    ["FString", "FName"],
    ["FString", "FText"],
    ["FText", "FString"],
  ].map(([from, to]) =>
    pure(
      "Convert_" + from + "_To_" + to,
      `${from} 转 ${to}`,
      [pin("In", from), pin("return", to, "out", "Return Value")],
      { category: "conversion", kind: "conversion" },
    ),
  ),
  // Array library: one set of nodes for any element type ("$T" resolves when something is wired).
  ...(() => {
    const T = "$T",
      array = `TArray<${T}>`,
      wild = (name, label, english, pins, options = {}) => {
        const def = node(name, label, pins, { english, category: "arrays", ...options });
        return { ...def, wild: pins.map((p) => ({ ...p })) };
      },
      wildPure = (name, label, english, pins) =>
        wild(name, label, english, pins, { kind: "pure", pure: true });
    return [
      wild(
        "ForEachLoop",
        "遍历数组",
        "For Each Loop",
        [execIn(), pin("Array", array), execOut("LoopBody", "Loop Body"), pin("ArrayElement", T, "out", "Array Element"), pin("ArrayIndex", "int32", "out", "Array Index"), execOut("Completed", "Completed")],
      ),
      wild(
        "ForEachLoopWithBreak",
        "可中断遍历数组",
        "For Each Loop with Break",
        [execIn(), pin("Array", array), pin("Break", "exec"), execOut("LoopBody", "Loop Body"), pin("ArrayElement", T, "out", "Array Element"), pin("ArrayIndex", "int32", "out", "Array Index"), execOut("Completed", "Completed")],
      ),
      wildPure("ArrayLength", "数组长度", "Length", [pin("Array", array), pin("return", "int32", "out", "Length")]),
      wildPure("GetArrayItem", "获取数组元素", "Get", [pin("Array", array), pin("Index", "int32"), pin("Item", T, "out")]),
      wildPure("MakeArray", "构造数组", "Make Array", [pin("Item0", T, "in", "0"), pin("Item1", T, "in", "1"), pin("Array", array, "out")]),
      wildPure("ArrayContains", "数组包含元素", "Contains Item", [pin("Array", array), pin("ItemToFind", T, "in", "Item To Find"), pin("return", "bool", "out", "Return Value")]),
      wildPure("ArrayFind", "查找元素下标", "Find Item", [pin("Array", array), pin("ItemToFind", T, "in", "Item To Find"), pin("return", "int32", "out", "Index")]),
      wildPure("ArrayLastIndex", "最后一个下标", "Last Index", [pin("Array", array), pin("return", "int32", "out", "Return Value")]),
      wildPure("ArrayIsValidIndex", "下标是否有效", "Is Valid Index", [pin("Array", array), pin("IndexToTest", "int32", "in", "Index To Test"), pin("return", "bool", "out", "Return Value")]),
      wild("AddUniqueArrayItem", "添加不重复元素", "Add Unique", [execIn(), pin("Array", array), pin("NewItem", T, "in", "New Item"), execOut(), pin("Index", "int32", "out")], { kind: "function" }),
      wild("RemoveArrayItem", "移除数组元素", "Remove Item", [execIn(), pin("Array", array), pin("Item", T), execOut(), pin("return", "bool", "out", "Return Value")], { kind: "function" }),
      wild("ClearArray", "清空数组", "Clear", [execIn(), pin("Array", array), execOut()], { kind: "function" }),
      wild("AddArrayItem", "添加数组元素", "Add", [execIn(), pin("Array", array), pin("NewItem", T, "in", "New Item"), execOut(), pin("Index", "int32", "out")], { kind: "function" }),
      wildPure("ArrayIsEmpty", "数组为空", "Is Empty", [pin("Array", array), pin("return", "bool", "out", "Return Value")]),
      wildPure("ArrayIsNotEmpty", "数组不为空", "Is Not Empty", [pin("Array", array), pin("return", "bool", "out", "Return Value")]),
      wildPure("ArrayFirstItem", "第一个元素", "First Item", [pin("Array", array), pin("Item", T, "out")]),
      wildPure("ArrayLastItem", "最后一个元素", "Last Item", [pin("Array", array), pin("Item", T, "out")]),
      wild("InsertArrayItem", "插入数组元素", "Insert", [execIn(), pin("Array", array), pin("NewItem", T, "in", "New Item"), pin("Index", "int32"), execOut()], { kind: "function" }),
      wild("RemoveArrayIndex", "按下标移除", "Remove Index", [execIn(), pin("Array", array), pin("IndexToRemove", "int32", "in", "Index To Remove"), execOut()], { kind: "function" }),
      wild("SetArrayElem", "设置数组元素", "Set Array Elem", [execIn(), pin("Array", array), pin("Index", "int32"), pin("Item", T), pin("SizeToFit", "bool", "in", "Size to Fit"), execOut()], { kind: "function" }),
      wild("AppendArray", "追加数组", "Append Array", [execIn(), pin("TargetArray", array, "in", "Target Array"), pin("SourceArray", array, "in", "Source Array"), execOut()], { kind: "function" }),
      wild("ReverseArray", "反转数组", "Reverse", [execIn(), pin("Array", array), execOut()], { kind: "function" }),
      wild("ShuffleArray", "打乱数组", "Shuffle", [execIn(), pin("Array", array), execOut()], { kind: "function" }),
      wild("SwapArrayElements", "交换两个元素", "Swap", [execIn(), pin("Array", array), pin("FirstIndex", "int32", "in", "First Index"), pin("SecondIndex", "int32", "in", "Second Index"), execOut()], { kind: "function" }),
      wild("ResizeArray", "调整数组长度", "Resize", [execIn(), pin("Array", array), pin("Size", "int32"), execOut()], { kind: "function" }),
    ];
  })(),
];
// Engine accessors make object-to-KARDS casting workflows possible from familiar UE nodes.
ueBuiltins.push(
  node("ReceiveBeginPlay", "开始游戏", [execOut()], {
    category: "events",
    kind: "event",
    event: true,
    class: "AActor",
  }),
  node(
    "ReceiveTick",
    "每帧更新",
    [execOut(), pin("DeltaSeconds", "float", "out")],
    { category: "events", kind: "event", event: true, class: "AActor" },
  ),
  pure(
    "GetGameInstance",
    "获取游戏实例",
    [pin("return", "UGameInstance*", "out", "Return Value")],
    { category: "queries" },
  ),
  pure(
    "GetGameMode",
    "获取游戏模式",
    [pin("return", "AGameModeBase*", "out", "Return Value")],
    { category: "queries" },
  ),
  pure(
    "GetGameState",
    "获取游戏状态",
    [pin("return", "AGameStateBase*", "out", "Return Value")],
    { category: "queries" },
  ),
  pure(
    "GetPlayerController",
    "获取玩家控制器",
    [
      pin("PlayerIndex", "int32"),
      pin("return", "APlayerController*", "out", "Return Value"),
    ],
    { category: "queries" },
  ),
  pure(
    "GetPlayerPawn",
    "获取玩家 Pawn",
    [
      pin("PlayerIndex", "int32"),
      pin("return", "APawn*", "out", "Return Value"),
    ],
    { category: "queries" },
  ),
  pure(
    "GetWorldDeltaSeconds",
    "获取帧间隔",
    [pin("return", "float", "out", "Return Value")],
    { category: "queries" },
  ),
  pure(
    "GetOwner",
    "获取拥有者",
    [
      pin("target", "AActor*", "in", "Target"),
      pin("return", "AActor*", "out", "Return Value"),
    ],
    { class: "AActor", category: "queries" },
  ),
  pure(
    "GetActorLocation",
    "获取 Actor 位置",
    [
      pin("target", "AActor*", "in", "Target"),
      pin("return", "FVector", "out", "Return Value"),
    ],
    { class: "AActor", category: "queries" },
  ),
  node(
    "SetActorLocation",
    "设置 Actor 位置",
    [
      execIn(),
      pin("target", "AActor*", "in", "Target"),
      pin("NewLocation", "FVector"),
      pin("Sweep", "bool"),
      pin("Teleport", "bool"),
      execOut(),
      pin("return", "bool", "out", "Return Value"),
    ],
    { class: "AActor", category: "utilities", kind: "function" },
  ),
  pure(
    "GetClass",
    "获取类型",
    [
      pin("Object", "UObject*"),
      pin("return", "UClass*", "out", "Return Value"),
    ],
    { category: "queries" },
  ),
);
export const engineClasses = {
  UObject: { parent: null },
  UClass: { parent: "UObject" },
  AGameModeBase: { parent: "AActor" },
  AGameStateBase: { parent: "AActor" },
  UDataAsset: { parent: "UObject" },
  UPrimaryDataAsset: { parent: "UDataAsset" },
  AActor: { parent: "UObject" },
  AController: { parent: "AActor" },
  APlayerController: { parent: "AController" },
  APawn: { parent: "AActor" },
  ACharacter: { parent: "APawn" },
  UActorComponent: { parent: "UObject" },
  USceneComponent: { parent: "UActorComponent" },
  UWidget: { parent: "UObject" },
  UUserWidget: { parent: "UWidget" },
  UGameInstance: { parent: "UObject" },
  USubsystem: { parent: "UObject" },
  UGameInstanceSubsystem: { parent: "USubsystem" },
  UEngineSubsystem: { parent: "USubsystem" },
  UBlueprintFunctionLibrary: { parent: "UObject" },
};
export function typeNodes(classes, properties) {
  const casts = Object.keys(classes)
    .filter((c) => c !== "UObject")
    .map((cls) =>
      node(
        "Cast To " + cls.replace(/^[UA](?=[A-Z])/, " ").trim(),
        "转换为 " + cls,
        [
          execIn(),
          pin("Object", "UObject*"),
          execOut("Success", "Success"),
          execOut("Failed", "Cast Failed"),
          pin(
            "As" + cls,
            cls + "*",
            "out",
            "As " + cls.replace(/^[UA](?=[A-Z])/, ""),
          ),
        ],
        {
          id: "ue.cast." + cls,
          category: "conversion",
          kind: "cast",
          class: cls,
          targetClass: cls,
        },
      ),
    );
  const accessors = properties.flatMap((p) => {
    const target = () => pin("target", p.class + "*", "in", "Target");
    const base = {
      class: p.class,
      source: p.source,
      property: p.name,
      category: "properties",
    };
    const getter = pure(
      "Get " + p.name,
      "获取 " + p.name,
      [target(), pin("value", p.type, "out", p.name)],
      {
        ...base,
        id: p.class + ".get." + p.name,
        kind: "getter",
        signature: `${p.class}::${p.name} : ${p.type}`,
      },
    );
    const setter = node(
      "Set " + p.name,
      "设置 " + p.name,
      [
        execIn(),
        target(),
        pin("value", p.type, "in", p.name),
        execOut(),
        pin("result", p.type, "out", "Output"),
      ],
      {
        ...base,
        id: p.class + ".set." + p.name,
        kind: "setter",
        signature: `${p.class}::${p.name} = ${p.type}`,
      },
    );
    return p.writable ? [getter, setter] : [getter];
  });
  return [...casts, ...accessors];
}
