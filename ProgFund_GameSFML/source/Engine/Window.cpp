#include "pch.h"
#include "Window.hpp"

//*******************************************************************
//WindowMain
//*******************************************************************
WindowMain* WindowMain::base_ = nullptr;
WindowMain::WindowMain() {
	hInst_ = nullptr;
	hWnd_ = nullptr;

	hTimerQueue_ = nullptr;

	pDirect3D_ = nullptr;
	pDevice_ = nullptr;

	pBackBuffer_ = nullptr;
	pZBuffer_ = nullptr;

	vertexManager_ = nullptr;

	D3DXMatrixIdentity(&matView_);
	D3DXMatrixIdentity(&matProjection_);
	D3DXMatrixIdentity(&matViewport_);

	previousBlendMode_ = (BlendMode)0xff;

	fps_ = 0;
}
WindowMain::~WindowMain() {
}

void WindowMain::Initialize(HINSTANCE hInst) {
	if (base_) throw EngineError("WindowMain already initialized.");
	base_ = this;

	hInst_ = hInst;

	hTimerQueue_ = ::CreateTimerQueue();
	if (hTimerQueue_ == nullptr)
		throw EngineError("Failed to create the timer queue object.");

	{
		WNDCLASSEX wcex;
		ZeroMemory(&wcex, sizeof(WNDCLASSEX));
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WindowMain::StaticWndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		//wcex.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_TUTORIAL1);
		wcex.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
		wcex.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)COLOR_WINDOWTEXT;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"DxWindowClass";
		if (!::RegisterClassExW(&wcex))
			throw EngineError("Failed to initialize the window class.");
	}
	
	{
		RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		::AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW - WS_SIZEBOX, false);

		LONG rcWidth = rc.right - rc.left;
		LONG rcHeight = rc.bottom - rc.top;

		RECT rcMonitor;
		::GetWindowRect(::GetDesktopWindow(), &rcMonitor);
		LONG rcLeft = rcMonitor.right / 2L - rcWidth / 2L;
		LONG rcTop = rcMonitor.bottom / 2L - rcHeight / 2L;
		
		hWnd_ = CreateWindowW(L"DxWindowClass", L"Demon World's Ohr Ein Sof",
			WS_OVERLAPPEDWINDOW - WS_SIZEBOX,
			rcLeft, rcTop, rcWidth, rcHeight, nullptr, nullptr, hInst_, nullptr);
		if (!hWnd_)
			throw EngineError("Failed to open a window.");

		::ShowWindow(hWnd_, SW_SHOW);
		::UpdateWindow(hWnd_);

		//ShowCursor(FALSE);
	}

	pDirect3D_ = Direct3DCreate9(D3D_SDK_VERSION);
	if (pDirect3D_ == nullptr) throw EngineError("Direct3DCreate9 error.");

	D3DCAPS9 d3dcaps;
	pDirect3D_->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &d3dcaps);

	{
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
		d3dpp.BackBufferWidth = SCREEN_WIDTH;
		d3dpp.BackBufferHeight = SCREEN_HEIGHT;
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.hDeviceWindow = hWnd_;
		d3dpp.BackBufferCount = 1;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		d3dpp.FullScreen_RefreshRateInHz = 0;

		std::vector<std::pair<D3DDEVTYPE, DWORD>> driverTypes;
		if (d3dcaps.VertexShaderVersion >= D3DVS_VERSION(2, 0) && d3dcaps.PixelShaderVersion >= D3DPS_VERSION(2, 0)) {
			driverTypes.push_back(std::make_pair(D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING));
			driverTypes.push_back(std::make_pair(D3DDEVTYPE_HAL, D3DCREATE_SOFTWARE_VERTEXPROCESSING));
		}
		driverTypes.push_back(std::make_pair(D3DDEVTYPE_REF, D3DCREATE_HARDWARE_VERTEXPROCESSING));
		driverTypes.push_back(std::make_pair(D3DDEVTYPE_REF, D3DCREATE_SOFTWARE_VERTEXPROCESSING));

		HRESULT hr = S_OK;
		for (std::pair<D3DDEVTYPE, DWORD>& iType : driverTypes) {
			hr = pDirect3D_->CreateDevice(D3DADAPTER_DEFAULT, iType.first, hWnd_,
				iType.second | D3DCREATE_FPU_PRESERVE, &d3dpp, &pDevice_);
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			throw EngineError("Failed to create the D3D9 device.");
	}

	pDevice_->GetRenderTarget(0, &pBackBuffer_);
	pDevice_->GetDepthStencilSurface(&pZBuffer_);

	SetBlendMode(BlendMode::Alpha);
	SetViewPort(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f);

	pDevice_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pDevice_->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	pDevice_->SetRenderState(D3DRS_LIGHTING, TRUE);
	pDevice_->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	pDevice_->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_RGBA(192, 192, 192, 0));

	pDevice_->SetRenderState(D3DRS_ALPHATESTENABLE, true);
	pDevice_->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	pDevice_->SetRenderState(D3DRS_ALPHAREF, 0);

	SetZBufferMode(false, false);
	SetTextureFilter(D3DTEXF_LINEAR, D3DTEXF_LINEAR);

	vertexManager_ = new VertexDeclarationManager();
	vertexManager_->Initialize();
}
void WindowMain::Release() {
	::DeleteTimerQueue(hTimerQueue_);

	ptr_release(pBackBuffer_);
	ptr_release(pZBuffer_);
	ptr_release(pDevice_);
	ptr_release(pDirect3D_);
	ptr_delete(vertexManager_);
}

void WindowMain::BeginScene(D3DCOLOR clearColor) {
	pDevice_->Clear(1, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0);
	pDevice_->BeginScene();
}
void WindowMain::EndScene(bool bPresent) {
	pDevice_->EndScene();
	if (bPresent)
		pDevice_->Present(nullptr, nullptr, nullptr, nullptr);
}

void WindowMain::SetBlendMode(BlendMode mode) {
	if (mode == previousBlendMode_) return;
	if (previousBlendMode_ == (BlendMode)0xff) {
		pDevice_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		pDevice_->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		pDevice_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		pDevice_->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		pDevice_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
	}
	previousBlendMode_ = mode;

	pDevice_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pDevice_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

#define SETBLENDOP(op, alp) \
	pDevice_->SetRenderState(D3DRS_BLENDOP, op); \
	pDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, alp);
#define SETBLENDARGS(sbc, dbc, sba, dba) \
	pDevice_->SetRenderState(D3DRS_SRCBLEND, sbc); \
	pDevice_->SetRenderState(D3DRS_DESTBLEND, dbc); \
	pDevice_->SetRenderState(D3DRS_SRCBLENDALPHA, sba); \
	pDevice_->SetRenderState(D3DRS_DESTBLENDALPHA, dba);

	switch (mode) {
	case BlendMode::Add:
		SETBLENDOP(D3DBLENDOP_ADD, TRUE);
		SETBLENDARGS(D3DBLEND_SRCALPHA, D3DBLEND_ONE, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA);
		break;
	case BlendMode::Subtract:
		SETBLENDOP(D3DBLENDOP_REVSUBTRACT, TRUE);
		SETBLENDARGS(D3DBLEND_SRCALPHA, D3DBLEND_ONE, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA);
		break;
	case BlendMode::RevSubtract:
		SETBLENDOP(D3DBLENDOP_SUBTRACT, TRUE);
		SETBLENDARGS(D3DBLEND_SRCALPHA, D3DBLEND_ONE, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA);
		break;
	case BlendMode::Invert:
		SETBLENDOP(D3DBLENDOP_ADD, TRUE);
		SETBLENDARGS(D3DBLEND_INVDESTCOLOR, D3DBLEND_INVSRCCOLOR, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA);
		break;
	case BlendMode::Alpha:
	default:
		SETBLENDOP(D3DBLENDOP_ADD, TRUE);
		SETBLENDARGS(D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA);
		break;
	}

#undef SETBLENDOP
#undef SETBLENDARGS
}
void WindowMain::SetViewPort(float x, float y, float w, float h, float zn, float zf) {
	D3DVIEWPORT9 viewPort;
	ZeroMemory(&viewPort, sizeof(D3DVIEWPORT9));
	viewPort.X = x;
	viewPort.Y = y;
	viewPort.Width = w;
	viewPort.Height = h;
	viewPort.MinZ = 0.0f;
	viewPort.MaxZ = 1.0f;
	pDevice_->SetViewport(&viewPort);
	D3DXMatrixPerspectiveFovLH(&matProjection_, GM_PI_4, w / h, zn, zf);
	{
		D3DXMatrixIdentity(&matViewport_);
		matViewport_._11 = 2.0f / w;
		matViewport_._22 = -2.0f / h;
		matViewport_._33 = -2.0f / (zf - zn);
		matViewport_._41 = -(w + x) / w;
		matViewport_._42 = (h + y) / h;
		matViewport_._43 = -(zf + zn) / (zf - zn);
	}
}
void WindowMain::SetZBufferMode(bool bWrite, bool bUse) {
	pDevice_->SetRenderState(D3DRS_ZENABLE, bUse);
	pDevice_->SetRenderState(D3DRS_ZWRITEENABLE, bWrite);
}
void WindowMain::SetTextureFilter(D3DTEXTUREFILTERTYPE min, D3DTEXTUREFILTERTYPE mag, D3DTEXTUREFILTERTYPE mip) {
	pDevice_->SetSamplerState(0, D3DSAMP_MINFILTER, min);
	pDevice_->SetSamplerState(0, D3DSAMP_MAGFILTER, mag);
	pDevice_->SetSamplerState(0, D3DSAMP_MIPFILTER, mip);
}

LRESULT WindowMain::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(hWnd, &ps);
		::EndPaint(hWnd, &ps);
		break;
	}
	case WM_CLOSE:
		::DestroyWindow(hWnd);
		return FALSE;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return FALSE;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* info = (MINMAXINFO*)lParam;
		int wWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);
		int wHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);

		RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
		::AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW - WS_SIZEBOX, FALSE);

		int width = wr.right - wr.left;
		int height = wr.bottom - wr.top;

		info->ptMaxSize.x = width;
		info->ptMaxSize.y = height;
		return FALSE;
	}
	/*
	case WM_SYSCHAR:
	{
		if (wParam == VK_RETURN)
			this->ChangeScreenMode();
		return FALSE;
	}
	*/
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
	return FALSE;
}