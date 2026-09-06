# Little Wonders · 小小奇趣键盘

给约两岁孩子玩的 M5Stack Cardputer **ADV** 离线声音动画玩具。

后续开发先读 [`AGENTS.md`](AGENTS.md) 和 [产品目标与偏好](docs/PRODUCT_BRIEF.md)。

按任意键：中央字母弹跳，周围冒出泡泡、弹跳球、小花或星星，同时响起有趣的短音。
同一个键始终对应相同的颜色、动画类型和音色，只有粒子的方向略有变化。
底部输入栏展示本轮累计输入，太长时显示最后 12 个字符；退格时末尾字符缩小消失。
松开全部按键 **3 秒** 后，若这一轮完整输入是词库里的英文单词，就显示对应图案并朗读。
乱按没有错误提示。再次按键立即开始新一轮，并中断正在播放的朗读。

## 直接玩

1. 打开设备侧面的电源开关，等待出现绿色小伙伴。
2. 任意按键即可玩声音与动画。长按只触发一次；松开再按才再次触发。
3. 测试朗读时，先静置 3 秒结束上一轮，再依次按 `c`、`a`、`t`，松手等待 3 秒。
4. 英文 `cat` 和小猫会出现，并播放一次 “cat”。新按键随时打断朗读。

不需要 Wi-Fi、账号、microSD 卡或连接电脑。USB 仅用于开发、调试和供电。
拔掉 USB 使用时，侧面电源开关需要保持 ON。

## 家长设置

长按机身 **G0 / BtnA** 按钮约 **2 秒**，进入设置。

| 操作 | 按键 |
| --- | --- |
| 选择设置项 | `W` / `S` |
| 减少 / 增加 | `A` / `D` |
| 保存并返回玩具 | `Enter` |
| 取消更改 | 左上角反引号键，或再次长按 G0 |

可调整等待时间（1–15 秒，步长 1 秒）、音量和屏幕亮度。保存后断电仍保留。
默认等待 3 秒，音量 64/255，亮度 130/255；设置页面显示相对于硬件刻度的百分比。
音量调整上限是 160/255。

首次启动的默认值集中在 [`include/toy_config.h`](include/toy_config.h)。
已经保存过的设备设置优先于代码默认值；修改代码默认值不会覆盖设备已有设置。
也可以从 USB 设置毫秒级等待时间，例如 6500 毫秒（同样会保存）：

```sh
./.venv/bin/python scripts/device.py 'timeout 6500'
```

## 识别规则与词库

- 中央显示刚按下的字母（大写），底部显示累计输入（小写）；每个字母的颜色与中央一致。
- 空格、退格、Enter、Tab 和修饰键显示各自图标或简短标记；空格在输入栏中显示为短横线。
- 每次按键都重新计时。按住任意键时不朗读；从最后全部松开开始计算停顿。
- 每轮只判断一次，然后清空输入。没有积压的音效或朗读队列。
- 只做整轮精确匹配：`cat` 会朗读，`xxcat`、`cat1`、`cat dog` 不会。
- 单个普通字母不朗读；词库内的单词才会朗读。
- 退格可以修改本轮输入；空格、数字、标点参与输入，因此会使整轮无法匹配单个词。
- 修饰键、Tab、Enter 仍有趣味反馈并延长计时，不添加字母；Enter 不提前朗读。
- 为避免孩子误按 Fn/Caps 进入另一套按键层，玩具固定使用按键上第一层字符。
- 输入最多保存 24 个字符；超长后本轮不朗读，不会误读截断后的前缀或后缀。

第一版内置 36 个词，完整列表见 [`assets/words.txt`](assets/words.txt)：

```text
apple baby ball banana bear bee bird blue boat bunny bus cake
car cat cow dad dog duck egg fish flower frog green happy
hello love milk mom moon pig rain red star sun tree yellow
```

## 本地开发

开发环境与下载缓存均放在本项目的 `.venv`、`.tools`、`.cache` 内。
当前项目已经安装好环境。后续操作始终使用包装脚本，避免使用系统的另一套 PlatformIO 配置：

```sh
./scripts/test.sh
./scripts/pio.sh run
./scripts/pio.sh run --target upload --upload-port /dev/cu.usbmodem101
./.venv/bin/python scripts/device.py status
```

USB 端口可能随插口或重新连接而改变；可用 `./scripts/pio.sh device list` 查询。

在另一台 Mac 重新部署时，需要安装 `uv` 和 Xcode Command Line Tools，然后执行：

```sh
./scripts/setup.sh
./scripts/test.sh
./scripts/pio.sh run
```

固定的版本：PlatformIO Core 6.1.19、Espressif32 平台 6.12.0、Arduino ESP32 2.0.17、
M5Cardputer 1.1.1、M5Unified 0.2.21、M5GFX 0.2.28、IRremote 4.7.1。
Python 依赖记录在 `requirements-dev.lock`。

## 音频与扩展

36 个英文词使用本机 macOS Samantha 英语语音生成，135 词/分钟，再以 0.9 倍速保留音调放慢，降低响度并柔化高频；
以 16kHz、单声道、16 位 PCM 存在固件 Flash 中。
四种原创合成音效为圆润水滴、柔和钢琴、轻木音、摇篮曲和声音色，各有五个音高，共 20 段。
采用较低的五声音阶，移除尖锐扫频和夸张弹簧声，降低响度，使用 22–55 毫秒渐入和柔和消退。
快速按键只切换音效而不排队；每个按键仍对应固定音色。

已生成的音频源码位于 `src/generated/`，正常构建不需要安装语音服务或 ffmpeg。
若需要调整词库：

1. 修改 `assets/words.txt`，保持字母排序、去重。
2. 在 Mac 上安装好 `ffmpeg`，运行 `python3 scripts/generate_audio.py`。
3. 在 `src/toy_visuals.cpp` 为新词补充图案；未实现专用图案时显示友好面孔。
4. 运行测试、编译、重新刷写。

生成器需要访问本机语音服务；在限制该服务的沙盒中可能只得到空音频，脚本会检测并报错。
`assets/audio_manifest.json` 保存每段语音的采样数和 PCM SHA-256。

## 检查与调试

`./scripts/test.sh` 使用本机 C++ 编译器与 AddressSanitizer/UndefinedBehaviorSanitizer，
检查等待边界、配置时间、长按计时、清空、大小写、无效字符、溢出、时钟回绕及全部音频素材。

真机 USB 集成检查：

```sh
./.venv/bin/python scripts/hardware_test.py
```

执行时请先不要碰键盘；测试会发声，临时修改等待时间，重启设备验证设置持久化，
最后恢复执行前的等待时间。通过 USB 注入的按键使用与实际键盘相同的输入、动画、播放路径，
但不能替代实体键盘和人耳听感检查。

读取当前设备绘制的画面（PPM）：

```sh
./.venv/bin/python scripts/device.py --frame artifacts/screen.ppm
sips -s format png artifacts/screen.ppm --out artifacts/screen.png
```

串口还支持 `clear`、`tap`、`key c`、`type cat`、`backspace`、`settings`、`save`、`cancel`、`reboot`。
这些只供 USB 调试，不出现在孩子的界面中。连接 USB 调试可能触发芯片复位。

## 原固件备份与恢复

首次刷写前已读取整块 8MB Flash，保存在本机：

- `artifacts/backups/original-flash.bin`
- `artifacts/backups/SHA256SUMS`

备份没有纳入 Git，可能包含原程序自己的设置。恢复时先检查校验和，再执行：

```sh
shasum -a 256 -c artifacts/backups/SHA256SUMS
./.venv/bin/python .tools/platformio/packages/tool-esptoolpy/esptool.py \
  --chip esp32s3 --port /dev/cu.usbmodem101 --baud 460800 \
  write_flash 0 artifacts/backups/original-flash.bin
```

这会恢复备份时的整个 Flash，也会覆盖玩具和家长设置。
如果无法连接下载模式，按官方方法：电源 OFF，按住 G0，再接通电源后松开。

## 参考

- [M5Stack Cardputer ADV 硬件与下载模式](https://docs.m5stack.com/en/core/Cardputer-Adv)
- [M5Cardputer 官方驱动](https://github.com/m5stack/M5Cardputer)
- [M5Unified 官方驱动](https://github.com/m5stack/M5Unified)
- [M5GFX 官方图形库](https://github.com/m5stack/M5GFX)
