#pragma once

#define _CRT_SECURE_NO_WARNINGS

//windows

#define _WIN32_WINNT _WIN32_WINNT_WIN7		//Minimum support -> Windows 7
#define WINVER _WIN32_WINNT

#include <Windows.h>

#pragma comment(lib, "winmm.lib")

//xmm

#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

//C++ STL

#include <cmath>
#include <ctime>
#include <cstdlib>

#include <typeindex>

#include <iostream>
#include <fstream>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>

#include <memory>

#include <chrono>

#include <filesystem>

#undef min
#undef max

#pragma warning(disable : 4005)		//macro redefinition
#pragma warning(disable : 4244)		//conversion from x to y, possible loss of data
#pragma warning(disable : 4305)		//double->float truncation
#pragma warning(disable : 26495)	//'x' is uninitialized
#pragma warning(disable : 26812)	//prefer enum class over enum

#pragma warning(disable : 28251)	//Inconsistent annotation

//DirectX

#define D3D_OVERLOADS

#define DIRECTSOUND_VERSION 0x0900
#define DIRECTINPUT_VERSION 0x0800

#include <DXGI.h>
#include <DxErr.h>
#include <dinput.h>
#include <dsound.h>

#include <D3D9.h>
#include <D3DX9.h>

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "d3dxof.lib")
#pragma comment(lib, "dxerr.lib")

#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

//Others

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#pragma comment(lib, "ogg_static.lib")
#pragma comment(lib, "vorbis_static.lib")
#pragma comment(lib, "vorbisfile_static.lib")

#pragma comment(lib, "freetype.lib")

//Pointer utilities
template<typename T> static constexpr inline void ptr_delete(T*& ptr) {
	if (ptr) delete ptr;
	ptr = nullptr;
}
template<typename T> static constexpr inline void ptr_delete_scalar(T*& ptr) {
	if (ptr) delete[] ptr;
	ptr = nullptr;
}
template<typename T> static constexpr inline void ptr_release(T*& ptr) {
	if (ptr) ptr->Release();
	ptr = nullptr;
}
using std::shared_ptr;
using std::weak_ptr;

namespace stdch = std::chrono;
namespace stdfs = std::filesystem;

#define CD3DXVECTOR2 const D3DXVECTOR2&
#define CD3DXVECTOR3 const D3DXVECTOR3&

#define GET_INSTANCE(_type, _name) _type* _name = _type::GetBase();

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480