// Keep source identifiers verbatim beside the Chinese display label.
const words = Object.fromEntries(
  `
On:当 Get:获取 Card:卡牌 Side:阵营 Other:其他 Is:是否 From:从 To:到 Hand:手牌 By:按 Cards:卡牌 Add:添加 Remove:移除 Campaign:战役 Set:设置 Attack:攻击 Json:JSON Has:拥有 Deck:牌组 After:之后 In:在 Array:数组 Kredit:指挥点 Defense:防御 Effect:效果 Turn:回合 Damage:伤害 Unit:单位 Before:之前 And:且 Location:位置 Board:战场 Custom:自定义 Played:打出 Cost:费用 Give:赋予 String:字符串 Static:静态 Name:名称 Gameplay:游戏 Of:的 Frontline:前线 Make:构造 Move:移动 Kredits:指挥点 Change:改变 Client:客户端 Conv:转换 Bool:布尔 Int:整数 Buff:增益 Destroyed:被摧毁 This:此 Multiple:多个 For:为 Match:对局 Value:值 Type:类型 Trigger:触发 Changed:改变 Gain:获得 Spawn:生成 All:全部 Playing:打出中 Web:网络 Socket:套接字 Play:打出 Reset:重置 Operation:行动 Mobilize:动员 End:结束 Active:激活 Opponent:对手 Player:玩家 Battle:战斗 Encrypt:加密 Smokescreen:烟幕 Deal:造成 Blitz:闪击 Veteran:老兵 Left:左侧 Total:总计 Object:对象 Discard:弃牌 Start:开始 Moved:已移动 Triggered:已触发 Slot:槽位 Armor:护甲 Guard:守护 Tag:标签 Fury:奋战 Ambush:伏击 Decrypt:解密 Can:可以 The:该 Text:文本 Most:最 Units:单位 Sort:排序 Input:输入 Field:字段 Log:日志 Choose:选择 Encryption:加密 Combat:战斗 Pincer:钳击 Covert:潜伏 Lose:失去 Leave:离开 Or:或 Fully:完全 Target:目标 Intel:情报 Create:创建 Tank:坦克 Ability:能力 Countdown:倒计时 Heavy:重型 Top:顶部 Random:随机 Support:支援 Line:线 Enum:枚举 Compare:比较 Action:动作 Reconnect:重连 Integer:整数 Time:时间 Event:事件 Send:发送 Message:消息 Open:打开 Ping:心跳 One:一个 Extra:额外 Gotcha:反制 Data:数据 Key:键 Suppressed:被压制 Game:游戏 Receive:受到 Pinned:被压制 Spawned:已生成 Retreat:撤退 Owner:拥有者 Spent:已消耗 Repaired:已修复 Attacks:攻击 Deployment:部署 Apply:应用 Counter:计数器 Forecast:预备 Located:所在 Tags:标签 Cant:不可 Be:成为 Upgrade:升级 Bond:羁绊 Visuals:视觉 Amount:数量 Shock:震慑 Immune:免疫 Alpine:山地 Nation:国家 As:作为 Does:是否 Tampering:篡改 Next:下一个 Right:右侧 Control:控制 Enemy:敌方 Show:显示 Notification:通知 Draw:抽牌 Salvage:回收 With:含 Owners:拥有者 Count:数量 Number:数量 Last:上一个 Session:会话 Backhandler:返回处理 Bindings:绑定 Mulligan:调度 Float:浮点 Config:配置 Raw:原始 Retry:重试 Do:执行 Which:选择 Update:更新 Triggers:触发器 Should:应当 Image:图像 Table:表 Row:行 Attributes:属性 Survived:存活 Received:已收到 Revealed:已揭示 Enter:进入 Drawn:已抽取 Calc:计算 Becoming:成为 Destruction:亡语 Fields:字段 Operaction:行动 Same:相同 Exile:放逐 Damaged:受伤 Buffed:已增强 Anti:反 Air:空中 Initialize:初始化 Temp:临时 Filter:筛选 Exist:存在 Attribute:属性 Copy:复制 Targetted:被选为目标 Op:操作 Texture:纹理 Main:主要 Strategy:策略 Was:曾 When:当 Star:星级 Suppress:压制 Battlefield:战场 Objective:目标 Seen:可见 Select:选择 Pin:压制 Restriction:限制 Directly:直接 Max:最大 ID:ID Force:强制 Destroy:摧毁 Editor:编辑器 State:状态 Axis:轴 Binding:绑定 Item:项 Sub:子 Resync:重新同步 Info:信息 Evaluating:求值 Actions:动作 Try:尝试 Code:编码 Asset:资源 Typed:带类型 Null:空 Server:服务器 Local:本地 JSON:JSON Date:日期 Error:错误 Pass:通行证 Connected:已连接 Debug:调试 Failure:失败 Off:关闭 Close:关闭 Highlight:高亮 Instigator:发起者 Succesful:成功 Removed:已移除 Applied:已应用 Unpinned:解除压制 Salvaged:已回收 Forecasted:已预备 Discarded:已弃牌 Developed:已开发 Created:已创建 Alter:变体 Converted:已转换 Switch:切换 Abilities:能力 Selected:已选择 Ownership:归属 Fatigue:疲劳 Shuffled:已洗牌 Changes:变化 Upgrades:升级 Measure:测量 Modify:修改 Dealt:已造成 Attempted:尝试 Stopped:已停止 Buffs:增益 Weather:天气 Valid:有效 Unrevealed:未揭示 Destroyer:歼击车 Owned:拥有 Order:指令 Infantry:步兵 Gun:火炮 Ground:地面 Fighter:战斗机 Bomber:轰炸机 Artillery:炮兵 Init:初始化 Movement:移动 Attacked:被攻击 Runtime:运行时 Passive:被动 Opposite:相反 Keywords:关键词 Children:子项 Strings:字符串 Scry:占卜 Simulate:模拟 Title:标题 Increase:增加 Block:阻挡 Being:正在 Available:可用 Unique:唯一 Material:材质 Status:状态 Take:夺取 Steal:窃取 Shuffle:洗牌 Tutorial:教程 Cipher:密码 Reveal:揭示 Operations:行动 Report:报告 Release:释放 Range:范围 Stream:流 Persist:持久化 Notify:通知 Front:前方 Fight:战斗 Clear:清空 Using:使用 Navy:海军 Full:全部 Reserved:预留 Increment:递增 Keyword:关键词 Lost:失去 Targeted:被选中 Starting:起始 Limit:限制 Byside:按阵营 Functions:函数 Notifier:通知器 Ally:友方 Gotchas:反制 Ordered:有序 Adjacent:相邻 More:更多 Heal:治疗 Resolve:结算 Queue:队列 Specific:指定 Decrement:递减 Convert:转换 Limiter:限制器 Turns:回合 Append:追加 Adjust:调整 Position:位置 Tax:税 Until:直到 UGC:UGC Bar:条 Widget:控件 Console:控制台 Simulating:模拟 Store:商店 Branch:分支 Platform:平台 Mobile:移动端 Faction:阵营 Disconnect:断开 Setup:配置 Sides:阵营 Possible:可能 HQ:总部 Poll:轮询 Notifications:通知 Items:项目 Equip:装备 Role:角色 Connect:连接 Unreal:虚幻 Users:用户 Username:用户名 Context:上下文 Variable:变量 Fatal:严重 Heartbeat:心跳 Current:当前 Sent:已发送 Draft:竞技场 Controller:控制器 AI:AI Reconecting:重连 Spectate:观战 Finished:已结束 Timed:定时 Out:输出 Waiting:等待 Variables:变量 Kards:KARDS Manager:管理器 Kard:卡牌 Map:映射 Capture:采集 Performance:性能 Analytics:分析 Cardback:卡背 Load:加载 Settings:设置 Return:返回 Condition:条件 True:为真 False:为假 Then:然后 Exec:执行 Success:成功 Failed:失败 Result:结果 Self:自身 Index:索引 First:首个 Loop:循环 Body:体 Completed:已完成 Duration:时长 Reset:重置 StartClosed:初始关闭 Break:中断 Selection:选择 Default:默认 Screen:屏幕 Print:打印 A:A B:B X:X Y:Y Z:Z Min:最小 Max:最大 FVector:向量 FName:名称 FText:文本 FString:字符串 UObject:对象 AttackBuff:攻击增益 Value:值
`
    .trim()
    .split(/\s+/)
    .map((s) => {
      const at = s.indexOf(":");
      return [s.slice(0, at).toLowerCase(), s.slice(at + 1)];
    }),
);
const special = {
  "Cast Failed": "转换失败",
  "Return Value": "返回值",
  targetCard: "目标卡牌",
  target: "目标",
  card: "卡牌",
  CardID: "卡牌 ID",
  instigatorID: "发起者 ID",
  giverID: "赋予者 ID",
  qqq: "结果",
  exec: "执行",
  amount: "数量",
  return: "返回值",
  Boolean: "布尔变量",
  Integer: "整数变量",
  String: "字符串变量",
  BaseCardObject: "基础卡牌对象",
  CardFunctionsStub: "卡牌函数",
  OnPlayedFromHand: "从手牌打出",
  OnStartOfTurn: "回合开始",
  OnStartOfGame: "对局开始",
  OnReceiveDamage: "受到伤害",
  GiveBlitz: "赋予闪击",
  GiveGuard: "赋予守护",
  GiveFury: "赋予奋战",
  GiveSmokescreen: "赋予烟幕",
  Branch: "分支",
  Sequence: "序列",
  IsValid: "是否有效",
  ObjectReference: "对象引用",
  DrawCardsFromDeckBySide: "按阵营从牌组抽牌",
  DamageCard: "对卡牌造成伤害",
  TakeControlOfEnemyUnit: "夺取敌方单位控制权",
};
export function translate(identifier) {
  if (special[identifier]) return special[identifier];
  const tokens = String(identifier).match(
    /[A-Z]+(?=[A-Z][a-z]|\b)|[A-Z]?[a-z]+|[0-9]+|\*/g,
  ) || [String(identifier)];
  return tokens.map((t) => words[t.toLowerCase()] ?? t).join("");
}
export function nodeKind(d) {
  return (
    d.kind ||
    (d.event || d.category === "events"
      ? "event"
      : d.category === "flow"
        ? "flow"
        : d.category === "variables"
          ? "variable"
          : d.pure
            ? "pure"
            : "function")
  );
}
export function nodeEnglish(d) {
  return nodeKind(d) === "event"
    ? `Event ${d.name.replace(/^Event\s+/, "")}`
    : d.english || d.name;
}
export function nodeChinese(d) {
  const kind = nodeKind(d);
  if (kind === "cast")
    return "转换为 " + translate(d.targetClass.replace(/^[UA](?=[A-Z])/, ""));
  if (d.property)
    return (kind === "setter" ? "设置 " : "获取 ") + translate(d.property);
  const label = d.label || translate(d.name);
  return kind === "event" ? "事件 · " + label : label;
}
export const kindLabels = {
  event: "EVENT · 事件",
  function: "FUNCTION · 函数",
  pure: "PURE · 纯函数",
  flow: "FLOW CONTROL · 流程",
  variable: "VARIABLE · 变量",
  cast: "CAST · 类型转换",
  conversion: "CONVERSION · 转换",
  getter: "GET · 读取属性",
  setter: "SET · 写入属性",
  functionEntry: "FUNCTION ENTRY · 函数入口",
  functionResult: "FUNCTION RESULT · 函数返回",
  macroEntry: "MACRO INPUT · 宏输入",
  macroExit: "MACRO OUTPUT · 宏输出",
  customEvent: "CUSTOM EVENT · 自定义事件",
  call: "CALL · 调用",
  macro: "MACRO · 宏",
};
