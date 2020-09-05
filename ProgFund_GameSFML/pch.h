#pragma once

#define _CRT_SECURE_NO_WARNINGS

#pragma warning (disable : 26495)	//'x' is uninitialized

//windows

#pragma comment(lib, "winmm.lib")

//C++ STL

#include <cmath>
#include <ctime>
#include <cstdlib>

#include <iostream>
#include <fstream>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>

#include <memory>

#include <chrono>

//OpenGL

#include <GLEW/glew.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLEW/glew32s.lib")

//SFML

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/OpenGL.hpp>

#ifdef _DEBUG

#pragma comment(lib, "SFML/sfml-graphics-s-d.lib")
#pragma comment(lib, "SFML/sfml-audio-s-d.lib")
#pragma comment(lib, "SFML/sfml-window-s-d.lib")
#pragma comment(lib, "SFML/sfml-system-s-d.lib")
#pragma comment(lib, "SFML/sfml-main-d.lib")

#else

#pragma comment(lib, "SFML/sfml-graphics-s.lib")
#pragma comment(lib, "SFML/sfml-audio-s.lib")
#pragma comment(lib, "SFML/sfml-window-s.lib")
#pragma comment(lib, "SFML/sfml-system-s.lib")
#pragma comment(lib, "SFML/sfml-main.lib")

#endif

//XNA Maths

#pragma warning(push)
#pragma warning(disable : 4838)	//conversion requires a narrowing conversion
#pragma warning(disable : 6011)	//dereferencing nullptr 'x'
#pragma warning(disable : 6385)	//buffer overrun while writing to 'x'
#pragma warning(disable : 6386)	//reading invalid data from 'x'

#include <XNAMaths/xnamath.h>

#pragma warning(pop)

//libogg + libvorbis

#pragma comment(lib, "libogg/ogg.lib")
#pragma comment(lib, "libvorbis/vorbis.lib")
#pragma comment(lib, "libvorbis/vorbisfile.lib")

//freetype

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