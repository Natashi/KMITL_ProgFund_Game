#pragma once

#include "../../pch.h"

#include "VectorExtensions.hpp"
#include "Table.hpp"

//*******************************************************************
//GUID utilities
//*******************************************************************
class DxGUID {
public:
	DxGUID(const GUID& id) : id_(id) {};

	const GUID& Get() { return id_; }

	bool operator<(const GUID& other) {
		/*
		uint64_t* pL = (uint64_t*)&l;
		uint64_t* pR = (uint64_t*)&r;
		if (pL[0] != pR[0]) return pL[0] < pR[0];
		return pL[1] < pR[1];
		*/
		return memcmp(this, &other, sizeof(GUID)) < 0;
	}
	bool operator==(const GUID& other) {
		return memcmp(this, &other, sizeof(GUID)) == 0;
	}
	operator GUID() { return this->Get(); }
private:
	GUID id_;
};

//*******************************************************************
//Error utilities
//*******************************************************************

class ErrorUtility {
public:
	static std::string StringFromHResult(HRESULT hr, bool bDescription = true) {
		std::string err = DXGetErrorStringA(hr);
		if (bDescription) {
			std::string desc = DXGetErrorDescriptionA(hr);
			err += "; " + desc;
		}
		return err;
	}
};
class EngineError {
public:
	EngineError() {}
	EngineError(const std::string& msg) : message_(msg) {}

	const std::string& GetError() { return message_; }
	const char* what() { return message_.c_str(); }
protected:
	std::string message_;
};

//*******************************************************************
//Color utilities
//*******************************************************************

class ColorUtility {
public:
	static inline byte GetA(const D3DCOLOR& color) {
		return (color >> 24) & 0xff;
	}
	static inline byte GetR(const D3DCOLOR& color) {
		return (color >> 16) & 0xff;
	}
	static inline byte GetG(const D3DCOLOR& color) {
		return (color >> 8) & 0xff;
	}
	static inline byte GetB(const D3DCOLOR& color) {
		return color & 0xff;
	}

	template<typename T>
	static inline void Clamp(T& color) {
		color = std::clamp(color, (T)0x00, (T)0xff);
	}
	template<typename T>
	static inline T ClampRet(T color) {
		ColorUtility::Clamp(color);
		return color;
	}
};

//*******************************************************************
//Math utilities
//*******************************************************************

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
			MODE_LINEAR,
			MODE_SMOOTH,
			MODE_SMOOTHER,
			MODE_ACCELERATE,
			MODE_DECELERATE,
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
};

//*******************************************************************
//Rect utilities
//*******************************************************************
template<typename T>
class DxRect {
public:
	DxRect() {
		left = (T)0;
		top = (T)0;
		right = (T)0;
		bottom = (T)0;
	}
	DxRect(T l, T t, T r, T b) : left(l), top(t), right(r), bottom(b) {}
	DxRect(const DxRect<T>& src) {
		left = src.left;
		top = src.top;
		right = src.right;
		bottom = src.bottom;
	}
	DxRect(const RECT& src) {
		left = (T)src.left;
		top = (T)src.top;
		right = (T)src.right;
		bottom = (T)src.bottom;
	}
	template<typename L>
	DxRect(const DxRect<L>& src) {
		left = (T)src.left;
		top = (T)src.top;
		right = (T)src.right;
		bottom = (T)src.bottom;
	}

	template<typename L>
	inline DxRect<L> NewAs() {
		DxRect<L> res = DxRect<L>((L)left, (L)top,
			(L)right, (L)bottom);
		return res;
	}
	inline void Set(T l, T t, T r, T b) {
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	T GetWidth() const { return right - left; }
	T GetHeight() const { return bottom - top; }

	static DxRect<T> SetFromIndex(int wd, int ht, int id, int ict, int ox = 0, int oy = 0) {
		int iw = (id % ict) * wd + ox;
		int ih = (id / ict) * ht + oy;
		return DxRect<T>(iw, ih, iw + wd, ih + ht);
	}

	bool IsIntersected(const DxRect<T>& other) const {
		return !(other.left > right || other.right < left
			|| other.top > bottom || other.bottom < top);
	}
public:
	T left, top, right, bottom;
};

//*******************************************************************
//String utilities
//*******************************************************************

class StringUtility {
public:
	static std::string Format(const char* str, ...) {
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
};

//*******************************************************************
//Path utilities
//*******************************************************************

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