# Sony Companion App

A premium desktop companion for Sony headphones on Windows — control Noise Cancelling, Ambient Sound, EQ, DSEE & battery, without the phone.

## Features

- 🎚️ **Ambient Sound Control** — Noise Cancelling · Ambient Sound (0–20 levels) · Off
- 🗣️ **Focus on Voice** passthrough
- 🎛️ **Equalizer** — presets and Manual mode with 5 bands + Clear Bass
- ✨ **DSEE** — Sony's audio upscaling toggle
- 🔋 **Battery Level** — live arc gauge with per-earbud + case for TWS
- 🎧 **Codec & Firmware** readout
- 🧩 **Capability-gated extras** — Auto Power-Off · Speak-to-Chat · Adaptive Volume
- 🖼️ **Device hero image** — official Sony product renders
- 🔄 **Live button sync** — headphone changes reflect in the app
- 🧬 **Dual-protocol** — auto-detects v1 (WH-1000XM3/XM4) or v2 (WH-CH720N, XM5, WF-series)

## UI Design

- **Glassmorphic dark theme** — OLED-black with translucent cards
- **Borderless window** — custom title bar, no Windows chrome
- **Custom widgets** — arc battery gauge, gradient sliders, animated toggle switches
- **Sony-inspired palette** — warm orange accent, electric blue, teal highlights

## Build

Requires Visual Studio 2022 with C++ desktop development workload.

```powershell
cd Client
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

The built `SonyCompanionApp.exe` will be in `build/Release/`. Keep it next to the `resources/` folder.

## Usage

1. Pair your Sony headphones in Windows Bluetooth settings first
2. Run `SonyCompanionApp.exe`
3. Select your headphones and click Connect
4. Keep audio playing — Sony headsets drop the control link when idle

## Credits

Based on [SonyBridge](https://github.com/AmitRajput-Dev/SonyBridge) by AmitRajput-Dev (MIT License).
Protocol reverse-engineering by [SonyHeadphonesClient](https://github.com/Plutoberth/SonyHeadphonesClient) and [GadgetBridge](https://codeberg.org/Freeyourgadget/Gadgetbridge).

## License

MIT — see [LICENSE](LICENSE).

## Disclaimer

This project is not affiliated with, endorsed by, or connected to Sony. Use at your own risk.
