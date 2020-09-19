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

static constexpr DWORD GenColorARGB(byte a, byte r, byte g, byte b) {
	return ((DWORD)r << 24) | ((DWORD)g << 16) | ((DWORD)b << 8) | ((DWORD)a);
}
static constexpr DWORD GenColorXRGB(byte r, byte g, byte b) {
	return GenColorARGB(0xff, r, g, b);
}
static constexpr DWORD GenColorRGBA(byte r, byte g, byte b, byte a) {
	return GenColorARGB(a, r, g, b);
}

//Normalized RGBA [0.0~1.0]
class GLColor {
public:
	GLColor() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
	GLColor(float _r, float _g, float _b) : r(_r), g(_g), b(_b), a(1.0f) {}
	GLColor(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
	GLColor(DWORD argb) {
		r = ((argb >> 24) & 0xff) / 255.0f;
		g = ((argb >> 16) & 0xff) / 255.0f;
		b = ((argb >> 8) & 0xff) / 255.0f;
		a = ((argb) & 0xff) / 255.0f;
	}

	operator sf::Color() {
		return sf::Color(r * 255, g * 255, b * 255, a * 255);
	}
	operator DWORD() {
		sf::Color c = this->operator sf::Color();
		GenColorRGBA(c.r, c.g, c.b, c.a);
	}
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
class Math {
public:
	static inline constexpr double DegreeToRadian(double angle) { return angle * GM_PI / 180.0; }
	static inline constexpr double RadianToDegree(double angle) { return angle * 180.0 / GM_PI; }

	template<typename T>
	static inline T HypotSq(const T& x, const T& y) {
		return (x * x + y * y);
	}

	class Lerp {
	public:
		typedef enum : uint8_t {
			LINEAR,
			SMOOTH,
			SMOOTHER,
			ACCELERATE,
			DECELERATE,
		} Type;
	public:
		template<typename T, typename L>
		static inline T Linear(T a, T b, L x) {
			return a + (b - a) * x;
		}
		template<typename T, typename L>
		static inline T Smooth(T a, T b, L x) {
			return a + x * x * ((L)3 - (L)2 * x) * (b - a);
		}
		template<typename T, typename L>
		static inline T Smoother(T a, T b, L x) {
			return a + x * x * x * (x * (x * (L)6 - (L)15) + (L)10) * (b - a);
		}
		template<typename T, typename L>
		static inline T Accelerate(T a, T b, L x) {
			return a + x * x * (b - a);
		}
		template<typename T, typename L>
		static inline T Decelerate(T a, T b, L x) {
			return a + ((L)1 - ((L)1 - x) * ((L)1 - x)) * (b - a);
		}
	};
	class XNAExt {
	public:
		static XMFINLINE XMVECTOR Swizzle(CXMVECTOR V, UINT E0, UINT E1, UINT E2, UINT E3) {
#if defined(_XM_NO_INTRINSICS_)
			return ::XMVectorSwizzle(V, E0, E1, E2, E3);
#else // XM_SSE_INTRINSICS_
			XMASSERT((E0 < 4) && (E1 < 4) && (E2 < 4) && (E3 < 4));
			{
				const float* f = V.m128_f32;
				return _mm_set_ps(f[E3], f[E2], f[E1], f[E0]);
			}
#endif
		}
	};
};

struct GLRect {
	float left;
	float top;
	float right;
	float bottom;

	GLRect() : left(0), top(0), right(0), bottom(0) {}
	GLRect(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
};

enum class BlendMode : uint8_t {
	Alpha,
	Add,
	Subtract,
	RevSubtract,
	Invert,
};

static std::string StringFormat(const char* str, ...) {
	va_list	vl;
	va_start(vl, str);

	//The size returned by _vsnprintf does NOT include null terminator
	int size = _vsnprintf(nullptr, 0U, str, vl);
	std::string res;
	if (size > 0) {
		res.resize(size + 1);
		_vsnprintf((char*)res.c_str(), res.size(), str, vl);
		res.pop_back();	//Don't include the null terminator
	}

	va_end(vl);
	return res;
}

class PathProperty {
public:
	static const std::string& GetModuleDirectory() {
		static std::string moduleDir;
		if (moduleDir.size() == 0) {
			char modulePath[_MAX_PATH];
			ZeroMemory(modulePath, sizeof(modulePath));
			GetModuleFileNameA(NULL, modulePath, _MAX_PATH - 1);
			moduleDir = stdfs::path(modulePath).parent_path().make_preferred().generic_string();
			moduleDir = moduleDir + "/";
		}
		return moduleDir;
	}
	static std::string GetUnique(const std::string& srcPath) {
		std::string p = stdfs::weakly_canonical(srcPath).make_preferred().generic_string();
		return p;
	}
};