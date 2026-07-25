#pragma once

#include "imgui/imgui.h"
#include "Constants.h"
#include "IBluetoothConnector.h"
#include "BluetoothWrapper.h"
#include "CommandSerializer.h"
#include "Exceptions.h"
#include "TimedMessageQueue.h"
#include "SingleInstanceFuture.h"
#include "CascadiaCodeFont.h"
#include "Headphones.h"
#include "MediaManager.h"

#include <future>
#include <array>
#include <map>
#include <string>
#include <cmath>

// ── Sony Companion App — Futuristic Glassmorphic UI Design System ───────────
constexpr auto GUI_MAX_MESSAGES = 5;
constexpr auto GUI_HEIGHT = 740;
constexpr auto GUI_WIDTH  = 490;
constexpr auto FPS = 60;
constexpr auto MS_PER_FRAME = 1000 / FPS;

constexpr auto FONT_SIZE_BODY   = 15.0f;
constexpr auto FONT_SIZE_HEADER = 20.0f;
constexpr auto FONT_SIZE_TITLE  = 24.0f;
constexpr auto FONT_SIZE_SMALL  = 12.0f;

// ── Color Palette (Sony Audio Glassy Dark) ──────────────────────────────────
const auto COLOR_CANVAS       = ImVec4(0.04f, 0.04f, 0.06f, 1.00f);  // #0A0A0F OLED Black
const auto COLOR_GLASS_BG     = ImVec4(0.09f, 0.10f, 0.14f, 0.88f);  // Glass card fill
const auto COLOR_GLASS_BORDER = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);  // Crisp rim
const auto COLOR_GLASS_HOVER  = ImVec4(0.14f, 0.15f, 0.22f, 0.95f);  // Card hover

const auto SONY_ORANGE        = ImVec4(1.00f, 0.42f, 0.21f, 1.00f);  // #FF6B35 Signature Accent
const auto SONY_ORANGE_GLOW   = ImVec4(1.00f, 0.42f, 0.21f, 0.30f);  // Glow
const auto SONY_BLUE          = ImVec4(0.00f, 0.66f, 0.91f, 1.00f);  // #00A8E8 Electric Blue
const auto SONY_BLUE_GLOW     = ImVec4(0.00f, 0.66f, 0.91f, 0.25f);  // Blue Glow
const auto SONY_TEAL          = ImVec4(0.00f, 0.85f, 0.73f, 1.00f);  // #00D9BB Emerald Teal

const auto COLOR_TEXT_MAIN    = ImVec4(0.96f, 0.96f, 0.98f, 1.00f);  // Bright text
const auto COLOR_TEXT_SUB     = ImVec4(0.58f, 0.61f, 0.69f, 1.00f);  // Subtitle
const auto COLOR_TEXT_MUTED   = ImVec4(0.40f, 0.42f, 0.49f, 1.00f);  // Muted

// Physical-button changes (ASM) poll rate
constexpr auto DYNAMIC_POLL_FRAMES = FPS * 2;
constexpr float TITLE_BAR_HEIGHT = 42.0f;

class CrossPlatformGUI
{
public:
	CrossPlatformGUI(BluetoothWrapper bt);

	// Perform single frame pass. Returns false when user exits.
	bool performGUIPass();

private:
	void _applyGlassTheme();

	// ── Render Sections ──────────────────────────────────────────────────
	void _drawTitleBar();
	void _drawErrors();
	void _drawDeviceDiscovery();
	void _drawHeroHeader();
	void _drawNowPlaying();
	void _drawASMControls();
	void _drawEqualizer();
	void _drawDseeSection();
	void _drawOptionalFeatures();
	void _drawSurroundControls();
	void _drawFooter();

	// ── Custom Glass Widgets ─────────────────────────────────────────────
	bool _drawToggle(const char* label, bool* v, const char* subtitle = nullptr);
	bool _drawPillButton(const char* label, bool selected, ImVec2 size);
	bool _drawGradientSlider(const char* id, int* v, int vMin, int vMax, const char* fmt = "%d");
	void _drawBatteryRing(float cx, float cy, float radius, float percent, bool charging, const char* label = nullptr);
	void _drawEqSpectrumBar(float x, float y, float w, float h, float normalizedVal, const char* label, bool isBass);

	// ── Connection Logic ─────────────────────────────────────────────────
	void _pumpConnectionState();
	void _syncUIFromHeadphones();
	void _sendPendingASMChanges();
	template <typename F> void _sendFeatureCommand(F&& fn);

	// ── Container Helpers ────────────────────────────────────────────────
	bool _beginGlassCard(const char* id, float height = 0.f);
	void _endGlassCard();
	void _sectionHeader(const char* title, const char* badgeText = nullptr);

	// ── Image Manager ────────────────────────────────────────────────────
	struct DeviceTexture { ImTextureRef ref; int w = 0; int h = 0; bool ok = false; };
	const DeviceTexture& _deviceTexture(const std::string& name);
	std::string _resourceBase();

	bool _isV2() { return _bt.getProtocolVersion() == SonyProtocolVersion::V2; }

	// ── Members ──────────────────────────────────────────────────────────
	BluetoothDevice _connectedDevice;
	BluetoothWrapper _bt;
	SingleInstanceFuture<std::vector<BluetoothDevice>> _connectedDevicesFuture;
	SingleInstanceFuture<void> _sendCommandFuture;
	SingleInstanceFuture<void> _featureCommandFuture;
	SingleInstanceFuture<void> _refreshFuture;
	SingleInstanceFuture<void> _probeFuture;
	SingleInstanceFuture<void> _pollFuture;
	SingleInstanceFuture<void> _connectFuture;
	TimedMessageQueue _mq;
	Headphones _headphones;

	bool _initialized = false;
	bool _synced = false;
	bool _probed = false;
	int _pollCounter = 0;
	bool _wantClose = false;

	// UI Local State
	bool _uiAsmOn = false;
	int _uiAsmMode = 0;          // 0 = Off, 1 = Noise Cancelling, 2 = Ambient Sound
	int _uiAsmLevel = 0;
	bool _uiFocusOnVoice = false;
	int _uiEqPreset = 0;
	std::array<int, 5> _uiEqBands = { 0, 0, 0, 0, 0 };
	int _uiClearBass = 0;
	bool _uiDsee = false;
	int _uiAutoPowerOff = 0;
	bool _uiSpeakToChat = false;
	bool _uiAdaptiveVolume = false;
	int _uiSoundPosition = 0;
	int _uiVptType = 0;

	// ── UX Polish & Tooltips ─────────────────────────────────────────────
	void _showToast(const std::string& text);
	void _drawToasts();
	void _drawTooltip(const char* text);
	void _saveLastDevice(const std::string& name, const std::string& mac);
	std::pair<std::string, std::string> _loadLastDevice();

	struct ToastNotification {
		std::string text;
		float timeRemaining = 2.5f;
	};
	std::vector<ToastNotification> _toasts;
	std::string _lastDeviceMac;
	std::string _lastDeviceName;
	std::map<std::string, DeviceTexture> _deviceTextures;
	float _animTimer = 0.0f;
};

template <typename F>
void CrossPlatformGUI::_sendFeatureCommand(F&& fn)
{
	if (_featureCommandFuture.valid())
		return;
	_featureCommandFuture.setFromAsync(std::forward<F>(fn));
}
