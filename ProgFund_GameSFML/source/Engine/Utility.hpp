#pragma once

#include "../../pch.h"

class EngineError {
public:
	EngineError() {}
	EngineError(const std::string& msg) : message_(msg) {}

	const std::string& GetError() { return message_; }
	const char* what() { return message_.c_str(); }
protected:
	std::string message_;
};

class GLColor {
public:
	GLColor() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
	GLColor(float _r, float _g, float _b) : r(_r), g(_g), b(_b), a(1.0f) {}
	GLColor(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
public:
	float r;
	float g;
	float b;
	float a;
};

constexpr double GM_PI = 3.14159265358979323846;
constexpr double GM_PI_X2 = GM_PI * 2.0;
constexpr double GM_PI_X4 = GM_PI * 4.0;
constexpr double GM_PI_2 = GM_PI / 2.0;
constexpr double GM_PI_4 = GM_PI / 4.0;
constexpr double GM_1_PI = 1.0 / GM_PI;
constexpr double GM_2_PI = 2.0 / GM_PI;
constexpr double GM_SQRTP = 1.772453850905516027298;
constexpr double GM_1_SQRTP = 1.0 / GM_SQRTP;
constexpr double GM_2_SQRTP = 2.0 / GM_SQRTP;
constexpr double GM_SQRT2 = 1.41421356237309504880;
constexpr double GM_SQRT2_2 = GM_SQRT2 / 2.0;
constexpr double GM_SQRT2_X2 = GM_SQRT2 * 2.0;

enum class BlendMode : uint8_t {
	Alpha,
	Add,
	Subtract,
	RevSubtract
};