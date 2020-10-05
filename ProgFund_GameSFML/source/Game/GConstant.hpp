#pragma once

#include "../../pch.h"

#include "../Engine/VectorExtensions.hpp"

#include "../Engine/Utility.hpp"

#include "../Engine/Input.hpp"
#include "../Engine/Scene.hpp"
#include "../Engine/ResourceManager.hpp"

#include "../Engine/Vertex.hpp"
#include "../Engine/Object.hpp"

#include "../Engine/Window.hpp"

//-------------------------------------------------

template<typename T, typename I, typename L>
static inline T IncUntil(T& v, I rate, L lim) {
	v = std::min((T)lim, v + (T)rate);
	return v;
}
template<typename T, typename I, typename L>
static inline T DecUntil(T& v, I rate, L lim) {
	v = std::max((T)lim, v - (T)rate);
	return v;
}