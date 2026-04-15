# Neon 80 Plus Firmware

项目基于怒喵（Angry Miao）提供的开源代码进行开发与适配。

## 功能说明

- 基于 QMK 固件框架
- 使用 `vial` keymap 编译，支持 Vial 配置
- 已适配 SignalRGB 软件

## 编译

在仓库根目录执行：

```bash
make angrymiao2:vial
```

编译完成后会生成对应固件文件（例如 `angrymiao2_vial.bin`）。

## 固件文件位置

原始固件和适配 SignalRGB 的固件都放在仓库根目录的 `firmware/` 文件夹下。

- 原始固件：`firmware/怒喵官方固件/angrymiao_804_vial.bin`
- 适配 SignalRGB 的固件：`firmware/angrymiao_signalrgb_0_1.bin`

## SignalRGB 脚本使用方法

仓库内脚本文件：

- `keyboards/angrymiao2/signalrgb_neon80.js`

请将该文件复制到 SignalRGB 插件目录，并命名为 `AM_Neon_80.js`：

- `C:\Users\<你的用户名>\Documents\WhirlwindFX\Plugins\AM_Neon_80.js`

PowerShell 示例命令：

```powershell
Copy-Item -LiteralPath ".\keyboards\angrymiao2\signalrgb_neon80.js" -Destination "$env:USERPROFILE\Documents\WhirlwindFX\Plugins\AM_Neon_80.js" -Force
```

## 使用与刷机注意事项

1. 使用 Vial 前，请先退出 SignalRGB 软件。
2. 刷机工具使用 QMK Toolbox。
3. 键盘进入 DFU 模式的方法：按住 `Esc` 键上电（插入 USB），MCU选择ATmega32U4。
4. 进入 DFU 后，在 QMK Toolbox 中选择固件并烧录。
