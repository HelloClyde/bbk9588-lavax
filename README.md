# LavaXVM for BBK 9588

<p align="center">
  <img src="assets/lavax-icon.png" width="180" alt="LavaXVM for BBK 9588 icon">
</p>

这是把 LavaXVM 3.5 移植到步步高 9588 原生 BDA 的实验性端口。VM 解释器直接以
MIPS BDA 运行，显示、实体键、触摸、时钟和文件系统由 9588 平台层实现。

## 当前实现

- 支持 LavaX 常见的 `160x80`（2 倍）和 `240x160` 画面，横屏旋转到 9588。
- 实体方向键随横屏方向映射；Enter/ESC 直接传给 LavaX。
- 屏幕下方提供 QWERTY、功能键、翻页键和方向软键盘。
- 触摸游戏区会转换成 LavaX 逻辑坐标。
- 自动查找 `B:\LavaXOS`，找不到时回退到 `A:\LavaXOS`。
- 同时按住 Enter + ESC 1.5 秒退出 BDA。
- 使用参考原版 LVM 掌机造型重新设计的透明菜单图标，源图位于 `assets\lavax-icon.png`。

## 构建

```powershell
cd lavax-for9588
.\tools\bootstrap.ps1
.\tools\build.ps1 -Clean
```

`bootstrap.ps1` 会按 `deps.lock.psd1` 拉取固定版本的 SDK；也可通过
`BDA_SDK_ROOT` 指定已有 SDK。
输出为 `build\LavaX-9588.bda`。

启动隔离的 9588 模拟器测试 NAND：

```powershell
$env:BBK9588_EMULATOR_ROOT = 'C:\path\to\bbk9588-emulator'
.\tools\test-emulator.ps1 -ResetImage
```

模拟器中导入自有运行文件（模拟器必须已停止）：

```powershell
python .\tools\import-runtime-emulator.py C:\path\to\LavaXOS `
  --emulator-root C:\path\to\bbk9588-emulator `
  --nand C:\path\to\bbk9588-emulator\runtime\bda_test\bbk9588_nand.bin
```

脚本校验 `Shell.sys`，复制后复核文件，并明确排除所有 `.nds`。
仅在隔离测试 NAND 中，可用 `--boot-program path\game.lav` 临时把指定游戏作为
Shell 启动，用于端口冒烟；它不会改动源目录。

## 放置运行文件

本仓库只包含 GPL-2.0 的 LavaXVM 源码，不分发 LavaXOS 系统、游戏或 `.nds` ROM。
请从你有权使用的原版介质中，把整个 `LavaXOS` 目录复制到设备：

```text
B:\LavaXOS\System\Shell.sys
B:\LavaXOS\System\Config.ini
B:\LavaXOS\PROGRAM\...
```

也可以放在 `A:\LavaXOS`。BDA 与 `LavaXOS` 目录相互独立，不要把 `.nds` 文件复制进
BDA。若缺少 `Shell.sys`，程序会显示可读的诊断页面而不是黑屏。

## 已知边界

- 9588 SDK 当前没有公开、已验证的删除文件 API，所以 LavaX 的删文件操作返回失败；
  新建目录、读写、定位和枚举目录已实现。
- RTC 暂用 VM 启动后的软件时钟；不会修改设备系统时间。
- 声音仍是空实现（上游 NDS 版的 `c_beep` 本身也是空操作）。
- 首版目标是兼容官方 Shell 与普通 `.lav` 游戏；依赖 NDS 双屏特性的个别程序可能需要
  单独适配。

第三方版本和许可边界见 [THIRD_PARTY.md](THIRD_PARTY.md)。

## 许可证

本项目整体按 [GNU GPL v2.0](LICENSE) 发布。LavaXVM 的上游来源、固定版本和本地修改
说明见 [THIRD_PARTY.md](THIRD_PARTY.md)。LavaXOS 运行文件、游戏及 NDS ROM 不包含在
本仓库中。
