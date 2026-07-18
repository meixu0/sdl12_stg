# SDL12_STG 
用了非常古老的SDL1.2图形库和C++98语法写的仿东方STG
---
# 1.编译
Windows在mingw中安装安装
```
mingw-w64-x86_64-gcc
mingw-w64-x86_64-make
mingw-w64-x86_64-SDL
mingw-w64-x86_64-SDL_image
mingw-w64-x86_64-SDL_ttf
mingw-w64-x86_64-SDL_mixer
mingw-w64-x86_64-physfs
```
然后在项目根目录`make`

Debian/Ubuntu
```
sudo apt-get update
sudo apt-get install -y \
build-essential \
libsdl1.2-dev \
libsdl-image1.2-dev \
libsdl-ttf2.0-dev \
libsdl-mixer1.2-dev \
libphysfs-dev
```
然后在项目根目录`make`
# 2.json配置

关卡文件位于 `level/` 目录，命名规则：`level{N}.json` (道中+Boss) / `level{N}bs.json` (纯Boss)。

## 2.1 顶层结构

```json
{
  "behaviors": { ... },
  "timeline": [ ... ]
}
```

- `behaviors`: 定义所有敌机行为模板（键名可任意，后续通过键名引用）
- `timeline`: 时间线，控制敌机出场顺序

## 2.2 行为定义

```json
"行为名": {
  "hp": 25,
  "hitbox": [32, 32],
  "score": 1000,
  "halfLife": 0.8,
  "lifeTime": 12.0,
  "move": { ... },
  "attack": [ ... ],
  "death": { "itemDrop": "random" },
  "flags": ["midboss"],
  "difficulty": { ... },
  "move_bounds": [minX, minY, maxX, maxY]
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `hp` | int | 基础血量 (normal难度) |
| `hitbox` | [w, h] | 碰撞判定框 (像素) |
| `score` | int | 击破得分 |
| `halfLife` | float | 半血触发符卡/阶段变化 (0=不触发) |
| `lifeTime` | float | 存活时间 (秒)，倒计时结束后自动消失 |
| `move` | object | 移动模式 |
| `attack` | array | 攻击/弹幕发射器列表 |
| `death` | object | 死亡掉落，`itemDrop`: `"random"`/`"power_small"`/`"power_big"`/`"full_power"`/`"bomb"`/`"life"` |
| `flags` | array | 特殊标记，`"midboss"` 表示道中Boss |
| `difficulty` | object | 难度覆盖，见 §2.5 |
| `move_bounds` | [minX,minY,maxX,maxY] | 限制移动范围 |

## 2.3 移动模式 (move)

### linear — 直线
```json
{ "type": "linear", "speedX": 0, "speedY": 120 }
```

### sine — 正弦波
```json
{ "type": "sine", "speedX": 0, "speedY": 120, "vertAmp": 40, "vertPeriod": 2.0, "horizAmp": 0, "horizPeriod": 1.0 }
```

### bezier — 贝塞尔曲线
```json
{ "type": "bezier", "p1x": 0, "p1y": -100, "p2x": 100, "p2y": 0, "endX": 200, "endY": 200, "duration": 3.0 }
```

### stop_and_go — 移动到目标后停止
```json
{ "type": "stop_and_go", "speed": 120, "target": [192, 128] }
```

### homing — 追踪玩家
```json
{ "type": "homing", "speed": 120, "angularVelocity": 3, "minDist": 80 }
```

### interception — 拦截预判
```json
{ "type": "interception", "speed": 150 }
```

### position_interp — 位置插值 (缓动)
```json
{ "type": "position_interp", "duration": 60, "easing": 4, "target": [192, 128] }
```
`duration` 单位是帧 (60帧=1秒)。`easing`: 0=线性, 4=减速。

## 2.4 弹幕发射器 (attack/emitter)

```json
{
  "time": 0.0,
  "interval": 1.2,
  "burst": 1,
  "burstInterval": 0.05,
  "pattern": { ... }
}
```

| 字段 | 说明 |
|------|------|
| `time` | 首次发射延迟 (秒) |
| `interval` | 发射间隔 (秒) |
| `burst` | 每次发射的连发数 |
| `burstInterval` | 连发间隔 (秒) |
| `pattern` | 弹幕模式，见 §2.4.1 |

### 2.4.1 弹幕模式 (pattern)

```json
{
  "type": "fan_aimed",
  "mainCnt": 6,
  "subCnt": 1,
  "spread": 0,
  "angleStep": 0.15,
  "speed": 120,
  "speedStep": 50,
  "spriteID": 0,
  "hitboxRadius": 4,
  "lifeTime": 6,
  "angularVelocity": 0,
  "isSplit": false
}
```

| 字段 | 说明 |
|------|------|
| `type` | 弹型 (见下表) |
| `mainCnt` | 主弹数 (一圈或一排的子弹数) |
| `subCnt` | 层数 (速度分几档，>1 时弹幕由多颗子弹连成线) |
| `spread` | 角度偏移/波幅 (不同弹型含义不同) |
| `angleStep` | 角度间隔/波长系数 |
| `speed` | 速度 |
| `speedStep` | 速度递变 (subCnt>1 时外层速度) |
| `spriteID` | 子弹精灵索引 |
| `hitboxRadius` | 碰撞半径 |
| `lifeTime` | 子弹存活时间 (秒) |
| `angularVelocity` | 角速度/旋转速度 (仅 spiral/sinewave 使用) |
| `isSplit` | 是否启用分裂弹 (每90帧分裂一次) |

### 弹型一览

| type | 枚举 | 说明 |
|------|------|------|
| `fan_aimed` | 0 | 扇形自机狙 (交替向两侧展开，朝向玩家) |
| `fan` | 1 | 固定扇形 |
| `aimed` | 2 | 圆形自机狙 |
| `ring` | 3 | 固定圆形 |
| `ring_aimed` | 4 | 环形自机狙 |
| `spiral` | 5 | 螺旋 (角速度旋转，圆形散射) |
| `spread` | 6 | 随机散射 (速度随机) |
| `ring_aimed_variable` | 7 | 可变环形自机狙 |

## 2.5 难度覆盖 (difficulty)

```json
"difficulty": {
  "easy": { "hp": 15 },
  "hard": { "hp": 30 },
  "lunatic": { "hp": 45 },
  "extra": { "hp": 50 }
}
```

支持覆盖的字段: `hp`, `score`, `halfLife`, `lifeTime`, `hitbox`, `move` (所有移动参数), `attack` (按索引覆盖单条发射器参数)。

## 2.6 时间线

```json
"timeline": [
  { "delta": 0.02, "cmd": "spawn", "behavior": "zako_initial", "pos": [192, -16] },
  { "delta": 3.0,  "cmd": "wave",  "behavior": "zako_small",  "pos": [40, -16], "count": 4, "dx": 30, "gap": 0.5 },
  { "delta": 100,  "cmd": "spawn", "behavior": "boss_entry", "pos": [192, -16] }
]
```

| 命令 | 说明 |
|------|------|
| `spawn` | 在 `pos` 位置生成一个 `behavior` 敌机 |
| `wave` | 生成一排 `count` 个敌机，间距 `dx`，间隔 `gap` 秒 |

`delta` 是与上一条命令的时间间隔 (秒)。最后一个 `spawn` 通常放 `boss_entry`(sentinel) 且 `delta` 设为 100+，等待前面敌机全灭后自动触发 Boss 战。

## 2.7 Phase 序列

Boss 行为中可加 `phases` 数组实现 ECL 风格的多阶段攻击循环：

```json
"phases": [
  {
    "duration": 8.0,
    "move_duration": 60,
    "move_easing": 4,
    "move_after": [256, 200],
    "patterns": [ ... ]
  },
  {
    "duration": 6.0,
    "patterns": [ ... ]
  }
]
```

| 字段 | 说明 |
|------|------|
| `duration` | 本阶段持续秒数，结束后自动切换到下一阶段 (循环) |
| `move_duration` | 阶段开始时移动动画帧数 (0=不移动) |
| `move_easing` | 缓动类型: 0=线性, 4=减速 |
| `move_after` | 阶段开始时移动目标 [x, y] |
| `patterns` | 本阶段使用的发射器列表 |

## 2.8 符卡

详见 `level/sc/stage{N}.json`:

```json
{
  "spellcards": [
    {
      "id": 0,
      "name": "冰符「アイシクルフォール」",
      "hp": 2000,
      "timeout": 45,
      "patterns": [ ... ],
      "difficulty": { ... }
    }
  ]
}
```

`patterns` 格式与 `attack` 相同。"difficulty" 支持覆盖 `hp` 和 `timeout`。
---
# 3.0
游戏根目录下如果有youmu文件则不开启自机判定点