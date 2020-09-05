#pragma once

#include "../../pch.h"

#include "Utility.hpp"

class WindowMain {
	static WindowMain* base_;
public:
	WindowMain();
	~WindowMain();

	void Initialize();
	void Release();

	void BeginScene(GLColor clearColor = GLColor(0.0f, 0.11f, 0.87f));
	void EndScene(bool bPresent = true);

	sf::ContextSettings* const GetDeviceContext() { return glContext_; }
	sf::RenderWindow* const GetWindow() { return window_; }

	const GLuint GetBackBufferTexture() { return backBufferTexture_; }
	const GLuint GetBackBufferDepth() { return backBufferDepth_; }

	void SetFPS(float fps) { fps_ = fps; }
	float GetFPS(float fps) { return fps_; }

	void SetBlendMode(BlendMode mode);
private:
	sf::ContextSettings* glContext_;
	sf::RenderWindow* window_;

	XMMATRIX matWorld_;
	XMMATRIX matView_;
	XMMATRIX matProjection_;

	GLuint backBufferFrameBuffer_;
	GLuint backBufferTexture_;
	GLuint backBufferDepth_;

	float fps_;
};