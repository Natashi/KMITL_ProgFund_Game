#pragma once

#define _CRT_SECURE_NO_WARNINGS

//windows

#include <Windows.h>

#pragma comment(lib, "winmm.lib")

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

//DirectX

#pragma warning(disable : 4005)		//macro redefinition
#pragma warning(disable : 4244)		//conversion from x to y, possible loss of data
#pragma warning(disable : 26495)	//'x' is uninitialized
#pragma warning(disable : 26812)	//prefer enum class over enum

#include <DXGI.h>
#include <DxErr.h>
#include <dinput.h>
#include <dsound.h>

#include <D3D9.h>
#include <D3DX9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "DxErr.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dsound.lib")

#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

//XNA Maths

#pragma warning(push)
#pragma warning(disable : 4838)	//conversion requires a narrowing conversion
#pragma warning(disable : 6011)	//dereferencing nullptr 'x'
#pragma warning(disable : 6385)	//buffer overrun while writing to 'x'
#pragma warning(disable : 6386)	//reading invalid data from 'x'

#include <XNAMaths/xnamath.h>

#pragma warning(pop)

//Others

#pragma comment(lib, "libogg/ogg.lib")
#pragma comment(lib, "libvorbis/vorbis.lib")
#pragma comment(lib, "libvorbis/vorbisfile.lib")

#pragma comment(lib, "freetype.lib")

//using namespace sf;

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