#include "pch.h"
#include "Window.hpp"

//*******************************************************************
//WindowMain
//*******************************************************************
WindowMain* WindowMain::base_ = nullptr;
WindowMain::WindowMain() {
	glContext_ = nullptr;
	window_ = nullptr;

	matView_ = XMMatrixIdentity();
	matProjection_ = XMMatrixIdentity();

	backBufferTexture_ = 0;
	backBufferDepth_ = 0;

	fps_ = 0;
}
WindowMain::~WindowMain() {
}

void WindowMain::Initialize() {
	if (base_) throw EngineError("WindowMain already initialized.");
	base_ = this;

	glContext_ = new sf::ContextSettings;
	glContext_->depthBits = 24;
	glContext_->majorVersion = 3;
	glContext_->minorVersion = 3;

	window_ = new sf::RenderWindow(sf::VideoMode(640, 480, 32), "Re: Mystic Square",
		sf::Style::Titlebar | sf::Style::Close, *glContext_);
	window_->setVerticalSyncEnabled(true);

	window_->setActive(true);

	//Enables blending
	glEnable(GL_BLEND);

	//Enables textures
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	//Enables the Z-buffer
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);

	glAlphaFunc(GL_LEQUAL, 0);
	glDepthFunc(GL_LEQUAL);

	SetBlendMode(BlendMode::Alpha);
	SetViewPort(0, 0, 640, 480, 10.0f, 2000.0f);

	{
		glGenFramebuffers(1, &backBufferFrameBuffer_);
		glBindFramebuffer(GL_FRAMEBUFFER, backBufferFrameBuffer_);

		glGenTextures(1, &backBufferTexture_);
		glBindTexture(GL_TEXTURE_2D, backBufferTexture_);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glGenRenderbuffers(1, &backBufferDepth_);
		glBindRenderbuffer(GL_RENDERBUFFER, backBufferDepth_);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 640, 480);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, backBufferDepth_);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, backBufferTexture_, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
	}

	window_->setActive(true);
}
void WindowMain::Release() {
	ptr_delete(glContext_);

	window_->close();
	ptr_delete(window_);
}

void WindowMain::BeginScene(GLColor clearColor) {
	glClearDepth(1.0f);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void WindowMain::EndScene(bool bPresent) {
	if (bPresent)
		window_->display();
}

void WindowMain::SetBlendMode(BlendMode mode) {
	switch (mode) {
	case BlendMode::Add:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendMode::Subtract:
		glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendMode::RevSubtract:
		glBlendEquationSeparate(GL_FUNC_SUBTRACT, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendMode::Invert:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendMode::Alpha:
	default:
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
}
void WindowMain::SetViewPort(float x, float y, float w, float h, float zn, float zf) {
	float r = x + w;
	float b = y + h;
	float zn2 = 2.0f * zn;
	float zd = zf - zn;
	matProjection_.r[0] = XMVectorSet(zn2 / w, 0.0f, (x + r) / w, 0.0f);
	matProjection_.r[1] = XMVectorSet(0.0f, zn2 / h, (y + b) / h, 0.0f);
	matProjection_.r[2] = XMVectorSet(0.0f, 0.0f, -(zf + zn) / zd, -(zf * zn2) / zd);
	matProjection_.r[3] = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	glViewport(x, y, w, h);
	glDepthRange(zn, zf);
}