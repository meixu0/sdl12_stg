# 敌机子弹发射系统 (Emitter System)

## 概览

敌机子弹系统分为 **5 层**，从 JSON 数据定义到每帧子弹生成和运动：

```
JSON 关卡数据  →  EmitterConfig  →  Enemy::enemy_attack()  →  EnemyBulletManager  →  渲染
   (数据)          (配置结构体)        (每帧发射驱动)            (子弹池管理)         (SDL)
```

---

## 1. JSON 数据格式

以 `level/level1.json` 中 `zako_small` 行为为例：

```json
{
  "behaviors": {
    "zako_small": {
      "hp": 50,
      "move": { "type": "linear", "speedY": 120 },
      "attack": [
        {
          "time": 1.5,
          "interval": 0.3,
          "burst": 1,
          "burstInterval": 0.05,
          "pattern": {
            "type": "fan_aimed",
            "mainCnt": 6,
            "subCnt": 1,
            "spread": 0,
            "angleStep": 0.185,
            "speed": 100,
            "speedStep": 50,
            "spriteID": 0,
            "hitboxRadius": 4,
            "lifeTime": 6
          }
        }
      ]
    }
  }
}
```

### attack 数组字段

| JSON 字段 | EmitterConfig 成员 | 说明 |
|-----------|-------------------|------|
| `time` | `startDelay` | 敌人激活后延迟多少秒开始发射 |
| `interval` | `emitInterval` | 两次发射之间的间隔（秒），控制弹幕密度 |
| `burst` | `burstCount` | 每次爆发的子弹数。`1` = 单发，`>1` = 霰弹/连射 |
| `burstInterval` | `burstInterval` | burst 模式下弹与弹之间的间隔（秒） |
| `pattern.type` | `patternDesc.patternType` | 弹幕形状，见下方类型表 |
| `pattern.mainCnt` | `patternDesc.mainCnt` | 每圈/每排的子弹数量 |
| `pattern.subCnt` | `patternDesc.subCnt` | 层数（速度分几档，每层速度递减） |
| `pattern.spread` | `patternDesc.angleOffset` | 角度偏移（扇形偏移角，圆形起始角） |
| `pattern.angleStep` | `patternDesc.angleInterval` | 子弹之间的角度间隔（弧度） |
| `pattern.speed` | `patternDesc.speed1` | 基础弹速（px/s） |
| `pattern.speedStep` | `patternDesc.speed2` | 每层速度递减量（px/s） |
| `pattern.spriteID` | `patternDesc.spriteID` | 弹幕贴图 ID（0=红, 1=绿, 2=蓝, 3=黄） |
| `pattern.hitboxRadius` | `patternDesc.hitboxRadius` | 子弹判定半径（px） |
| `pattern.lifeTime` | `patternDesc.lifeTime` | 子弹存活时间（秒），超时会被回收 |

---

## 2. 核心数据结构

### EmitterConfig（发射器配置）

```cpp
// EnemyBulletManager.h
struct EmitterConfig {
    float emitInterval;   // 发射间隔（秒）
    int   burstCount;     // 每次爆发的子弹数
    float burstInterval;  // 连射时弹与弹的间隔（秒）
    float startDelay;     // 开始延迟（秒）
    EnemyBulletPatternDesc patternDesc;  // 弹幕形状参数
};
```

### EnemyBulletPatternDesc（弹幕形状）

```cpp
struct EnemyBulletPatternDesc {
    int   patternType;     // 弹幕类型 (0–7)
    int   mainCnt;         // 一圈/一排的子弹数
    int   subCnt;          // 层数
    float angleOffset;     // 角度偏移
    float angleInterval;   // 角度间隔
    float speed1;          // 基础弹速
    float speed2;          // 每层速度递减量
    int   spriteID;        // 贴图 ID
    float hitboxRadius;    // 判定半径
    float lifeTime;        // 存活时间
    int   spawnEffect;     // （预留）生成特效
    int   soundEffect;     // （预留）音效
    int   reboundEffect;   // （预留）反弹特效
};
```

### EmitterRuntime（发射器运行时状态）

```cpp
// Enemy.h (Enemy 内部 struct)
struct EmitterRuntime {
    float timer;           // 累计计时器（秒）
    int   burstRemaining;  // 当前 burst 剩余子弹数
    int   cycleCount;      // 循环计数
};
```

### Enemy 持有的成员

```cpp
class Enemy {
    ...
public:
    std::vector<EmitterConfig>  emitterConfig;   // 发射器配置（从 JSON 复制）
    std::vector<EmitterRuntime> emitterRuntime;  // 运行时状态（需与 config 同 size）
    EnemyBulletManager* bulletManager;           // 子弹池指针（外部注入）
};
```

---

## 3. 弹幕类型一览

| JSON type | 枚举值 | 常量 | 行为 |
|-----------|--------|------|------|
| `fan_aimed` | 0 | `PAT_FAN_AIMED` | 扇形自机狙：以玩家方向为中心，向两侧交替展开 |
| `fan` | 1 | `PAT_FAN` | 固定方向扇形：不跟踪玩家 |
| `aimed` | 2 | `PAT_CIRCLE_AIMED` | 圆形自机狙：环绕玩家方向均匀分布 |
| `ring` | 3 | `PAT_CIRCLE` | 固定方向圆形：不跟踪玩家 |
| `ring_aimed` | 4 | `PAT_RANDOM_ANGLE` | 环形自机狙：在自机狙基础上偏移 0.5 步避免对称 |
| `spiral` | 5 | `PAT_RANDOM_SPEED` | 随机角度散射 |
| `spread` | 6 | `PAT_RANDOM_BOTH` | 随机速度散射 |
| `ring_aimed_variable` | 7 | `PAT_RING_AIMED` | 随机角度+速度散射 |

### 各类型角度计算公式

`aimAngle` = 从子弹原点到玩家的角度。`PI` = π。

| 类型 | 角度公式 | 速度 |
|------|---------|------|
| `fan_aimed` (0) | `aimAngle + offset + (±1) * (i/2 + i%2) * interval` | `speed1 - (speed1-speed2) * layer/(subCnt-1)` |
| `fan` (1) | `offset + (±1) * (i/2 + i%2) * interval` | 同上 |
| `aimed` (2) | `aimAngle + offset + i * 2π/mainCnt + layer * interval` | 同上 |
| `ring` (3) | `offset + i * 2π/mainCnt + layer * interval` | 同上 |
| `ring_aimed` (4) | `aimAngle + offset + (i+0.5) * 2π/mainCnt + layer * interval` | 同上 |
| `spiral` (5) | `offset + rand() * (interval - offset)` | 同上 |
| `spread` (6) | `offset + i * 2π/mainCnt` | `speed2 + rand() * (speed2 - speed1)` |
| 混合 (7) | `offset + rand() * (interval - offset)` | `speed2 + rand() * (speed2 - speed1)` |

---

## 4. 发射驱动（Enemy::enemy_attack）

每帧由 `LevelManager::attack_all_enemies(dt)` 调用，核心逻辑：

```
enemy_attack(dt):
│
├─ 条件检查: isActive && bulletManager != NULL
│
├─ [Boss] Phase Sequencer（见第 5 节）
│
├─ 遍历每个发射器 i:
│   │
│   ├─ rt.timer += dt
│   ├─ if (rt.timer < ec.startDelay) → 跳过（等待开始延迟）
│   │
│   ├─ burstCount == 1（普通单发）:
│   │   if (activeTime >= ec.emitInterval):
│   │       bulletManager->spawn_pattern(ec.patternDesc, x, y, px, py, ...)
│   │       rt.timer = ec.startDelay  // 重置计时器
│   │
│   └─ burstCount > 1（连射/霰弹）:
│       ├─ if (burstRemaining == 0 && interval 到了):
│       │    burstRemaining = burstMax  // 开启新一轮连射
│       │
│       └─ if (activeTime >= burstInterval && burstRemaining > 0):
│            spawn_pattern(...)
│            burstRemaining--
│            rt.timer = ec.startDelay
│
└─ 返回
```

### 时序示意

```
startDelay=1.5s    emitInterval=0.3s    emitInterval=0.3s
     |                   |                    |
     v                   v                    v
激活  .....(1.5s等待)....|--发射!--(0.3s)--|--发射!--(0.3s)--|--发射!...
                        |                                    |
                        burstCount=1: 单发                   burstCount=1
                        
                        burstCount=5, burstInterval=0.05s:
                        |--b1--b2--b3--b4--b5--| (0.3s后) |--b1--b2--...
                        |<--- 0.2s 连射 --->|
```

---

## 5. Boss Phase Sequencer

Boss 额外支持按时间自动切换攻击阶段。由 JSON 中的 `phases` 数组定义：

```json
"phases": [
  {
    "duration": 15.0,
    "move_after": [192, 140],
    "move_duration": 120,
    "move_easing": 4,
    "patterns": [ ... ]    // 此阶段的 emitter 配置
  },
  {
    "duration": 10.0,
    "patterns": [ ... ]    // 下一阶段的配置
  }
]
```

### advance_phase() 逻辑（Enemy.cpp:515）

```
advance_phase():
  currentPhase_ = (currentPhase_ + 1) % bossPhases_.size()  // 循环
  phaseTimer_ = 0
  // 如果定义了 move_after，执行 position_interp 移动
  if (moveDuration > 0): start_position_interp(duration, easing, targetX, targetY)
  // 替换 emitterConfig
  emitterConfig = bossPhases_[currentPhase_].patterns
  emitterRuntime.resize(emitterConfig.size())
```

---

## 6. 子弹池管理（EnemyBulletManager）

### 对象池

```cpp
static const int POOL_SIZE = 640;    // 最大同时存在 640 颗敌弹
Bullet bullets[POOL_SIZE];           // 连续数组，用 state 标记空闲/活跃
int nextBulletIndex;                 // 轮转索引起点
```

### Bullet 结构

```cpp
struct Bullet {
    float x, y;            // 位置
    float speedX, speedY;  // 速度（px/s）
    int   state;           // SLEEPING / ALIVE
    float lifeTime;        // 剩余存活时间
    float hitboxRadius;    // 判定半径
    int   color;           // 贴图 ID
    float acceleration;    // 加速度（px/s²）
    float angularVelocity; // 角速度（rad/s）
    int   enemyType;       // 发射者类型
    int   enemyID;         // 发射者 ID
};
```

### spawn_bullet 分配策略

轮转搜索：从 `nextBulletIndex` 开始找第一个 `SLEEPING` 槽位写入，然后 `nextBulletIndex` 前进一位。

### update 逻辑（每帧）

```
for each bullet in pool:
    if (state != ALIVE) continue
    // 加速度 & 角速度
    if (accel != 0 || angularVelocity != 0):
        newSpeed = curSpeed + accel * dt
        newAngle = curAngle + angularVelocity * dt
    // 移动
    x += speedX * dt
    y += speedY * dt
    lifeTime -= dt
    // 超出屏幕范围 → SLEEPING（回收到池）
    if (outOfBounds) state = SLEEPING
```

---

## 7. 每帧完整调用链

```
Game::update_game(dt)
│
├─ levelManager->attack_all_enemies(dt)         // ① 敌人攻击
│   └─ for each enemy in pool:
│       enemy->enemy_attack(dt)
│         ├─ [Boss] phase sequencer → advance_phase()
│         └─ for each emitter:
│              bulletManager->spawn_pattern(desc, x, y, px, py, type, id)
│                └─ for layer in subCnt:
│                     for bullet in mainCnt:
│                       spawn_bullet(x, y, angle, speed, sprite, ...)
│                         └─ 写入 bullets[640] 的空闲槽位
│
├─ enemyBulletManager_.update(dt)               // ② 子弹运动
│   └─ for each ALIVE bullet:
│        x += speedX*dt, y += speedY*dt
│        lifeTime -= dt
│        outOfBounds → SLEEPING
│
└─ enemyBulletManager_.render()                 // ③ 子弹渲染
    └─ for each ALIVE bullet:
         apply_surface(x, y, sprite, screen)
```

---

## 8. 特殊操作

### Spellcard 切换时（LevelManager::start_spellcard_phase）

```cpp
// 用 spellcard 的 patterns 替换 boss 的 emitterConfig
boss->emitterConfig = sc->patterns;
boss->emitterRuntime.resize(boss->emitterConfig.size());
// 重置所有运行时状态
for each emitterRuntime[i]:
    timer = 0; burstRemaining = 0; cycleCount = 0;
```

### 子弹清除

- `despawn_all_for_spellcard()`：将所有 ALIVE 子弹置为 SLEEPING（符卡开始时/结束时清除弹幕）
- `convert_all_to_p_items(ItemManager*)`：将所有 ALIVE 敌弹转换为 P 道具（击败 Boss/道中 Boss 时用）

---

## 9. 难度覆盖

JSON 中支持 `difficulty` 对象，按难度覆盖 attack 参数：

```json
"difficulty": {
  "hard": {
    "attack": {
      "0": { "interval": 0.2, "burst": 3 },    // 覆盖第 0 个 emitter
      "pattern": { "speed": 120 }               // 不支持直接覆写，需通过 attack key
    }
  }
}
```

在 `apply_behavior_difficulty()` 中实现，遍历 `difficulty.{currentDifficulty}.attack` 覆盖对应 index 的 `EmitterConfig` 字段。
