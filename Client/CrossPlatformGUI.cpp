#include "CrossPlatformGUI.h"

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifndef IM_PI
#define IM_PI 3.14159265358979323846f
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// MAIN FRAME RENDERER
// ═══════════════════════════════════════════════════════════════════════════════

bool CrossPlatformGUI::performGUIPass()
{
	ImGui::NewFrame();

	ImGuiIO& io = ImGui::GetIO();
	_animTimer += io.DeltaTime;

	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize(io.DisplaySize);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("##root_window", nullptr,
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoScrollbar);
	ImGui::PopStyleVar();

	// Render custom borderless dark title bar
	this->_drawTitleBar();

	// Main content area with smooth scrollbar
	ImGui::SetCursorPos(ImVec2(16.f, TITLE_BAR_HEIGHT + 6.f));
	ImGui::BeginChild("##content_viewport",
		ImVec2(io.DisplaySize.x - 32.f, io.DisplaySize.y - TITLE_BAR_HEIGHT - 38.f),
		ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground);

	this->_drawErrors();
	this->_drawDeviceDiscovery();

	if (this->_bt.isConnected())
	{
		this->_pumpConnectionState();

		if (this->_synced)
		{
			this->_drawHeroHeader();
			this->_drawNowPlaying();
			this->_drawASMControls();
			if (this->_isV2())
			{
				this->_drawEqualizer();
				this->_drawDseeSection();
			}
			this->_drawOptionalFeatures();
			if (!this->_isV2())
				this->_drawSurroundControls();

			this->_sendPendingASMChanges();
		}
		else
		{
			ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
			float pulse = 0.5f + 0.5f * sinf(_animTimer * 5.0f);
			ImGui::SetCursorPosX((io.DisplaySize.x - 220.f) * 0.5f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, pulse));
			ImGui::Text("Syncing device settings  %c", "|/-\\"[(int)(ImGui::GetTime() / 0.08f) & 3]);
			ImGui::PopStyleColor();
		}
	}
	else
	{
		this->_synced = false;
		this->_probed = false;
		this->_initialized = false;
		this->_pollCounter = 0;
	}

	ImGui::EndChild();

	// Subtle bottom status bar
	this->_drawFooter();

	ImGui::End();
	this->_drawToasts();
	ImGui::Render();

	return !_wantClose;
}

// ═══════════════════════════════════════════════════════════════════════════════
// CUSTOM TITLE BAR
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawTitleBar()
{
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 p0(0, 0);
	ImVec2 p1(io.DisplaySize.x, TITLE_BAR_HEIGHT);

	// Deep header background
	dl->AddRectFilled(p0, p1, IM_COL32(10, 10, 15, 255));

	// Neon accent divider line at bottom of title bar
	dl->AddLine(ImVec2(0, TITLE_BAR_HEIGHT), ImVec2(io.DisplaySize.x, TITLE_BAR_HEIGHT),
		IM_COL32(255, 107, 53, 50), 1.0f);

	// App Icon / Logo Dot
	float dotX = 18.f, dotY = TITLE_BAR_HEIGHT * 0.5f;
	dl->AddCircleFilled(ImVec2(dotX, dotY), 4.5f, IM_COL32(255, 107, 53, 255));
	dl->AddCircle(ImVec2(dotX, dotY), 7.5f, IM_COL32(255, 107, 53, 100), 0, 1.5f);

	// App Title
	ImGui::SetCursorPos(ImVec2(32.f, 10.f));
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MAIN);
	ImGui::Text("SONY COMPANION");
	ImGui::PopStyleColor();

	// Sub-label
	ImGui::SameLine();
	ImGui::SetCursorPosY(12.f);
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MUTED);
	ImGui::Text("  Audio Control System");
	ImGui::PopStyleColor();

	// Close Button
	float btnSize = 24.f;
	float closeX = io.DisplaySize.x - btnSize - 12.f;
	ImGui::SetCursorPos(ImVec2(closeX, 9.f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.20f, 0.20f, 0.70f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.95f, 0.10f, 0.10f, 0.90f));
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_SUB);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
	if (ImGui::Button("x##close", ImVec2(btnSize, btnSize)))
		_wantClose = true;
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(4);

	// Minimize Button
	float minX = closeX - btnSize - 6.f;
	ImGui::SetCursorPos(ImVec2(minX, 9.f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.10f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.18f));
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_SUB);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
	if (ImGui::Button("-##min", ImVec2(btnSize, btnSize)))
	{
#ifdef _WIN32
		HWND hwnd = (HWND)ImGui::GetMainViewport()->PlatformHandle;
		if (hwnd) ShowWindow(hwnd, SW_MINIMIZE);
#endif
	}
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(4);
}

void CrossPlatformGUI::_drawFooter()
{
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	float h = 26.f;
	ImVec2 p0(0, io.DisplaySize.y - h);
	ImVec2 p1(io.DisplaySize.x, io.DisplaySize.y);

	dl->AddRectFilled(p0, p1, IM_COL32(8, 8, 12, 255));
	dl->AddLine(p0, ImVec2(io.DisplaySize.x, p0.y), IM_COL32(255, 255, 255, 10));

	ImGui::SetCursorPos(ImVec2(16.f, io.DisplaySize.y - h + 4.f));
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MUTED);
	ImGui::Text("Sony Headphones Client v1.0.0");
	ImGui::SameLine(io.DisplaySize.x - 140.f);
	ImGui::Text("%s", this->_bt.isConnected() ? "Connected" : "Disconnected");
	ImGui::PopStyleColor();
}

// ═══════════════════════════════════════════════════════════════════════════════
// ERRORS DISPLAY
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawErrors()
{
	if (this->_mq.begin() == this->_mq.end())
		return;

	for (auto&& message : this->_mq)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
		ImGui::TextWrapped("! %s", message.message.c_str());
		ImGui::PopStyleColor();
	}
	ImGui::Spacing();
}

// ═══════════════════════════════════════════════════════════════════════════════
// DEVICE DISCOVERY SCREEN
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawDeviceDiscovery()
{
	static std::vector<BluetoothDevice> connectedDevices;
	static int selectedDevice = -1;

	if (!this->_beginGlassCard("##discovery")) { this->_endGlassCard(); return; }

	if (this->_bt.isConnected())
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 p = ImGui::GetCursorScreenPos();
		dl->AddCircleFilled(ImVec2(p.x + 8.f, p.y + 10.f), 4.f, IM_COL32(0, 217, 187, 255));

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 18.f);
		ImGui::PushStyleColor(ImGuiCol_Text, SONY_TEAL);
		ImGui::Text("CONNECTED");
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::TextColored(COLOR_TEXT_MAIN, "  %s", this->_connectedDevice.name.c_str());

		ImGui::Spacing();
		if (_drawPillButton("Disconnect", false, ImVec2(130, 32)))
		{
			selectedDevice = -1;
			this->_bt.disconnect();
		}
		this->_endGlassCard();
		return;
	}

	if (this->_connectedDevicesFuture.valid() && this->_connectedDevicesFuture.ready())
	{
		try {
			connectedDevices = this->_connectedDevicesFuture.get();
			if (selectedDevice == -1 && !_lastDeviceMac.empty())
			{
				for (int i = 0; i < (int)connectedDevices.size(); ++i)
				{
					if (connectedDevices[i].mac == _lastDeviceMac)
					{
						selectedDevice = i;
						break;
					}
				}
			}
		}
		catch (const RecoverableException& exc) {
			if (exc.shouldDisconnect) this->_bt.disconnect();
			this->_mq.addMessage(exc.what());
		}
	}

	this->_sectionHeader("Select Headphones", "BLUETOOTH");

	int temp = 0;
	if (connectedDevices.empty())
	{
		ImGui::TextColored(COLOR_TEXT_MUTED, "No paired Sony devices detected yet.");
	}

	for (const auto& device : connectedDevices)
	{
		std::string name = device.name.empty() ? "Sony Device" : device.name;
		std::string label = name + "##" + device.mac;
		bool sel = selectedDevice == temp;

		ImGui::PushStyleColor(ImGuiCol_Header, sel ? ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, 0.20f) : ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0.06f));
		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.04f, 0.5f));
		if (ImGui::Selectable(label.c_str(), sel, ImGuiSelectableFlags_None, ImVec2(0, 36)))
			selectedDevice = temp;
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
		temp++;
	}

	ImGui::Spacing(); ImGui::Spacing();

	if (this->_connectFuture.valid())
	{
		if (this->_connectFuture.ready())
		{
			try {
				this->_connectFuture.get();
				_saveLastDevice(this->_connectedDevice.name, this->_connectedDevice.mac);
				_showToast("Connected to " + this->_connectedDevice.name);
			}
			catch (const RecoverableException& exc) {
				if (exc.shouldDisconnect) this->_bt.disconnect();
				this->_mq.addMessage(exc.what());
			}
		}
		else
		{
			float pulse = 0.5f + 0.5f * sinf(_animTimer * 6.0f);
			ImGui::TextColored(ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, pulse),
				"Connecting to device  %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
		}
	}
	else
	{
		if (_drawPillButton("Connect Device", true, ImVec2(150, 36)))
		{
			if (selectedDevice != -1)
			{
				this->_connectedDevice = connectedDevices[selectedDevice];
				this->_connectFuture.setFromAsync([this]() { this->_bt.connect(this->_connectedDevice.mac); });
			}
		}
	}

	ImGui::SameLine();

	if (this->_connectedDevicesFuture.valid())
	{
		if (this->_connectedDevicesFuture.ready())
		{
			try { connectedDevices = this->_connectedDevicesFuture.get(); }
			catch (const RecoverableException& exc) {
				if (exc.shouldDisconnect) this->_bt.disconnect();
				this->_mq.addMessage(exc.what());
			}
		}
		else
		{
			ImGui::TextColored(COLOR_TEXT_SUB, "Scanning Bluetooth  %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
		}
	}
	else if (_drawPillButton("Scan Devices", false, ImVec2(130, 36)))
	{
		selectedDevice = -1;
		this->_connectedDevicesFuture.setFromAsync([this]() { return this->_bt.getConnectedDevices(); });
	}

	this->_endGlassCard();
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONNECTION STATE MACHINE
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_pumpConnectionState()
{
	if (!this->_synced && !this->_refreshFuture.valid())
	{
		this->_refreshFuture.setFromAsync([this]() {
			try { if (!this->_initialized) { this->_headphones.initDevice(); this->_initialized = true; } }
			catch (const std::exception&) {}
			try { this->_headphones.requestAmbientState(); } catch (const std::exception&) {}
			if (this->_isV2())
			{
				try { this->_headphones.requestBattery(); } catch (const std::exception&) {}
				try { this->_headphones.requestEqualizer(); } catch (const std::exception&) {}
				try { this->_headphones.requestDsee(); } catch (const std::exception&) {}
			}
		});
	}

	if (this->_refreshFuture.valid() && this->_refreshFuture.ready())
	{
		try { this->_refreshFuture.get(); } catch (const std::exception&) {}
		this->_syncUIFromHeadphones();
		this->_synced = true;
		this->_pollCounter = 0;
	}

	if (!this->_synced)
		return;

	if (this->_isV2() && !this->_probed && !this->_probeFuture.valid())
	{
		this->_probeFuture.setFromAsync([this]() { try { this->_headphones.probeCapabilities(); } catch (const std::exception&) {} });
	}
	if (this->_probeFuture.valid() && this->_probeFuture.ready())
	{
		try { this->_probeFuture.get(); } catch (const std::exception&) {}
		this->_probed = true;
		this->_uiAutoPowerOff = this->_headphones.getAutoPowerOff();
		this->_uiSpeakToChat = this->_headphones.getSpeakToChat();
		this->_uiAdaptiveVolume = this->_headphones.getAdaptiveVolume();
	}

	if (this->_pollFuture.valid() && this->_pollFuture.ready())
	{
		try { this->_pollFuture.get(); } catch (const std::exception&) {}
		if (!this->_sendCommandFuture.valid())
		{
			this->_uiAsmOn = this->_headphones.getAmbientSoundControl();
			int lvl = this->_headphones.getAsmLevel();
			this->_uiAsmMode = !this->_uiAsmOn ? 0 : (lvl > 0 ? 2 : 1);
			if (lvl > 0) this->_uiAsmLevel = lvl;
			this->_uiFocusOnVoice = this->_headphones.getFocusOnVoice();
		}
	}
	if (!this->_pollFuture.valid() && ++this->_pollCounter >= DYNAMIC_POLL_FRAMES)
	{
		this->_pollCounter = 0;
		this->_pollFuture.setFromAsync([this]() { this->_headphones.requestAmbientState(); });
	}
}

void CrossPlatformGUI::_syncUIFromHeadphones()
{
	this->_uiAsmOn = this->_headphones.getAmbientSoundControl();
	int lvl = this->_headphones.getAsmLevel();
	this->_uiAsmMode = !this->_uiAsmOn ? 0 : (lvl > 0 ? 2 : 1);
	this->_uiAsmLevel = lvl > 0 ? lvl : 10;
	this->_uiFocusOnVoice = this->_headphones.getFocusOnVoice();

	this->_uiEqPreset = (int)(unsigned char)this->_headphones.getEqualizerPreset();
	for (int i = 0; i < 5; ++i)
		this->_uiEqBands[i] = this->_headphones.getEqualizerBand(i);
	this->_uiClearBass = this->_headphones.getClearBass();
	this->_uiDsee = this->_headphones.getDsee();

	this->_uiAutoPowerOff = this->_headphones.getAutoPowerOff();
	this->_uiSpeakToChat = this->_headphones.getSpeakToChat();
	this->_uiAdaptiveVolume = this->_headphones.getAdaptiveVolume();
}

// ═══════════════════════════════════════════════════════════════════════════════
// HERO HEADER (Headphone Image + Battery Ring + Details)
// ═══════════════════════════════════════════════════════════════════════════════

std::string CrossPlatformGUI::_resourceBase()
{
#if defined(_WIN32)
	char buf[MAX_PATH];
	DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
	std::string p(buf, n > 0 ? n : 0);
	auto s = p.find_last_of("\\/");
	return s == std::string::npos ? std::string(".") : p.substr(0, s);
#else
	return ".";
#endif
}

const CrossPlatformGUI::DeviceTexture& CrossPlatformGUI::_deviceTexture(const std::string& name)
{
	std::string slug = name;
	std::transform(slug.begin(), slug.end(), slug.begin(), [](unsigned char ch) { return (char)std::tolower(ch); });
	std::replace(slug.begin(), slug.end(), ' ', '-');

	auto it = this->_deviceTextures.find(slug);
	if (it != this->_deviceTextures.end())
		return it->second;

	// List of supported model slugs and keywords
	static const struct {
		const char* keyword;
		const char* targetSlug;
	} MODEL_MAP[] = {
		{ "wh-1000xm6", "wh-1000xm6" },
		{ "wh-1000xm5", "wh-1000xm5" },
		{ "wf-1000xm5", "wf-1000xm5" },
		{ "wh-1000xm4", "wh-1000xm4" },
		{ "wf-1000xm4", "wf-1000xm4" },
		{ "wh-1000xm3", "wh-1000xm3" },
		{ "wh-1000xm2", "wh-1000xm2" },
		{ "ch720n",     "wh-ch720n" },
		{ "ch520",      "wh-ch520" },
		{ "c700n",      "wf-c700n" },
		{ "linkbuds",   "linkbuds-s" },
		{ "xb910n",     "wh-xb910n" },
		{ "xb900n",     "wh-xb900n" },
		{ "xb950bt",    "mdr-xb950bt" },
		{ "1000xm5",    "wh-1000xm5" },
		{ "1000xm4",    "wh-1000xm4" },
		{ "1000xm3",    "wh-1000xm3" },
		{ "1000xm2",    "wh-1000xm2" }
	};

	std::string matchedSlug = slug;
	for (const auto& entry : MODEL_MAP)
	{
		if (slug.find(entry.keyword) != std::string::npos)
		{
			matchedSlug = entry.targetSlug;
			break;
		}
	}

	DeviceTexture out;
	const std::string base = this->_resourceBase();
	const std::string candidates[] = {
		base + "/resources/devices/" + matchedSlug + ".png",
		base + "/" + matchedSlug + ".png",
		"resources/devices/" + matchedSlug + ".png",
		base + "/resources/devices/" + slug + ".png",
		base + "/resources/devices/wh-1000xm4.png", // Default fallback
	};

	int w = 0, h = 0, comp = 0;
	unsigned char* pixels = nullptr;
	for (const auto& path : candidates)
	{
		pixels = stbi_load(path.c_str(), &w, &h, &comp, 4);
		if (pixels) break;
	}

#if defined(_WIN32)
extern void* CreateD3D11TextureFromRGBA(const unsigned char* pixels, int width, int height);
#endif

	if (pixels && w > 0 && h > 0)
	{
#if defined(_WIN32)
		void* srv = CreateD3D11TextureFromRGBA(pixels, w, h);
		if (srv)
		{
			out.ref = (ImTextureID)srv;
			out.w = w;
			out.h = h;
			out.ok = true;
		}
#endif
	}
	if (pixels)
		stbi_image_free(pixels);

	auto res = this->_deviceTextures.emplace(slug, out);
	return res.first->second;
}

void CrossPlatformGUI::_drawHeroHeader()
{
	if (!this->_beginGlassCard("##hero_card")) { this->_endGlassCard(); return; }

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 wp = ImGui::GetCursorScreenPos();
	float cardW = ImGui::GetContentRegionAvail().x;

	// Device image hero section with pulsating radial ambient glow
	const DeviceTexture& hero = this->_deviceTexture(this->_connectedDevice.name);
	if (hero.ok)
	{
		const float maxH = 145.f;
		float scale = (std::min)(cardW / (float)hero.w, maxH / (float)hero.h);
		if (scale > 1.f) scale = 1.f;
		const ImVec2 sz(hero.w * scale, hero.h * scale);
		const float offx = (cardW - sz.x) * 0.5f;

		// Ambient Glow
		float glowRadius = sz.y * 0.65f + 5.f * sinf(_animTimer * 2.f);
		ImVec2 glowCenter(wp.x + offx + sz.x * 0.5f, wp.y + sz.y * 0.5f);
		dl->AddCircleFilled(glowCenter, glowRadius, IM_COL32(255, 107, 53, 18), 64);
		dl->AddCircleFilled(glowCenter, glowRadius * 0.6f, IM_COL32(0, 168, 232, 12), 64);

		if (offx > 0.f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offx);
		ImGui::Image(hero.ref, sz);
		ImGui::Spacing();
	}

	// Device Name Header
	std::string devName = this->_connectedDevice.name.empty() ? "Sony Headphones" : this->_connectedDevice.name;
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MAIN);
	ImGui::Text("%s", devName.c_str());
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::TextColored(COLOR_TEXT_MUTED, "[ %s ]", this->_isV2() ? "v2 Protocol" : "v1 Protocol");

	ImGui::Spacing();

	// Battery Section with Circular Ring Gauges
	if (this->_headphones.hasDualBattery())
	{
		int left = this->_headphones.getBatteryLeft();
		int right = this->_headphones.getBatteryRight();
		int caseB = this->_headphones.getBatteryCase();

		ImVec2 pos = ImGui::GetCursorScreenPos();
		float r = 20.f;
		float startY = pos.y + r + 2.f;

		_drawBatteryRing(pos.x + 24.f, startY, r, (float)left / 100.f, false, "LEFT");
		_drawBatteryRing(pos.x + 94.f, startY, r, (float)right / 100.f, false, "RIGHT");
		if (caseB >= 0)
			_drawBatteryRing(pos.x + 164.f, startY, r, (float)caseB / 100.f, false, "CASE");

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + r * 2.f + 16.f);
	}
	else
	{
		int batt = this->_headphones.getBatteryLevel();
		if (batt >= 0)
		{
			ImVec2 pos = ImGui::GetCursorScreenPos();
			float r = 24.f;
			_drawBatteryRing(pos.x + 28.f, pos.y + r + 2.f, r, (float)batt / 100.f, this->_headphones.isBatteryCharging(), "BATTERY");

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + r * 2.f + 24.f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.f);
			ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MAIN);
			ImGui::Text("%d%% Charge", batt);
			ImGui::PopStyleColor();
			if (this->_headphones.isBatteryCharging())
			{
				ImGui::SameLine();
				ImGui::TextColored(SONY_TEAL, "( Charging )");
			}
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + r * 2.f - 6.f);
		}
	}

	// Codec & Firmware badges
	ImGui::Spacing();
	if (this->_headphones.hasCodec())
	{
		ImGui::TextColored(COLOR_TEXT_SUB, "Codec");
		ImGui::SameLine(100);
		ImGui::TextColored(SONY_BLUE, "%s", this->_headphones.getCodec().c_str());
	}
	if (this->_headphones.hasFirmware())
	{
		ImGui::TextColored(COLOR_TEXT_MUTED, "Firmware");
		ImGui::SameLine(100);
		ImGui::TextColored(COLOR_TEXT_MUTED, "%s", this->_headphones.getFirmware().c_str());
	}

	this->_endGlassCard();
}

void CrossPlatformGUI::_drawNowPlaying()
{
	if (!this->_beginGlassCard("##now_playing_card")) { this->_endGlassCard(); return; }

	MediaInfo media = MediaManager::Instance().getCurrentMediaInfo();

	this->_sectionHeader("Now Playing", media.isPlaying ? "PLAYING" : "PAUSED");

	// Display Track Title and Artist
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MAIN);
	ImGui::TextWrapped(" %s", media.title.c_str());
	ImGui::PopStyleColor();

	if (!media.artist.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, SONY_BLUE);
		ImGui::TextWrapped(" %s", media.artist.c_str());
		ImGui::PopStyleColor();
	}

	ImGui::Spacing();

	// Media Controls: [⏮ Prev]  [⏯ Play / Pause]  [⏭ Next]
	float totalW = ImGui::GetContentRegionAvail().x;
	float btnW = (totalW - 8.f) / 3.f;

	if (_drawPillButton("Prev", false, ImVec2(btnW, 34)))
	{
		MediaManager::Instance().previousTrack();
	}

	ImGui::SameLine(0, 4.f);

	std::string ppLabel = media.isPlaying ? "Pause" : "Play";
	if (_drawPillButton(ppLabel.c_str(), media.isPlaying, ImVec2(btnW, 34)))
	{
		MediaManager::Instance().playPause();
	}

	ImGui::SameLine(0, 4.f);

	if (_drawPillButton("Next", false, ImVec2(btnW, 34)))
	{
		MediaManager::Instance().nextTrack();
	}

	this->_endGlassCard();
}

// ═══════════════════════════════════════════════════════════════════════════════
// AMBIENT SOUND CONTROLS
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawASMControls()
{
	if (!this->_beginGlassCard("##asm_card")) { this->_endGlassCard(); return; }
	this->_sectionHeader("Ambient Sound Control", "NOISE CANCELLING");

	// 3-Way Segmented Mode Buttons
	const char* modes[] = { "Off", "Noise Cancelling", "Ambient Sound" };
	float cardW = ImGui::GetContentRegionAvail().x;
	float btnW = (cardW - 8.f) / 3.f;

	for (int i = 0; i < 3; ++i)
	{
		if (i) ImGui::SameLine(0, 4.f);
		if (_drawPillButton(modes[i], this->_uiAsmMode == i, ImVec2(btnW, 36)))
		{
			this->_uiAsmMode = i;
			_showToast(std::string("Ambient Sound: ") + modes[i]);
		}
		_drawTooltip("Switch to Noise Cancelling / Ambient Sound / Off");
	}

	// Slider when Ambient Sound mode (2) is active
	if (this->_uiAsmMode == 2)
	{
		int maxLevel = this->_isV2() ? 20 : 19;
		if (this->_uiAsmLevel < 1) this->_uiAsmLevel = 1;
		if (this->_uiAsmLevel > maxLevel) this->_uiAsmLevel = maxLevel;

		ImGui::Spacing(); ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_SUB);
		ImGui::Text("Passthrough Volume");
		ImGui::PopStyleColor();

		_drawGradientSlider("##asmlevel_slider", &this->_uiAsmLevel, 1, maxLevel, "Level %d");

		if (this->_headphones.isFocusOnVoiceAvailable())
		{
			ImGui::Spacing();
			_drawToggle("Focus on Voice", &this->_uiFocusOnVoice, "Emphasize human speech over background noise");
		}
	}

	// Push state back to model
	switch (this->_uiAsmMode)
	{
	case 0:
		this->_headphones.setAmbientSoundControl(false);
		break;
	case 1:
		this->_headphones.setAmbientSoundControl(true);
		this->_headphones.setAsmLevel(0);
		this->_headphones.setFocusOnVoice(false);
		break;
	case 2:
		this->_headphones.setAmbientSoundControl(true);
		this->_headphones.setAsmLevel(this->_uiAsmLevel < 1 ? 1 : this->_uiAsmLevel);
		this->_headphones.setFocusOnVoice(this->_uiFocusOnVoice);
		break;
	}

	this->_endGlassCard();
}

// ═══════════════════════════════════════════════════════════════════════════════
// EQUALIZER SECTION
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawEqualizer()
{
	if (!this->_beginGlassCard("##eq_card")) { this->_endGlassCard(); return; }
	this->_sectionHeader("Equalizer Presets", "DSP");

	static const struct { const char* name; int val; } presets[] = {
		{ "Off", 0x00 }, { "Bright", 0x10 }, { "Excited", 0x11 }, { "Mellow", 0x12 }, { "Relaxed", 0x13 },
		{ "Vocal", 0x14 }, { "Treble", 0x15 }, { "Bass", 0x16 }, { "Speech", 0x17 }, { "Manual", 0xA0 }
	};

	float cardW = ImGui::GetContentRegionAvail().x;
	float cellW = (cardW - 4.f * 4.f) / 5.f;

	for (int i = 0; i < 10; ++i)
	{
		if (i % 5) ImGui::SameLine(0, 4.f);
		bool sel = this->_uiEqPreset == presets[i].val;
		if (_drawPillButton(presets[i].name, sel, ImVec2(cellW, 32)))
		{
			this->_uiEqPreset = presets[i].val;
			int p = presets[i].val;
			_showToast(std::string("Equalizer: ") + presets[i].name);
			this->_sendFeatureCommand([this, p]() { this->_headphones.setEqualizerPreset((EQ_PRESET)p); });
		}
		_drawTooltip("Select Equalizer preset");
	}

	// Manual EQ visualization & sliders
	if (this->_uiEqPreset == 0xA0)
	{
		ImGui::Spacing(); ImGui::Spacing();
		const char* bandLabels[] = { "ClearBass", "400 Hz", "1 kHz", "2.5 kHz", "6.3 kHz", "16 kHz" };
		int* values[] = { &this->_uiClearBass, &this->_uiEqBands[0], &this->_uiEqBands[1],
						  &this->_uiEqBands[2], &this->_uiEqBands[3], &this->_uiEqBands[4] };
		bool changed = false;

		float barAreaW = ImGui::GetContentRegionAvail().x;
		float barW = (barAreaW - 5.f * 10.f) / 6.f;
		float barH = 110.f;
		ImVec2 startPos = ImGui::GetCursorScreenPos();

		for (int i = 0; i < 6; ++i)
		{
			float x = startPos.x + i * (barW + 10.f);
			float normalVal = (float)(*values[i]) / 10.f;
			_drawEqSpectrumBar(x, startPos.y, barW, barH, normalVal, bandLabels[i], i == 0);
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + barH + 24.f);
		for (int i = 0; i < 6; ++i)
		{
			std::string id = "##eqband_sl_" + std::to_string(i);
			char fmt[64];
			if (i == 0) {
				int dbVal = (*values[0] - 5) * 2;
				snprintf(fmt, sizeof(fmt), "ClearBass  %+d dB", dbVal);
			} else {
				int dbVal = (int)roundf((*values[i] - 5) * 1.2f);
				snprintf(fmt, sizeof(fmt), "%s  %+d dB", bandLabels[i], dbVal);
			}

			if (_drawGradientSlider(id.c_str(), values[i], 0, 10, fmt))
				changed = true;
			_drawTooltip(i == 0 ? "Adjust Sony sub-bass boost (-10 to +10 dB)" : "Adjust frequency band gain (-6 to +6 dB)");
		}

		if (changed)
		{
			std::vector<int> bands(this->_uiEqBands.begin(), this->_uiEqBands.end());
			int cb = this->_uiClearBass;
			this->_sendFeatureCommand([this, cb, bands]() { this->_headphones.setEqualizerCustom(cb, bands); });
		}
	}

	this->_endGlassCard();
}

// ═══════════════════════════════════════════════════════════════════════════════
// DSEE UPSCALING
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawDseeSection()
{
	if (!this->_beginGlassCard("##dsee_card")) { this->_endGlassCard(); return; }
	this->_sectionHeader("DSEE Audio Enhancement", "UPSCALING");

	if (_drawToggle("Enable DSEE", &this->_uiDsee, "Sony's digital sound enhancement for compressed music"))
	{
		bool v = this->_uiDsee;
		_showToast(v ? "DSEE Audio Upscaling Enabled" : "DSEE Disabled");
		this->_sendFeatureCommand([this, v]() { this->_headphones.setDsee(v); });
	}
	_drawTooltip("Restores high-frequency spectrum lost in compressed MP3/AAC audio files");

	this->_endGlassCard();
}

// ═══════════════════════════════════════════════════════════════════════════════
// OPTIONAL FEATURES
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawOptionalFeatures()
{
	const bool any = this->_headphones.hasAutoPowerOff() || this->_headphones.hasSpeakToChat() ||
					 this->_headphones.hasAdaptiveVolume();
	if (!any)
		return;

	if (!this->_beginGlassCard("##features_card")) { this->_endGlassCard(); return; }
	this->_sectionHeader("Smart Features", "AUTOMATION");

	if (this->_headphones.hasAutoPowerOff())
	{
		ImGui::TextColored(COLOR_TEXT_SUB, "Auto Power-Off Timer");
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.10f, 0.10f, 0.15f, 0.96f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);
		if (ImGui::Combo("##apo_combo", &this->_uiAutoPowerOff,
			"Off\0After 5 minutes\0After 30 minutes\0After 1 hour\0After 3 hours\0When taken off\0\0"))
		{
			int idx = this->_uiAutoPowerOff;
			_showToast("Auto Power-Off updated");
			this->_sendFeatureCommand([this, idx]() { this->_headphones.setAutoPowerOff(idx); });
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		_drawTooltip("Set inactive power-off timer to conserve battery");
		ImGui::Spacing();
	}

	if (this->_headphones.hasSpeakToChat())
	{
		if (_drawToggle("Speak-to-Chat", &this->_uiSpeakToChat, "Automatically pauses music when you start talking"))
		{
			bool v = this->_uiSpeakToChat;
			_showToast(v ? "Speak-to-Chat Enabled" : "Speak-to-Chat Disabled");
			this->_sendFeatureCommand([this, v]() { this->_headphones.setSpeakToChat(v); });
		}
		_drawTooltip("Detects your voice and pauses music automatically");
	}

	if (this->_headphones.hasAdaptiveVolume())
	{
		if (_drawToggle("Adaptive Volume Control", &this->_uiAdaptiveVolume, "Adjusts volume dynamically based on ambient noise"))
		{
			bool v = this->_uiAdaptiveVolume;
			_showToast(v ? "Adaptive Volume Enabled" : "Adaptive Volume Disabled");
			this->_sendFeatureCommand([this, v]() { this->_headphones.setAdaptiveVolume(v); });
		}
		_drawTooltip("Automatically adjusts headphone volume based on room noise levels");
	}

	this->_endGlassCard();
}

// ═══════════════════════════════════════════════════════════════════════════════
// SURROUND SOUND (V1 Devices)
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_drawSurroundControls()
{
	if (!this->_beginGlassCard("##surround_card")) { this->_endGlassCard(); return; }
	this->_sectionHeader("Virtual Soundstage", "VPT");

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.10f, 0.10f, 0.15f, 0.96f));
	if (ImGui::Combo("##pos_combo", &this->_uiSoundPosition,
		"Off\0Front Left\0Front Right\0Front\0Rear Left\0Rear Right\0\0"))
		this->_uiVptType = 0;

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::Combo("##vpt_combo", &this->_uiVptType,
		"Off\0Outdoor Festival\0Arena\0Concert Hall\0Club\0\0"))
		this->_uiSoundPosition = 0;
	ImGui::PopStyleColor();

	this->_headphones.setSurroundPosition(SOUND_POSITION_PRESET_ARRAY[this->_uiSoundPosition]);
	this->_headphones.setVptType(this->_uiVptType);

	this->_endGlassCard();
}

// ═══════════════════════════════════════════════════════════════════════════════
// ASYNC COMMAND DISPATCHER
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_sendPendingASMChanges()
{
	if (this->_featureCommandFuture.valid() && this->_featureCommandFuture.ready())
	{
		try { this->_featureCommandFuture.get(); }
		catch (const RecoverableException& e) { if (e.shouldDisconnect) this->_bt.disconnect(); this->_mq.addMessage(e.what()); }
		catch (const std::exception& e) { this->_mq.addMessage(e.what()); }
	}

	if (this->_sendCommandFuture.valid() && this->_sendCommandFuture.ready())
	{
		try { this->_sendCommandFuture.get(); }
		catch (const RecoverableException& exc) {
			std::string prefix;
			if (exc.shouldDisconnect) { this->_bt.disconnect(); prefix = "Disconnected due to: "; }
			this->_mq.addMessage(prefix + exc.what());
		}
		catch (const std::exception& e) { this->_mq.addMessage(e.what()); }
	}
	else if (!this->_sendCommandFuture.valid() && this->_headphones.isChanged())
	{
		this->_sendCommandFuture.setFromAsync([this]() { this->_headphones.setChanges(); });
	}
}

// ═══════════════════════════════════════════════════════════════════════════════
// GLASS CONTAINER HELPERS
// ═══════════════════════════════════════════════════════════════════════════════

bool CrossPlatformGUI::_beginGlassCard(const char* id, float height)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, COLOR_GLASS_BG);
	ImGui::PushStyleColor(ImGuiCol_Border, COLOR_GLASS_BORDER);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18, 16));

	bool ret = ImGui::BeginChild(id, ImVec2(0.f, height),
		ImGuiChildFlags_Borders | (height == 0.f ? ImGuiChildFlags_AutoResizeY : 0) | ImGuiChildFlags_AlwaysUseWindowPadding);

	return ret;
}

void CrossPlatformGUI::_endGlassCard()
{
	ImGui::EndChild();
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(2);
	ImGui::Spacing();
}

void CrossPlatformGUI::_sectionHeader(const char* title, const char* badgeText)
{
	ImGui::PushStyleColor(ImGuiCol_Text, SONY_ORANGE);
	ImGui::TextUnformatted(title);
	ImGui::PopStyleColor();

	if (badgeText)
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MUTED);
		ImGui::Text("  [ %s ]", badgeText);
		ImGui::PopStyleColor();
	}
	ImGui::Spacing();
}

// ═══════════════════════════════════════════════════════════════════════════════
// CUSTOM WIDGETS
// ═══════════════════════════════════════════════════════════════════════════════

bool CrossPlatformGUI::_drawToggle(const char* label, bool* v, const char* subtitle)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 p = ImGui::GetCursorScreenPos();
	float h = 24.f;
	float w = 46.f;
	float r = h * 0.5f;
	bool changed = false;

	ImGui::InvisibleButton(label, ImVec2(ImGui::GetContentRegionAvail().x, h + (subtitle ? 14.f : 0.f)));
	if (ImGui::IsItemClicked())
	{
		*v = !*v;
		changed = true;
	}

	ImU32 bgCol = *v ? IM_COL32(255, 107, 53, 240) : IM_COL32(40, 42, 54, 220);
	ImU32 knobCol = IM_COL32(245, 245, 250, 255);

	// Toggle track
	dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), bgCol, r);

	// Glow on active
	if (*v)
		dl->AddRectFilled(p, ImVec2(p.x + w, p.y + h), IM_COL32(255, 107, 53, 50), r);

	// Smooth knob position
	float knobX = *v ? p.x + w - r : p.x + r;
	dl->AddCircleFilled(ImVec2(knobX, p.y + r), r - 3.f, knobCol);

	// Title
	ImGui::SameLine();
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + w + 12.f, ImGui::GetCursorPosY() - h + 3.f));
	ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MAIN);
	ImGui::Text("%s", label);
	ImGui::PopStyleColor();

	if (subtitle)
	{
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + w + 12.f, ImGui::GetCursorPosY() - 2.f));
		ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_MUTED);
		ImGui::Text("%s", subtitle);
		ImGui::PopStyleColor();
	}

	return changed;
}

bool CrossPlatformGUI::_drawPillButton(const char* label, bool selected, ImVec2 size)
{
	ImVec4 bg = selected ? ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, 0.28f)
						 : ImVec4(0.14f, 0.15f, 0.20f, 0.90f);
	ImVec4 bgHov = selected ? ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, 0.38f)
							: ImVec4(0.20f, 0.21f, 0.28f, 0.95f);
	ImVec4 border = selected ? ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, 0.70f)
							 : COLOR_GLASS_BORDER;
	ImVec4 textCol = selected ? SONY_ORANGE : COLOR_TEXT_MAIN;

	ImGui::PushStyleColor(ImGuiCol_Button, bg);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHov);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, 0.50f));
	ImGui::PushStyleColor(ImGuiCol_Text, textCol);
	ImGui::PushStyleColor(ImGuiCol_Border, border);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, selected ? 1.5f : 0.5f);

	bool pressed = ImGui::Button(label, size);

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(5);
	return pressed;
}

bool CrossPlatformGUI::_drawGradientSlider(const char* id, int* v, int vMin, int vMax, const char* fmt)
{
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.13f, 0.18f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.16f, 0.17f, 0.23f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.18f, 0.19f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, SONY_ORANGE);
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0.55f, 0.32f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 10.f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 18.f);

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	bool changed = ImGui::SliderInt(id, v, vMin, vMax, fmt);

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(5);
	return changed;
}

void CrossPlatformGUI::_drawBatteryRing(float cx, float cy, float radius, float percent, bool charging, const char* label)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	int segments = 48;
	float thickness = 4.5f;
	float startAngle = -IM_PI * 0.75f;
	float totalAngle = IM_PI * 1.5f;

	// Background track ring
	for (int i = 0; i < segments; ++i)
	{
		float a0 = startAngle + (float)i / segments * totalAngle;
		float a1 = startAngle + (float)(i + 1) / segments * totalAngle;
		dl->AddLine(
			ImVec2(cx + cosf(a0) * radius, cy + sinf(a0) * radius),
			ImVec2(cx + cosf(a1) * radius, cy + sinf(a1) * radius),
			IM_COL32(35, 38, 50, 180), thickness);
	}

	// Active progress ring
	int filledSegs = (int)(segments * percent);
	for (int i = 0; i < filledSegs; ++i)
	{
		float a0 = startAngle + (float)i / segments * totalAngle;
		float a1 = startAngle + (float)(i + 1) / segments * totalAngle;

		ImU32 col;
		if (percent > 0.5f)
			col = IM_COL32(0, 217, 187, 240);   // Emerald
		else if (percent > 0.2f)
			col = IM_COL32(255, 170, 53, 240);   // Amber
		else
			col = IM_COL32(255, 70, 60, 240);    // Red

		dl->AddLine(
			ImVec2(cx + cosf(a0) * radius, cy + sinf(a0) * radius),
			ImVec2(cx + cosf(a1) * radius, cy + sinf(a1) * radius),
			col, thickness);
	}

	// Percentage text center
	char buf[8];
	snprintf(buf, sizeof(buf), "%d%%", (int)(percent * 100.f));
	ImVec2 textSize = ImGui::CalcTextSize(buf);
	dl->AddText(ImVec2(cx - textSize.x * 0.5f, cy - textSize.y * 0.5f),
		IM_COL32(235, 238, 245, 240), buf);

	// Optional Sub-label under ring
	if (label)
	{
		ImVec2 lSize = ImGui::CalcTextSize(label);
		dl->AddText(ImVec2(cx - lSize.x * 0.5f, cy + radius + 4.f),
			IM_COL32(140, 145, 160, 200), label);
	}
}

void CrossPlatformGUI::_drawEqSpectrumBar(float x, float y, float w, float h, float normalizedVal, const char* label, bool isBass)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	float barH = h * (std::max)(0.04f, normalizedVal);

	// Card slot background
	dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h),
		IM_COL32(20, 22, 30, 220), 6.f);

	// Filled gradient bar
	if (barH > 0)
	{
		ImU32 topCol = isBass ? IM_COL32(255, 107, 53, 230) : IM_COL32(0, 168, 232, 230);
		ImU32 botCol = isBass ? IM_COL32(255, 107, 53, 60)  : IM_COL32(0, 168, 232, 60);

		dl->AddRectFilledMultiColor(ImVec2(x, y + h - barH), ImVec2(x + w, y + h),
			topCol, topCol, botCol, botCol);

		// Glowing top cap
		dl->AddRectFilled(ImVec2(x, y + h - barH), ImVec2(x + w, y + h - barH + 3.f),
			IM_COL32(255, 255, 255, 220), 2.f);
	}

	// Label below bar
	ImVec2 labelSize = ImGui::CalcTextSize(label);
	dl->AddText(ImVec2(x + (w - labelSize.x) * 0.5f, y + h + 6.f),
		IM_COL32(150, 155, 170, 220), label);
}

// ═══════════════════════════════════════════════════════════════════════════════
// THEME SETUP
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_applyGlassTheme()
{
	ImGui::StyleColorsDark();
	ImGuiStyle& s = ImGui::GetStyle();

	s.WindowRounding    = 0.f;
	s.ChildRounding     = 16.f;
	s.FrameRounding     = 10.f;
	s.GrabRounding      = 10.f;
	s.PopupRounding     = 12.f;
	s.ScrollbarRounding = 10.f;
	s.TabRounding       = 8.f;

	s.WindowPadding     = ImVec2(0, 0);
	s.FramePadding      = ImVec2(14, 8);
	s.ItemSpacing       = ImVec2(10, 8);
	s.ItemInnerSpacing  = ImVec2(8, 6);
	s.ChildBorderSize   = 1.f;
	s.ScrollbarSize     = 6.f;
	s.WindowBorderSize  = 0.f;

	ImVec4* c = s.Colors;
	c[ImGuiCol_WindowBg]         = COLOR_CANVAS;
	c[ImGuiCol_ChildBg]          = COLOR_GLASS_BG;
	c[ImGuiCol_PopupBg]          = ImVec4(0.09f, 0.10f, 0.15f, 0.96f);
	c[ImGuiCol_Border]           = COLOR_GLASS_BORDER;
	c[ImGuiCol_FrameBg]          = ImVec4(0.12f, 0.13f, 0.18f, 1.00f);
	c[ImGuiCol_FrameBgHovered]   = ImVec4(0.16f, 0.17f, 0.23f, 1.00f);
	c[ImGuiCol_FrameBgActive]    = ImVec4(0.18f, 0.19f, 0.26f, 1.00f);
	c[ImGuiCol_Text]             = COLOR_TEXT_MAIN;
	c[ImGuiCol_TextDisabled]     = COLOR_TEXT_MUTED;
	c[ImGuiCol_Button]           = ImVec4(0.14f, 0.15f, 0.20f, 0.90f);
	c[ImGuiCol_ButtonHovered]    = ImVec4(0.20f, 0.21f, 0.28f, 0.95f);
	c[ImGuiCol_ButtonActive]     = ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, 0.40f);
	c[ImGuiCol_CheckMark]        = SONY_ORANGE;
	c[ImGuiCol_SliderGrab]       = SONY_ORANGE;
	c[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.55f, 0.32f, 1.0f);
	c[ImGuiCol_Header]           = ImVec4(0.14f, 0.15f, 0.20f, 0.90f);
	c[ImGuiCol_HeaderHovered]    = ImVec4(0.18f, 0.19f, 0.26f, 0.95f);
	c[ImGuiCol_HeaderActive]     = ImVec4(SONY_ORANGE.x, SONY_ORANGE.y, SONY_ORANGE.z, 0.30f);
	c[ImGuiCol_Separator]        = ImVec4(1.f, 1.f, 1.f, 0.05f);
	c[ImGuiCol_ScrollbarBg]      = ImVec4(0.f, 0.f, 0.f, 0.f);
	c[ImGuiCol_ScrollbarGrab]    = ImVec4(0.3f, 0.3f, 0.38f, 0.5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTRUCTOR
// ═══════════════════════════════════════════════════════════════════════════════

CrossPlatformGUI::CrossPlatformGUI(BluetoothWrapper bt) : _bt(std::move(bt)), _headphones(_bt)
{
	this->_applyGlassTheme();

	ImGuiIO& io = ImGui::GetIO();
	this->_mq = TimedMessageQueue(GUI_MAX_MESSAGES);
	this->_connectedDevicesFuture.setFromAsync([this]() { return this->_bt.getConnectedDevices(); });

	io.IniFilename = nullptr;
	io.WantSaveIniSettings = false;

	// Load Embedded Cascadia Code Font
	char* fileData = new char[sizeof(CascadiaCodeTTF)];
	memcpy(fileData, CascadiaCodeTTF, sizeof(CascadiaCodeTTF));
	ImFont* font = io.Fonts->AddFontFromMemoryTTF(reinterpret_cast<void*>(fileData), sizeof(CascadiaCodeTTF), FONT_SIZE_BODY);
	IM_ASSERT(font != NULL);

	auto lastDev = _loadLastDevice();
	_lastDeviceMac = lastDev.first;
	_lastDeviceName = lastDev.second;
}

// ═══════════════════════════════════════════════════════════════════════════════
// UX POLISH HELPERS (TOASTS, TOOLTIPS, DEVICE MEMORY)
// ═══════════════════════════════════════════════════════════════════════════════

void CrossPlatformGUI::_showToast(const std::string& text)
{
	_toasts.push_back({ text, 2.5f });
}

void CrossPlatformGUI::_drawToasts()
{
	if (_toasts.empty()) return;

	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* dl = ImGui::GetForegroundDrawList();

	float yOffset = io.DisplaySize.y - 70.f;

	for (auto it = _toasts.begin(); it != _toasts.end(); )
	{
		it->timeRemaining -= io.DeltaTime;
		if (it->timeRemaining <= 0.f)
		{
			it = _toasts.erase(it);
			continue;
		}

		float alpha = (std::min)(1.0f, it->timeRemaining / 0.5f);
		ImVec2 textSize = ImGui::CalcTextSize(it->text.c_str());
		float boxW = textSize.x + 32.f;
		float boxH = 34.f;
		float boxX = (io.DisplaySize.x - boxW) * 0.5f;

		ImVec2 p0(boxX, yOffset);
		ImVec2 p1(boxX + boxW, yOffset + boxH);

		dl->AddRectFilled(p0, p1, IM_COL32(14, 16, 24, (int)(235 * alpha)), 10.f);
		dl->AddRect(p0, p1, IM_COL32(255, 107, 53, (int)(200 * alpha)), 10.f, 0, 1.5f);
		dl->AddText(ImVec2(boxX + 16.f, yOffset + 8.f), IM_COL32(245, 245, 250, (int)(255 * alpha)), it->text.c_str());

		yOffset -= 40.f;
		++it;
	}
}

void CrossPlatformGUI::_drawTooltip(const char* text)
{
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
	{
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.08f, 0.09f, 0.13f, 0.96f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 0.12f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 8));
		ImGui::SetTooltip("%s", text);
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
	}
}

void CrossPlatformGUI::_saveLastDevice(const std::string& name, const std::string& mac)
{
	std::string path = _resourceBase() + "/last_device.ini";
	FILE* f = fopen(path.c_str(), "w");
	if (f)
	{
		fprintf(f, "%s\n%s\n", mac.c_str(), name.c_str());
		fclose(f);
	}
}

std::pair<std::string, std::string> CrossPlatformGUI::_loadLastDevice()
{
	std::string path = _resourceBase() + "/last_device.ini";
	FILE* f = fopen(path.c_str(), "r");
	if (!f) return { "", "" };

	char mac[64] = { 0 };
	char name[128] = { 0 };
	if (fgets(mac, sizeof(mac), f))
	{
		mac[strcspn(mac, "\r\n")] = 0;
	}
	if (fgets(name, sizeof(name), f))
	{
		name[strcspn(name, "\r\n")] = 0;
	}
	fclose(f);
	return { mac, name };
}
