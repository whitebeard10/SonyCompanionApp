#pragma once

#include <stdexcept>

// The inclusion order is critical here. Do not change it.
#include "BluetoothWrapper.h"
#include "WindowsBluetoothConnector.h"

#include "Constants.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

#include <string>

#include "Windows.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include "CrossPlatformGUI.h"

#ifdef APP_NAME
#undef APP_NAME
#endif
#ifdef APP_NAME_W
#undef APP_NAME_W
#endif

static constexpr auto SONY_APP_NAME = "Sony Companion";
static constexpr auto SONY_APP_NAME_W = L"Sony Companion";

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void EnterGUIMainLoop(BluetoothWrapper bt);
void DisplayErrorMessagebox(const std::string& message);
void* CreateD3D11TextureFromRGBA(const unsigned char* pixels, int width, int height);

namespace WindowsGUIInternal
{
	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}
