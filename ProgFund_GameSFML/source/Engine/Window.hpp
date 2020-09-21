#pragma once

#include "../../pch.h"

#include "Utility.hpp"
#include "Vertex.hpp"

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
public:
	WindowMain();
	~WindowMain();

	static WindowMain* const GetBase() { return base_; }

	void Initialize(HINSTANCE hInst);
	void Release();

	void BeginScene(D3DCOLOR clearColor = 0xff000000);
	void EndScene(bool bPresent = true);

	HWND GetHandle() { return hWnd_; }

	IDirect3DDevice9* const GetDevice() { return pDevice_; }

	IDirect3DSurface9* const GetBackBuffer() { return pBackBuffer_; }
	IDirect3DSurface9* const GetZBuffer() { return pZBuffer_; }

	void SetFPS(float fps) { fps_ = fps; }
	float GetFPS(float fps) { return fps_; }

	void SetBlendMode(BlendMode mode);

	void SetViewPort(float x, float y, float w, float h, float zn = 0.0f, float zf = 1.0f);

	XMMATRIX* GetViewMatrix() { return &matView_; }
	XMMATRIX* GetProjectionMatrix() { return &matProjection_; }
	XMMATRIX* GetViewportMatrix() { return &matViewport_; }

	void SetZBufferMode(bool bWrite, bool bUse);
	void SetTextureFilter(D3DTEXTUREFILTERTYPE min, D3DTEXTUREFILTERTYPE mag, D3DTEXTUREFILTERTYPE mip = D3DTEXF_LINEAR);
private:
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	HINSTANCE hInst_;
	HWND hWnd_;

	IDirect3D9* pDirect3D_;
	IDirect3DDevice9* pDevice_;

	IDirect3DSurface9* pBackBuffer_;
	IDirect3DSurface9* pZBuffer_;

	VertexDeclarationManager* vertexManager_;

	XMMATRIX matView_;
	XMMATRIX matProjection_;
	XMMATRIX matViewport_;

	BlendMode previousBlendMode_;

	float fps_;
};