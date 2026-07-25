#include "WindowsGUI.h"

/*
 * Sony Companion App — Windows DirectX 11 + Dear ImGui Backend
 * Sleek borderless window with smooth DWM rounded corners and drag handling.
 */

// ── D3D11 State ─────────────────────────────────────────────────────────────
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

void EnterGUIMainLoop(BluetoothWrapper bt)
{
	// Hide console window if attached
	if (HWND consoleHwnd = GetConsoleWindow())
		ShowWindow(consoleHwnd, SW_HIDE);

	// Load application icon (resource ID 101)
	HICON hIconLarge = (HICON)::LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(101), IMAGE_ICON, 256, 256, LR_DEFAULTCOLOR);
	HICON hIconSmall = (HICON)::LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(101), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	if (!hIconLarge) hIconLarge = ::LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCEW(101));
	if (!hIconSmall) hIconSmall = ::LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCEW(101));

	// Register window class with drop shadow style and custom app icon
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_CLASSDC | CS_DROPSHADOW;
	wc.lpfnWndProc = WindowsGUIInternal::WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = hIconLarge;
	wc.hIconSm = hIconSmall;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = SONY_APP_NAME_W;
	::RegisterClassExW(&wc);

	// Sleek borderless pop-up style
	DWORD style = WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE;

	// Center the window on display
	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int screenH = GetSystemMetrics(SM_CYSCREEN);
	int posX = (screenW - GUI_WIDTH) / 2;
	int posY = (screenH - GUI_HEIGHT) / 2;

	HWND hwnd = ::CreateWindowW(wc.lpszClassName, SONY_APP_NAME_W, style,
		posX, posY, GUI_WIDTH, GUI_HEIGHT, NULL, NULL, wc.hInstance, NULL);

	if (!hwnd) {
		throw std::runtime_error("CreateWindowW failed: " + std::to_string(GetLastError()));
	}

	// Apply big and small icons to window handle
	if (hIconLarge) ::SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconLarge);
	if (hIconSmall) ::SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

	// Dark title bar via DWM
	BOOL darkMode = TRUE;
	DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &darkMode, sizeof(darkMode));

	// Smooth rounded corners on Windows 11 (DWMWCP_ROUND = 2)
	int cornerPref = 2; // DWMWCP_ROUND
	DwmSetWindowAttribute(hwnd, 33 /* DWMWA_WINDOW_CORNER_PREFERENCE */, &cornerPref, sizeof(cornerPref));

	// Initialize Direct3D
	if (!WindowsGUIInternal::CreateDeviceD3D(hwnd))
	{
		WindowsGUIInternal::CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		throw std::runtime_error("Failed to create D3D device");
	}

	// Show and bring window to front
	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);
	::SetForegroundWindow(hwnd);
	::BringWindowToTop(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	CrossPlatformGUI gui(std::move(bt));

	UINT presentFlags = 0;

	// ── Main Render Loop ────────────────────────────────────────────────
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		// Start ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		// Run GUI pass — returns false when user clicks close button
		if (!gui.performGUIPass())
		{
			::PostQuitMessage(0);
			break;
		}

		// Render frame
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&COLOR_CANVAS);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present with VSYNC
		if (g_pSwapChain->Present(1, presentFlags) == DXGI_STATUS_OCCLUDED) {
			presentFlags = DXGI_PRESENT_TEST;
			Sleep(MS_PER_FRAME);
		}
		else {
			presentFlags = 0;
		}
	}

	// Cleanup resources
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	WindowsGUIInternal::CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

void DisplayErrorMessagebox(const std::string& message)
{
	MessageBoxA(0, message.c_str(), "Sony Companion | Unrecoverable Error", MB_OK | MB_ICONSTOP);
	exit(GetLastError());
}

// ═══════════════════════════════════════════════════════════════════════════════
// D3D11 Device Management
// ═══════════════════════════════════════════════════════════════════════════════

namespace WindowsGUIInternal
{

bool CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

	HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
		featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice,
		&featureLevel, &g_pd3dDeviceContext);

	if (res != S_OK)
	{
		res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags,
			featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice,
			&featureLevel, &g_pd3dDeviceContext);
	}

	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain)         { g_pSwapChain->Release();         g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext)  { g_pd3dDeviceContext->Release();  g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice)         { g_pd3dDevice->Release();         g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer = nullptr;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (pBackBuffer == nullptr)
		throw std::runtime_error("Unexpected: pBackBuffer is null");
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// ── Window Procedure — handles borderless drag + resize ────────────────────
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;

	// Borderless window drag support — top title bar area
	case WM_NCHITTEST:
	{
		POINT pt = { LOWORD(lParam), HIWORD(lParam) };
		ScreenToClient(hWnd, &pt);
		// Drag area: top TITLE_BAR_HEIGHT (42px) except close/minimize buttons on right
		if (pt.y < (int)TITLE_BAR_HEIGHT && pt.x < (int)(GUI_WIDTH - 64))
			return HTCAPTION;
		break;
	}

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

} // namespace WindowsGUIInternal

void* CreateD3D11TextureFromRGBA(const unsigned char* pixels, int width, int height)
{
	using namespace WindowsGUIInternal;
	if (!g_pd3dDevice || !pixels || width <= 0 || height <= 0)
		return nullptr;

	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels;
	initData.SysMemPitch = width * 4;

	ID3D11Texture2D* pTexture = nullptr;
	HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &initData, &pTexture);
	if (FAILED(hr) || !pTexture)
		return nullptr;

	ID3D11ShaderResourceView* pSRV = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
	pTexture->Release();

	if (FAILED(hr))
		return nullptr;

	return (void*)pSRV;
}
