#pragma once

#include "../../pch.h"

#include "DxConstant.hpp"

#include "Utility.hpp"
#include "Vertex.hpp"

enum class WindowMode : uint8_t {
	Windowed,
	Fullscreen,
};
enum class BlendMode : uint8_t {
	Alpha,
	Add,
	Subtract,
	RevSubtract,
	Invert,
};
enum class TextureSample : uint8_t {
	LinearLinear,
	LinearNearest,
	NearestLinear,
	NearestNearest,
};
class WindowMain {
	static WindowMain* base_;
private:
	HINSTANCE hInst_;
	HWND hWnd_;

	HANDLE hTimerQueue_;

	D3DPRESENT_PARAMETERS presentParamWind_;
	D3DPRESENT_PARAMETERS presentParamFull_;

	IDirect3D9* pDirect3D_;
	IDirect3DDevice9* pDevice_;

	IDirect3DSurface9* pBackBuffer_;
	IDirect3DSurface9* pZBuffer_;

	VertexBufferManager* vertexManager_;

	D3DXMATRIX matView_;
	D3DXMATRIX matProjection_;
	D3DXMATRIX matViewport_;

	WindowMode windowMode_;
	BlendMode previousBlendMode_;

	std::list<DxResourceManagerBase*> listResourceManager_;

	float fps_;

	void _ResetDeviceState();
	void _ReleaseDxResource();
	void _RestoreDxResource();
	bool _TestDeviceCooperation();

	static LRESULT CALLBACK _StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
public:
	WindowMain();
	~WindowMain();

	static WindowMain* const GetBase() { return base_; }

	void Initialize(HINSTANCE hInst);
	void Release();

	void BeginScene(D3DCOLOR clearColor = 0xff000022);
	void EndScene(bool bPresent = true);

	HWND GetHandle() { return hWnd_; }

	HANDLE GetTimerQueue() { return hTimerQueue_; }

	IDirect3DDevice9* const GetDevice() { return pDevice_; }

	IDirect3DSurface9* const GetBackBuffer() { return pBackBuffer_; }
	IDirect3DSurface9* const GetZBuffer() { return pZBuffer_; }

	VertexBufferManager* GetVertexManager() { return vertexManager_; }

	void AddDxResourceListener(DxResourceManagerBase* object);
	void RemoveDxResourceListener(DxResourceManagerBase* object);

	void SetFPS(float fps) { fps_ = fps; }
	float GetFPS(float fps) { return fps_; }

	void SetBlendMode(BlendMode mode);
	WindowMode GetWindowMode() { return windowMode_; }

	void SetViewPort(float x, float y, float w, float h, float zn = 0.0f, float zf = 1.0f);

	D3DXMATRIX* GetViewMatrix() { return &matView_; }
	D3DXMATRIX* GetProjectionMatrix() { return &matProjection_; }
	D3DXMATRIX* GetViewportMatrix() { return &matViewport_; }

	void SetZBufferMode(bool bWrite, bool bUse);
	void SetTextureFilter(D3DTEXTUREFILTERTYPE min, D3DTEXTUREFILTERTYPE mag, D3DTEXTUREFILTERTYPE mip = D3DTEXF_LINEAR);
};