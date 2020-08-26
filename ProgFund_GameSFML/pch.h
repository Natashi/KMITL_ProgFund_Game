#pragma once

#define _CRT_SECURE_NO_WARNINGS

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

//libogg + libvorbis

#pragma comment(lib, "libogg/ogg.lib")
#pragma comment(lib, "libvorbis/vorbis.lib")
#pragma comment(lib, "libvorbis/vorbisfile.lib")