#pragma once

#include "../../pch.h"

#include "Utility.hpp"

class WindowMain {
	static WindowMain* base_;
public:
	WindowMain();
	~WindowMain();

	static WindowMain* const GetBase() { return base_; }

	void Initialize();
	void Release();

	void BeginScene(GLColor clearColor = GLColor(0.0f, 0.0f, 0.0f));
	void EndScene(bool bPresent = true);

	sf::ContextSettings* const GetDeviceContext() { return glContext_; }
	sf::RenderWindow* const GetWindow() { return window_; }

	const GLuint GetBackBufferTexture() { return backBufferTexture_; }
	const GLuint GetBackBufferDepth() { return backBufferDepth_; }

	void SetFPS(float fps) { fps_ = fps; }
	float GetFPS(float fps) { return fps_; }

	void SetBlendMode(BlendMode mode);

	void SetViewPort(float x, float y, float w, float h, float zn = 0.0f, float zf = 1.0f);

	XMMATRIX* GetViewMatrix() { return &matView_; }
	XMMATRIX* GetProjectionMatrix() { return &matProjection_; }
private:
	sf::ContextSettings* glContext_;
	sf::RenderWindow* window_;

	XMMATRIX matView_;
	XMMATRIX matProjection_;

	GLuint backBufferFrameBuffer_;
	GLuint backBufferTexture_;
	GLuint backBufferDepth_;

	float fps_;
};