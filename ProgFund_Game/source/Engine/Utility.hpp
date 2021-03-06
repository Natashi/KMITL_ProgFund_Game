#pragma once

#include "../../pch.h"

#include "PointerExtensions.hpp"
#include "VectorExtensions.hpp"
#include "Shape.hpp"
#include "Table.hpp"
#include "Rand.hpp"

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

	static inline D3DCOLOR VectorToD3DColor(const D3DXVECTOR4& vec) {
		__m128 v1 = Vectorize::Load(vec);
		v1 = Vectorize::Mul(v1, Vectorize::Replicate(255.0f));
		return D3DCOLOR_RGBA((int)v1.m128_f32[0], (int)v1.m128_f32[1], 
			(int)v1.m128_f32[2], (int)v1.m128_f32[3]);
	}
	static inline D3DXVECTOR4 D3DColorToVector(const D3DCOLOR& col) {
		return D3DXVECTOR4(GetR(col) / 255.0f, GetG(col) / 255.0f, 
			GetB(col) / 255.0f, GetA(col) / 255.0f);
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
#define GM_DTORA(a) Math::DegreeToRadian(a)
#define GM_RTODA(a) Math::RadianToDegree(a)

	static inline double NormalizeAngleDeg(double angle) {
		angle = fmod(angle, 360.0);
		return angle < 0.0 ? angle + 360.0 : angle;
	}
	static inline double NormalizeAngleRad(double angle) {
		angle = fmod(angle, GM_PI_X2);
		return angle < 0.0 ? angle + GM_PI_X2 : angle;
	}
	static inline double AngleDifferenceRad(double angleFrom, double angleTo) {
		double dist = NormalizeAngleRad(angleTo - angleFrom);
		return dist > GM_PI ? dist - GM_PI_X2 : dist;
	}

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

		template<typename T>
		static inline T DifferentiateLinear(T x) {
			return (T)1;
		}
		template<typename T>
		static inline T DifferentiateSmooth(T x) {
			return (T)6 * x * ((T)1 - x);
		}
		template<typename T>
		static inline T DifferentiateSmoother(T x) {
			return (T)30 * x * x * (x * x - (T)2 * x + (T)1);
		}
		template<typename T>
		static inline T DifferentiateAccelerate(T x) {
			return (T)2 * x;
		}
		template<typename T>
		static inline T DifferentiateDecelerate(T x) {
			return (T)2 * ((T)1 - x);
		}
	};
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
	static const std::string& GetWorkingDirectory() {
		static std::string dir;
		if (dir.size() == 0) {
			char path[MAX_PATH];
			GetCurrentDirectoryA(MAX_PATH, path);
			dir = stdfs::path(path).make_preferred().generic_string();
			dir = dir + "/";
		}
		return dir;
	}
	static std::string GetUnique(const std::string& srcPath) {
		std::string p = stdfs::weakly_canonical(srcPath).make_preferred().generic_string();
		return p;
	}
	static bool IsFileExists(const std::string& path) {
		stdfs::path p(path);
		bool res = stdfs::exists(p) && (stdfs::status(p).type() == stdfs::file_type::regular);
		return res;
	}
};

//*******************************************************************
//Threadutilities
//*******************************************************************
template<class F>
static void ParallelTask(size_t countLoop, F&& func) {
	static const size_t countCore = std::max(std::thread::hardware_concurrency(), 1U);

	std::vector<std::future<void>> workers;
	workers.reserve(countCore);

	auto coreTask = [&](size_t id) {
		const size_t begin = countLoop / countCore * id + std::min(countLoop % countCore, id);
		const size_t end = countLoop / countCore * (id + 1U) + std::min(countLoop % countCore, id + 1U);

		for (size_t i = begin; i < end; ++i)
			func(i);
	};

	for (size_t iCore = 0; iCore < countCore; ++iCore)
		workers.emplace_back(std::async(std::launch::async | std::launch::deferred, coreTask, iCore));
	for (const auto& worker : workers)
		worker.wait();
}