// Display-language layer for interface text that is written in Chinese in the source.
// "en" mode swaps known Chinese phrases for English; "zh" mode keeps only the Chinese side
// of "中文 / English" strings. Unknown text (user content) is never touched.
const EXACT = {
  // header and menu
  "项目名称": "Project name",
  "本地已保存": "Saved locally",
  "正在保存…": "Saving…",
  "存储空间不足，请导出": "Out of storage space — please export",
  "多选模式:点节点选中/取消,拖空白处框选": "Multi-select mode: tap nodes to select/deselect, drag on empty canvas to box-select",
  "已退出多选模式": "Left multi-select mode",
  "选择或输入 Reason 键": "Pick or type a Reason key",
  "点击选择 Reason 键，也可以自己输入": "Click to pick a Reason key, or type your own",
  "搜索键或文字（中英文都行）/ Search key or text…": "Search key or text…",
  "编辑父类属性的默认值": "Edit default values of the parent class properties",
  "恢复默认 / Reset": "Reset",
  "搜索属性 / Search properties…": "Search properties…",
  "逗号分隔 / comma separated": "comma separated",
  "详情面板：卡牌信息、类默认值、节点属性": "Details: card data, class defaults, node properties",
  "名称、阵营、类型、稀有度、费用、行动花费、攻击、防御、文字": "Name, faction, type, rarity, cost, operation cost, attack, defense, text",
  "行动花费": "Operation cost",
  "关闭详情": "Close details",
  "关闭工作区": "Close workspace",
  "取消选择": "Deselect",
  "整理节点布局并拆开重叠的节点": "Tidy the layout and separate overlapping nodes",
  "已整理节点布局": "Layout tidied",
  "框选节点 / Box select": "Box select",
  "平移画布 / Pan": "Pan",
  "操作帮助": "Help",
  "导出蓝图": "Export blueprint",
  "项目工具": "Project tools",
  "新建项目": "New project",
  "导入文件": "Import file",
  "保存": "Save",
  "撤销 Ctrl/Cmd+Z": "Undo Ctrl/Cmd+Z",
  "重做 Ctrl/Cmd+Shift+Z": "Redo Ctrl/Cmd+Shift+Z",
  "强制刷新：绕过缓存，重新从 GitHub 读取最新版本（本地草稿不会丢失）":
    "Force refresh: bypass the cache and reload the latest version from GitHub (local drafts are kept)",
  // left panel
  "新建事件图": "New event graph",
  "卡牌蓝图": "Card blueprint",
  "所有节点": "All nodes",
  "拖入画布": "Drag onto canvas",
  "可用节点": "Available nodes",
  "搜索节点": "Search nodes",
  "卡牌": "Card",
  "事件图": "Event Graph",
  "节点库": "Palette",
  "详情": "Details",
  "搜索可添加节点": "Search nodes to add",
  "节点库与蓝图列表": "Node library and graphs",
  "工作区、蓝图列表与节点库": "Workspace, blueprint and node library",
  // canvas
  "添加节点 (Tab)": "Add node (Tab)",
  "添加节点": "Add node",
  "适应画布 (F)": "Fit to canvas (F)",
  "显示/隐藏详情面板": "Show/hide details panel",
  "蓝图画布。滚轮缩放，拖动空白平移，Tab 添加节点":
    "Blueprint canvas. Scroll to zoom, drag empty space to pan, Tab to add a node",
  "卡牌事件图": "Card event graph",
  "节点连线": "Node wires",
  "拖动画布": "Drag to pan",
  "滚轮缩放": "Scroll to zoom",
  "缩小": "Zoom out",
  "重置缩放": "Reset zoom",
  "放大": "Zoom in",
  "画布导航": "Canvas navigator",
  "复制": "Duplicate",
  "删除": "Delete",
  "让一个想法，成为蓝图。": "Turn an idea into a blueprint.",
  "从左侧拖入节点，或按 Tab 开始。":
    "Drag a node in from the left, or press Tab to start.",
  "添加第一个节点": "Add the first node",
  "检查连接": "Check connections",
  "端口颜色图例": "Pin color legend",
  "用于设计、编辑与展示蓝图。": "For designing, editing and presenting blueprints.",
  "蓝图文件保存在你的设备上。": "Blueprint files stay on your device.",
  "KARDS 专用蓝图编辑器": "KARDS blueprint editor",
  "就绪": "Ready",
  "正在读取节点库…": "Loading node library…",
  "导出 SVG": "Export SVG",
  // pin kinds (legend)
  "执行": "Exec",
  "对象": "Object",
  "布尔": "Boolean",
  "整数": "Integer",
  "浮点": "Float",
  "字符串": "String",
  "枚举": "Enum",
  "向量": "Vector",
  "旋转": "Rotator",
  "数组颜色随元素类型变化": "Array color follows its element type",
  "数组·随元素类型": "Array · by element type",
  // factions
  "德国": "Germany",
  "英国": "Britain",
  "日本": "Japan",
  "苏联": "Soviet Union",
  "美国": "USA",
  "法国": "France",
  "意大利": "Italy",
  "波兰": "Poland",
  "芬兰": "Finland",
  "澳新军团": "ANZAC",
  "盟军": "Allies",
  "中立": "Neutral",
  "未指定": "Unspecified",
  // graphs and library
  "关闭": "Close",
  "事件图名称": "Event graph name",
  "新事件图": "New event graph",
  "创建事件图": "Create event graph",
  "事件图设置": "Event graph settings",
  "名称": "Name",
  "删除事件图": "Delete event graph",
  "仅显示前 120 项，输入关键词继续查找":
    "Showing the first 120 items — type a keyword to narrow down",
  "没有匹配的节点": "No matching nodes",
  "双击添加节点，或拖入画布": "Double-click to add a node, or drag it onto the canvas",
  "无兼容操作": "No compatible actions",
  // custom node dialog
  "自定义节点": "Custom node",
  "节点名称": "Node name",
  "节点类型": "Node type",
  "动作（含执行端口）": "Action (with exec pins)",
  "事件（执行输出）": "Event (exec output)",
  "纯数据节点": "Pure data node",
  "数据端口：每行「方向 类型 名称」": "Data pins: one per line “direction type name”",
  "方向为 in / out。支持 bool、int32、float、FString、UBaseCardObject* 及 KARDS 枚举。相同类型才能连线。":
    "Direction is in / out. Supports bool, int32, float, FString, UBaseCardObject* and KARDS enums. Only matching types can be connected.",
  "创建节点": "Create node",
  "请输入节点名称": "Enter a node name",
  "端口格式错误、名称重复或使用了保留类型 exec":
    "Invalid pin format, duplicate name, or reserved type exec",
  // status
  "选择一个兼容端口完成连线 · Esc 取消":
    "Select a compatible pin to finish the wire · Esc to cancel",
  "已选中连接 · Delete 删除": "Wire selected · Delete to remove",
  // inspector
  "连接详情": "Wire details",
  "选中连线后，可按 Delete 删除。每个输入端口只接受一条连接，新的连接会替换原连接。":
    "Press Delete to remove the selected wire. Each input pin accepts one connection; a new connection replaces the old one.",
  "删除连接": "Delete wire",
  "节点": "Node",
  "自定义": "Custom",
  "显示名称": "Display name",
  "节点注释": "Node comment",
  "描述这段逻辑的意图…": "Describe what this logic is for…",
  "位置 X": "Position X",
  "位置 Y": "Position Y",
  "默认参数": "Defaults",
  "由连接提供值 · 请在来源节点上修改":
    "Value comes from a connection · edit the source node",
  "此节点没有可编辑的默认输入。": "This node has no editable default inputs.",
  "源代码": "Source",
  "自定义节点 · 仅用于可视化设计": "Custom node · for visual design only",
  "复制节点": "Duplicate node",
  "删除选中节点": "Delete selected nodes",
  "卡牌信息": "Card info",
  "卡牌名称": "Card name",
  "所属阵营": "Faction",
  "卡牌类型": "Card type",
  "稀有度": "Rarity",
  "攻击力": "Attack",
  "防御力": "Defense",
  "指挥点消耗": "Kredits cost",
  "卡牌效果": "Card text",
  "点击节点可编辑参数和注释。双击顶部标签可重命名或删除事件图。":
    "Click a node to edit parameters and comments. Double-click a tab to rename or delete an event graph.",
  "管理当前事件图": "Manage current event graph",
  "文件格式": "File format",
  "包含资源信息、变量、函数、宏、EventGraph、节点定义、参数和连线。导入后可继续编辑。":
    "Contains asset info, variables, functions, macros, the EventGraph, node definitions, parameters and wires. You can keep editing after importing.",
  "未命名卡牌": "Untitled card",
  "点击切换 true / false": "Click to toggle true / false",
  // toasts
  "已导出蓝图，可随时导入继续编辑": "Blueprint exported — import it any time to keep editing",
  "请先添加节点": "Add a node first",
  "已导出可独立打开的 SVG 图像": "Standalone SVG exported",
  "项目已保存到当前浏览器": "Project saved in this browser",
  "已保存到当前浏览器": "Saved in this browser",
  "已从 GitHub 读取最新版本，正在刷新…": "Latest version fetched from GitHub — reloading…",
  "格式错误，无法导入": "Invalid format — cannot import",
  "文件超过 12 MB 限制": "File exceeds the 12 MB limit",
  "节点库载入失败": "Failed to load node library",
  "无法加载节点库": "Cannot load node library",
  "重新加载": "Reload",
  "本地草稿无法读取，已新建空白项目。可导入备份文件恢复。":
    "The local draft could not be read, so a blank project was created. Import a backup file to restore.",
  // dialogs
  "新建蓝图项目": "New blueprint project",
  "当前项目会保留在撤销记录中。建议先导出文件，方便以后继续编辑。":
    "The current project stays in the undo history. Export it first so you can keep editing later.",
  "导出当前项目": "Export current project",
  "创建项目": "Create project",
  "蓝图检查": "Blueprint check",
  "文件结构、端口类型和连接引用有效。":
    "File structure, pin types and connection references are valid.",
  "检查范围为蓝图文档结构。此编辑器不运行蓝图，也不验证游戏运行结果。":
    "The check covers the blueprint document structure only. This editor does not run blueprints or verify gameplay.",
  "设计你的下一张卡牌": "Design your next card",
  "Tab / 右键": "Tab / Right-click",
  "平移画布": "Pan canvas",
  "拖动空白 / 空格 + 拖动": "Drag empty space / Space + drag",
  "框选节点": "Box select",
  "拖动空白（白色框）": "Drag empty space (white box)",
  "空格 / 中键 / Alt + 拖动": "Space / Middle mouse / Alt + drag",
  "缩放画布": "Zoom canvas",
  "滚轮 / ＋ −": "Scroll / ＋ −",
  "适应所有节点": "Fit all nodes",
  "多选节点": "Multi-select nodes",
  "Shift + 单击": "Shift + Click",
  "Shift + 单击 / 手机「多选」按钮": "Shift + Click / Multi button on phones",
  "全选 / 复制 / 删除": "Select all / duplicate / delete",
  "撤销 / 重做": "Undo / redo",
  "保存到本地 / 导出文件": "Save locally / export file",
  "取消连线 / 取消选择": "Cancel wire / deselect",
  "从一个端口拖向另一个端口，或依次点击两个端口。只有类型相同的输出与输入可以连接。点击连线后按 Delete 可断开。":
    "Drag from one pin to another, or click two pins in turn. Only an output and an input of the same type can be connected. Click a wire and press Delete to disconnect it.",
  "修改会自动保存在当前浏览器。使用「导出蓝图」获得文件；更换设备或清除浏览器数据前请先导出。支持 macOS 的 ⌘ 快捷键。":
    "Changes are saved automatically in this browser. Use “Export blueprint” to get a file; export before switching devices or clearing browser data. macOS ⌘ shortcuts are supported.",

  // blueprint structure (functions, macros, variables, custom events)
  "新建": "New",
  "编辑函数": "Edit function",
  "编辑宏": "Edit macro",
  "新建函数": "New function",
  "新建宏": "New macro",
  "新建变量": "New variable",
  "重载父类函数": "Override parent function",
  "编辑变量": "Edit variable",
  "编辑自定义事件": "Edit custom event",
  "添加自定义事件": "Add custom event",
  "说明": "Description",
  "可以用中文写说明": "Write a description (any language)",
  "访问": "Access",
  "分类": "Category",
  "纯函数（没有执行引脚）": "Pure function (no exec pins)",
  "输入参数": "Inputs",
  "输出参数": "Outputs",
  "参数": "Parameters",
  "创建": "Create",
  "创建重载": "Create override",
  "没有可以重载的父类函数了。": "There are no parent functions left to override.",
  "实例可编辑": "Instance editable",
  "生成时公开": "Expose on spawn",
  "资源": "Asset",
  "资源名称": "Asset name",
  "父类": "Parent class",
  "包路径": "Package path",
  "图表": "Graph",
  "函数": "Function",
  "宏": "Macro",
  "蓝图": "Blueprint",
  "编辑": "Edit",
  "编辑函数签名": "Edit function signature",
  "编辑宏签名": "Edit macro signature",
  "转到函数": "Go to function",
  "转到宏": "Go to macro",
  "转到事件": "Go to event",
  "获取": "Get",
  "设置": "Set",
  "当前图表不能使用此节点": "This node cannot be used in the current graph",
  "新建函数 / 宏 / 变量": "New function / macro / variable",
  "请输入名称": "Enter a name",
  "名称只能包含英文字母、数字和下划线，不能以数字开头，最长 64 位（不支持中文）":
    "Names may only contain English letters, digits and underscores, cannot start with a digit, and are at most 64 characters (Chinese is not supported)",
  "名称只能包含英文字母、数字和下划线，不能以数字开头（不支持中文）。":
    "Names may only contain English letters, digits and underscores, and cannot start with a digit (Chinese is not supported).",
  "EventGraph 是唯一的事件图，不能再新建。更多逻辑请写在函数、宏或自定义事件里。":
    "EventGraph is the only event graph and cannot be created again. Put more logic in functions, macros or custom events.",
  "EventGraph 是蓝图唯一的事件图，不能新建、重命名或删除。":
    "EventGraph is the blueprint's only event graph; it cannot be created, renamed or deleted.",
  "事件（如“从手牌打出”）和自定义事件都放在这里；更多逻辑请新建函数或宏。":
    "Events (such as “On Played From Hand”) and custom events live here; create a function or macro for more logic.",
  "EventGraph 是蓝图唯一的事件图，不能新建、重命名或删除。点击节点可编辑参数和注释。":
    "EventGraph is the blueprint's only event graph; it cannot be created, renamed or deleted. Click a node to edit its parameters and comments.",
  "Function · 函数": "Function",
  "Macro · 宏": "Macro",
  "Event Graph · 卡牌事件图": "Event Graph",
  "资源信息不完整": "Asset info is incomplete",
  "变量列表无效": "Invalid variable list",
  "变量定义无效或名称重复": "Invalid variable definition or duplicate name",
  "蓝图必须有且仅有一个 EventGraph，并且排在第一位":
    "A blueprint must have exactly one EventGraph, placed first",
  "函数参数无效": "Invalid function parameters",
  "函数参数名无效或重复": "Invalid or duplicate parameter name",
  "事件图之外只能有函数或宏": "Besides the EventGraph there can only be functions or macros",
  "函数或宏的名称无效或重复": "Invalid or duplicate function/macro name",
  "函数定义无效": "Invalid function definition",
  "自定义事件名称无效或重复": "Invalid or duplicate custom event name",
  "自定义事件只能放在 EventGraph 中": "Custom events can only be placed in the EventGraph",
  "工作区": "Workspace",
  "新建卡牌": "New card",
  "导入卡牌": "Import cards",
  "重命名卡牌": "Rename card",
  "复制卡牌": "Duplicate card",
  "删除卡牌": "Delete card",
  "卡牌标题": "Card title",
  "创建卡牌": "Create card",
  "新卡牌会加入左侧的工作区，当前这张卡牌会自动保存。": "The new card is added to the workspace on the left; the current card is saved automatically.",
  "资源名称已被工作区里的另一张卡牌使用": "Another card in the workspace already uses this asset name",
  "找不到这张卡牌的数据": "The data for this card could not be found",
  "自定义倍率 0.5–8": "Custom scale 0.5–8",
  "自定义倍率": "Custom scale",
  "发现新版本": "New version available",
  "暂不更新": "Not now",
  "立即更新": "Update now",
  "更新会重新下载页面文件并刷新；你的卡牌保存在浏览器里，不会丢失。": "Updating re-downloads the page files and reloads; your cards are stored in this browser and are kept.",
  "这个版本包含功能改进和问题修复。": "This version contains improvements and fixes.",
  "局部变量": "Local variables",
  "切换导出格式": "Change export format",
  "PNG 倍率": "PNG scale",
  "已导出 PNG 图片": "PNG image exported",
  "PNG 导出失败，请改用 SVG 导出": "PNG export failed — try SVG instead",
  "全部": "All",
  "每次显示的数量": "Items to add per click",
  "事件分发器": "Event dispatchers",
  "新建事件分发器": "New event dispatcher",
  "编辑事件分发器": "Edit event dispatcher",
  "编辑函数（局部变量）": "Edit function (local variables)",
  "局部变量不能和输入、输出参数重名": "A local variable cannot share a name with an input or output parameter",
  "调用": "Call",
  "绑定事件": "Bind event",
  "解除绑定": "Unbind",
  "委托": "Delegate",
  "局部变量无效": "Invalid local variables",
  "局部变量名无效或重复": "Invalid or duplicate local variable name",
  "事件分发器列表无效": "Invalid event dispatcher list",
  "事件分发器名称无效或重复": "Invalid or duplicate event dispatcher name",
  // model errors
  "请连接两个不同的节点": "Connect two different nodes",
  "请将输出端口连接到输入端口": "Connect an output pin to an input pin",
  "无法识别文件。请导入版本 2 的 .kardsbp.json 蓝图文件（旧版本文件不兼容）。":
    "File not recognised. Import a version 2 .kardsbp.json blueprint file (older files are not supported).",
  "项目名称或事件图列表无效": "Invalid project name or event graph list",
  "卡牌信息不完整": "Card info is incomplete",
  "事件图结构无效或超出大小限制": "Invalid event graph structure or size limit exceeded",
  "节点标识或位置无效": "Invalid node id or position",
  "节点定义缺失": "Node definition is missing",
  "端口定义无效": "Invalid pin definition",
  "节点参数值无效": "Invalid node parameter value",
  "连接标识重复": "Duplicate connection id",
  文件含有不存在或类型不匹配的连接:
    "The file has connections that do not exist or whose types do not match",
  "文件中有重复的输入或执行连接": "The file has duplicate input or execution connections",
};

// English-only labels shown in the interface; "zh" mode swaps them for Chinese.
const ZH_EXACT = {
  "BLUEPRINT EDITOR": "蓝图编辑器",
  "LOCAL WORKSPACE": "本地工作区",
  "SCHEMA v1.0": "格式 v1.0",
  "KARDS C++ · Unreal Blueprint": "KARDS C++ · 虚幻蓝图",
  "BLUEPRINT GRAPH": "蓝图事件图",
  NAVIGATOR: "导航",
  Name: "名称",
  Text: "文本",
  "CARD DATA": "卡牌数据",
  GRAPH: "事件图",
  NODE: "节点",
  DEFAULTS: "默认参数",
  NATIVE: "原生",
  "Event Graph · 卡牌事件图": "事件图 · 卡牌事件图",
  "Function · 函数": "函数",
  "Macro · 宏": "宏",
  Delegate: "委托",
  ASSET: "资源",
  BLUEPRINT: "蓝图",
  FUNCTION: "函数",
  MACRO: "宏",
};
const ZH_FACTIONS = {
  GERMANY: "德国", BRITAIN: "英国", JAPAN: "日本", SOVIET: "苏联", AMERICA: "美国",
  USA: "美国", FRANCE: "法国", ITALY: "意大利", POLAND: "波兰", FINLAND: "芬兰",
  ANZAC: "澳新军团", ALLIES: "盟军", NEUTRAL: "中立",
};
const ZH_TYPES = { INFANTRY: "步兵", TANK: "坦克", ORDER: "命令", COUNTER: "反制", BUILDING: "建筑" };
const ZH_PATTERNS = [
  [/^(\d+) SELECTED$/, "已选 $1 个"],
];

const PARAM_LABELS = { 输入参数: "Inputs", 输出参数: "Outputs", 参数: "Parameters", 局部变量: "Local variables" };
const PATTERNS = [
  [/^⚠ (\d+) 个节点的 Target 没有连接，而且它们不属于 (.+)，不能默认使用 Self：(.+)。$/s, "⚠ $1 node(s) have an unconnected Target and do not belong to $2, so they cannot default to Self: $3."],
  [/^这个函数属于 (.+)，不是本蓝图（(.+)）的成员，Target 必须连接$/, "This function belongs to $1, not to this blueprint ($2); its Target must be connected"],
  [/^已导出 PNG 图片（图较大，倍率降为 ([\d.]+)×）$/, "PNG image exported (large graph: scale reduced to $1×)"],
  [/^无法打开这张卡牌：(.+)$/s, (m) => `Cannot open this card: ${translateText(m[1], "en")}`],
  [/^已导入 (\d+) 张卡牌$/, "Imported $1 cards"],
  [/^已复制为 (.+)$/, "Duplicated as $1"],
  [/^已导出 (\d+) 张卡牌（ZIP）$/, "Exported $1 cards (ZIP)"],
  [/^导出失败：(.+)$/, "Export failed: $1"],
  [/^删除卡牌“(.*)”？这不能撤销，建议先导出。$/, "Delete card “$1”? This cannot be undone — export it first if you need it."],
  [/^当前 v(.+) → 最新 v(.+)$/, "Current v$1 → latest v$2"],
  [/^宏不能引用自身，嵌套也不能超过 (\d+) 层$/, "A macro cannot reference itself, and nesting cannot exceed $1 levels"],
  [/^删除“(.*)”？使用它的调用、绑定节点也会被一并删除，可使用撤销恢复。$/, "Delete “$1”? Call and bind nodes that use it are deleted too; you can undo this."],
  [/^(输入参数|输出参数|参数|局部变量)：(.+)$/s, (m) => `${PARAM_LABELS[m[1]]}: ${translateText(m[2], "en")}`],
  [/^资源名称无效：(.+)$/s, (m) => `Invalid asset name: ${translateText(m[1], "en")}`],
  [/^“(.+)”是保留名称，请换一个$/, "“$1” is a reserved name — choose another"],
  [/^名称“(.+)”已被使用$/, "The name “$1” is already in use"],
  [/^删除“(.*)”？调用它的节点也会被一并删除，可使用撤销恢复。$/, "Delete “$1”? Nodes that call it are deleted too; you can undo this."],
  [/^删除“(.*)”？使用它的获取\/设置节点也会被一并删除，可使用撤销恢复。$/, "Delete “$1”? Get/Set nodes that use it are deleted too; you can undo this."],
  [/^(\d+) 个节点 · (\d+) 条连接 · (\d+) 个动作节点没有执行输入$/, "$1 nodes · $2 wires · $3 action nodes have no exec input"],
  [/^(\d+) 个节点 · (\d+) 条连接$/, "$1 nodes · $2 wires"],
  [/^(\d+) 个节点 · KARDS \+ Unreal Blueprint$/, "$1 nodes · KARDS + Unreal Blueprint"],
  [/^(\d+) 函数 · (\d+) 类 · (\d+) UE 节点$/, "$1 functions · $2 classes · $3 UE nodes"],
  [/^(\d+) 个操作 \/ Actions$/, "$1 Actions"],
  [/^已选中 (\d+) 个节点$/, "$1 node(s) selected"],
  [/^已导入 (.+) · (\d+) 个图表$/, "Imported $1 · $2 graph(s)"],
  [/^刷新失败：(.+)。请检查网络后重试。$/, "Refresh failed: $1. Check your network and try again."],
  [/^尚未接入执行流：(.+)。可以保留它们作为设计草稿。$/, "Not connected to an execution flow yet: $1. You can keep them as design drafts."],
  [/^(.*)。请刷新页面，或检查 catalog\.json 是否和站点一起发布。$/, "$1. Reload the page, or check that catalog.json is published with the site."],
  [/^端口类型不匹配：(.+)$/, "Pin type mismatch: $1"],
  [/^删除“(.*)”及其所有节点？可使用撤销恢复。$/, "Delete “$1” and all its nodes? You can undo this."],
  [/^自定义节点最多支持 (\d+) 个端口$/, "A custom node supports at most $1 pins"],
  [/^(.+) 输入端口$/, "$1 input pin"],
  [/^(.+) 输出端口$/, "$1 output pin"],
  [/^(.*) · (.*) · 输入$/, "$1 · $2 · Input"],
  [/^(.*) · (.*) · 输出$/, "$1 · $2 · Output"],
  [/^([A-Z][A-Z ]*) · [一-鿿]+ \/ (.+)$/, "$1 / $2"],
  [/^([A-Z][A-Z ]*) · [一-鿿]+ · (.+)$/, "$1 · $2"],
  [/^([A-Z][A-Z ]*) · [一-鿿]+$/, "$1"],
];

const HAS_CJK = /[一-鿿]/;
const BILINGUAL = /^(.*?[一-鿿][^/]*?) \/ ([A-Za-z].*)$/;
// Leading/trailing decoration that is kept as-is around a translated phrase.
const FRAME = /^([\s↶↷⟳＋+↗▥⌕⌘·•✓☰]*)(.*?)([\s↗…]*)$/s;

const squash = (s) => s.replace(/\s+/g, " ").trim();

// "苏联 · Common" and "Studio 自定义节点 · …": translate each Chinese segment on its own.
function translateParts(body) {
  const lookup = (part) => EXACT[part];
  if (body.includes(" · ")) {
    const parts = body.split(" · ");
    const out = parts.map((p) => (HAS_CJK.test(p) ? lookup(p) : p));
    if (out.every((p) => p !== undefined)) return out.join(" · ");
  }
  const word = /^([^\u4e00-\u9fff\s]+) (.*[\u4e00-\u9fff].*)$/.exec(body);
  if (word && lookup(word[2]) !== undefined) return `${word[1]} ${lookup(word[2])}`;
  return null;
}

function translateEnglishLabel(core) {
  if (ZH_EXACT[core] !== undefined) return ZH_EXACT[core];
  const kind = /^([A-Z]+) \/ ([A-Z]+)$/.exec(core);
  if (kind && (ZH_FACTIONS[kind[1]] || ZH_TYPES[kind[2]]))
    return `${ZH_FACTIONS[kind[1]] ?? kind[1]} / ${ZH_TYPES[kind[2]] ?? kind[2]}`;
  for (const [re, out] of ZH_PATTERNS) if (re.test(core)) return core.replace(re, out);
  return null;
}

export function translateText(text, mode) {
  if (mode === "both" || !text) return text;
  if (mode === "zh" && !HAS_CJK.test(text)) {
    const label = translateEnglishLabel(squash(text));
    return label === null ? text : label;
  }
  if (!HAS_CJK.test(text)) return text;
  const lead = /^\s*/.exec(text)[0],
    trail = /\s*$/.exec(text)[0],
    core = squash(text);
  const frame = FRAME.exec(core),
    deco = frame ? frame[1] : "",
    body = frame ? frame[2] : core,
    tail = frame ? frame[3] : "";
  const keep = (value) => lead + deco + value + tail.trimEnd() + trail;
  if (mode === "zh") {
    const label = ZH_EXACT[core] ?? null;
    if (label !== null) return lead + label + trail;
    const m = BILINGUAL.exec(body);
    return m ? keep(m[1]) : text;
  }
  const exact = EXACT[core] ?? EXACT[body];
  const joined = exact === undefined ? translateParts(body) : null;
  if (joined !== null) return keep(joined);
  if (exact !== undefined) return EXACT[core] !== undefined ? lead + exact + trail : keep(exact);
  for (const [re, out] of PATTERNS) {
    const m = re.exec(body);
    if (m) return keep(typeof out === "function" ? out(m) : body.replace(re, out));
  }
  const m = BILINGUAL.exec(body);
  return m ? keep(m[2]) : text;
}

const ATTRS = ["title", "placeholder", "aria-label", "alt"];
const SKIP_TEXT = "script,style,textarea,.node,#nodes,#edges,#minimap,#lang-select";
const originalText = new WeakMap();
const originalAttr = new WeakMap();
let mode = "both";
let observer = null;

function skip(el) {
  return !!el?.closest?.(SKIP_TEXT);
}

function translateTextNode(node) {
  const el = node.parentElement;
  if (!el || skip(el)) return;
  const original = originalText.get(node) ?? node.nodeValue;
  const next = translateText(original, mode);
  if (next === node.nodeValue) return;
  if (!originalText.has(node)) originalText.set(node, node.nodeValue);
  node.nodeValue = next;
}

function translateAttrs(el) {
  if (el.id === "lang-select") return;
  for (const name of ATTRS) {
    const value = el.getAttribute?.(name);
    if (!value || !HAS_CJK.test(value)) continue;
    const saved = originalAttr.get(el) ?? {};
    const original = saved[name] ?? value;
    const next = translateText(original, mode);
    if (next === value) continue;
    saved[name] = original;
    originalAttr.set(el, saved);
    el.setAttribute(name, next);
  }
}

function translateTree(root) {
  if (root.nodeType === Node.TEXT_NODE) return translateTextNode(root);
  if (root.nodeType !== Node.ELEMENT_NODE) return;
  const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
  for (let n = walker.nextNode(); n; n = walker.nextNode()) translateTextNode(n);
  translateAttrs(root);
  root.querySelectorAll("*").forEach(translateAttrs);
}

function restoreTree(root) {
  const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
  for (let n = walker.nextNode(); n; n = walker.nextNode()) {
    const original = originalText.get(n);
    if (original !== undefined) {
      n.nodeValue = original;
      originalText.delete(n);
    }
  }
  root.querySelectorAll("*").forEach((el) => {
    const saved = originalAttr.get(el);
    if (!saved) return;
    for (const [name, value] of Object.entries(saved)) el.setAttribute(name, value);
    originalAttr.delete(el);
  });
}

// Translate a one-off string (confirm dialogs, etc.) for the current mode.
export function tr(text) {
  return translateText(text, mode);
}

export function setUiLanguage(next) {
  if (observer) {
    observer.disconnect();
    observer = null;
  }
  restoreTree(document.body);
  mode = next;
  if (mode === "both") return;
  translateTree(document.body);
  observer = new MutationObserver((records) => {
    observer.disconnect();
    for (const r of records) {
      if (r.type === "characterData") translateTextNode(r.target);
      else if (r.type === "attributes") translateAttrs(r.target);
      else r.addedNodes.forEach(translateTree);
    }
    observer.observe(document.body, OPTIONS);
  });
  observer.observe(document.body, OPTIONS);
}

const OPTIONS = {
  subtree: true,
  childList: true,
  characterData: true,
  attributes: true,
  attributeFilter: ATTRS,
};
