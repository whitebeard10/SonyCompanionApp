<div align="center">
  <img src="Client/windows/app.png" width="128" height="128" alt="Sony Companion App Logo">
  <h1>Sony Companion App</h1>
  <p><b>A sleek, premium DirectX 11 desktop companion for Sony Headphones on Windows</b></p>

  <p>
    <a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++20"></a>
    <a href="https://microsoft.com"><img src="https://img.shields.io/badge/Platform-Windows%2010%20%7C%2011-0078D6.svg" alt="Platform"></a>
    <a href="https://microsoft.com"><img src="https://img.shields.io/badge/Renderer-DirectX%2011-00D9BB.svg" alt="Renderer"></a>
    <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-orange.svg" alt="License"></a>
  </p>
</div>

---

## 🎧 Overview

**Sony Companion App** is a modern, lightweight desktop audio control application designed specifically for Sony Bluetooth headphones on Windows. It delivers complete control over Noise Cancelling, Ambient Sound Mode, 5-Band Equalizer, Clear Bass, and DSEE audio upscaling without needing your mobile phone.

Built with **C++20**, **DirectX 11**, and **Dear ImGui**, featuring a dark glassmorphic design system, smooth Windows 11 DWM rounded corners, and instant Bluetooth control.

---

## ✨ Features

- 🎚️ **Ambient Sound Control & Noise Cancelling**
  - Seamlessly switch between Noise Cancelling, Ambient Sound Mode (Level 0–20), and Off.
  - **Focus on Voice** passthrough toggle.

- 🎛️ **5-Band Equalizer & Clear Bass**
  - Presets: *Bright, Excited, Mellow, Relaxed, Vocal, Treble Boost, Bass Boost, Speech, Manual*.
  - Individual frequency sliders (*400Hz, 1kHz, 2.4kHz, 6.3kHz, 16kHz*) + **Clear Bass** control.

- ✨ **DSEE Audio Upscaling**
  - Toggle Sony's proprietary Digital Sound Enhancement Engine (DSEE HX / Ultimate).

- 🔋 **Live Battery Monitoring**
  - Real-time battery gauges for individual Left/Right earbuds and charging case for TWS models, or single battery ring for over-ear models.

- 🎧 **Smart Device Matching & Graphics**
  - Case-insensitive & substring model matching for custom Bluetooth names (e.g. `WH-CH720N`, `WH-1000XM4`, `LE_WH-1000XM5`, `LinkBuds S`).
  - Direct3D 11 GPU texture pipeline for loading high-resolution device renders.

- 🧩 **Smart Capability Detection**
  - Auto Power-Off timeouts, Speak-to-Chat, and Adaptive Volume Control.

- 🧬 **Dual Protocol Engine**
  - Automatic detection for **v1 Protocol** (WH-1000XM3, WH-1000XM4) and **v2 Protocol** (WH-CH720N, WH-1000XM5, WF-series).

- 🖼️ **Modern Windows Integration**
  - Borderless window with dark title bar, Windows 11 DWM rounded corners (`DWMWCP_ROUND`), drop shadow (`CS_DROPSHADOW`), single-instance process lock, and embedded multi-resolution Windows app icon (`app.ico`).

---

## 📱 Supported Sony Headphones

- **WH Series**: WH-1000XM6, WH-1000XM5, WH-1000XM4, WH-1000XM3, WH-1000XM2, WH-CH720N, WH-CH520, WH-XB910N, WH-XB900N
- **WF Series**: WF-1000XM5, WF-1000XM4, WF-C700N, LinkBuds S
- **MDR Series**: MDR-XB950BT

---

## 🛠️ Building from Source

### Requirements
- **Windows 10 / 11**
- **Visual Studio 2022** (with *Desktop development with C++*)
- **CMake 3.15+**

### Building
```powershell
cd Client
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The compiled binary `SonyCompanionApp.exe` will be generated in `Client/build/Release/` and copied to the project root.

---

## 🚀 Usage

1. **Pair Headphones**: Ensure your Sony headphones are paired with Windows via Bluetooth.
2. **Launch App**: Run `SonyCompanionApp.exe`.
3. **Connect**: Select your headphones from the device list and click **Connect Device**.
4. **Enjoy**: Adjust Noise Cancelling, Equalizer, or DSEE settings directly from your desktop.

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## ⚠️ Disclaimer

This project is an independent open-source client and is not affiliated with, endorsed by, or associated with Sony Corporation.
