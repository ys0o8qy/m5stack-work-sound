# 项目协作说明

开始任务时先阅读 [docs/PRODUCT_BRIEF.md](docs/PRODUCT_BRIEF.md)，再结合用户本次指令执行。
它记录已确认的产品目标、交互规则和偏好；用户最新明确指令优先。

## 产品方向

- Little Wonders 是给约 **2 岁孩子**玩的声音与动画键盘玩具。即时反馈和自由探索是核心，英文朗读是偶尔触发的小惊喜。
- 声音需要 **宁静、柔和、适合小孩**：低一些的音高、轻音量、平滑起音与消退；避免尖锐高音、夸张弹簧声、密集叠音。
- 中央表现刚按下的字母/特殊键；底部显示本轮输入。同一个键固定对应颜色、动画类型和音色，粒子运动可以有小变化。
- 松开全部按键后默认等待 **3 秒**（可配置），仅在整轮输入精确匹配内置英文词库时朗读一次。不要改成 5 秒或 1.2 秒，也不要逐键朗读。
- 输入错误或乱按不惩罚、不报错；无关输入不强行朗读。新按键应立即打断朗读并恢复玩耍。
- 保持离线使用、无需电脑和 microSD。目标设备是 **Cardputer ADV**，使用支持 ADV 的驱动。

## 执行习惯

- 用户明确要求“先分析、确认后执行”时，先澄清并确认；已确认的范围内直接完成实现、适当验证及授权的刷写，不重复请求相同确认。
- 变更声音时同时检查实际音频素材、播放音量和设备持久化配置。修改代码默认值不会覆盖 NVS 中的设置。
- 使用 `scripts/pio.sh` 调用项目内开发环境。验证应针对实际变更；报告时区分编译、自动真机测试、屏幕帧检查与用户听感反馈。
- 自动真机测试前提醒用户暂时不要按键。USB 注入测试不等于实体按键或听感已获人工确认。
- 保留 `artifacts/backups/original-flash.bin` 及校验和，不覆盖首次原固件备份。
- 用户确认产品方向或规则发生变化时，同步更新需求说明与相关使用文档，避免保留互相冲突的“当前要求”。

<!-- codebase-memory-mcp:start -->
## Codebase Knowledge Graph (codebase-memory-mcp)

ALWAYS prefer MCP graph tools over grep/glob/file-search for code discovery.
Run `index_repository` first if this project is not indexed.

Priority order:
1. `search_graph` — find functions, classes, routes, variables by pattern.
2. `trace_path` — trace callers and callees.
3. `get_code_snippet` — read a specific symbol after finding its qualified name.
4. `query_graph` — complex graph queries.
5. `get_architecture` — high-level project overview.

Use `search_code` for graph-augmented text search. Fall back to grep/glob for string
literals, error messages, config values, non-code files, or insufficient graph results.
<!-- codebase-memory-mcp:end -->
