#pragma once
#include "../../pch.h"

class DxShapeBase {
public:
	virtual ~DxShapeBase() {};
};

//*******************************************************************
//Rectangle
//*******************************************************************
template<typename T>
class DxRectangle : public DxShapeBase {
public:
	T left, top, right, bottom;
public:
	DxRectangle() : DxRectangle(0, 0, 0, 0) {}
	DxRectangle(T v) : DxRectangle(v, v, v, v) {}
	DxRectangle(T l, T t, T r, T b) : left(l), top(t), right(r), bottom(b) {}
	DxRectangle(const DxRectangle<T>& src) {
		left = src.left;
		top = src.top;
		right = src.right;
		bottom = src.bottom;
	}
	DxRectangle(const RECT& src) {
		left = (T)src.left;
		top = (T)src.top;
		right = (T)src.right;
		bottom = (T)src.bottom;
	}
	template<typename L> DxRectangle(const DxRectangle<L>& src) {
		left = (T)src.left;
		top = (T)src.top;
		right = (T)src.right;
		bottom = (T)src.bottom;
	}

	template<typename L> inline DxRectangle<L> NewAs() {
		DxRectangle<L> res = DxRectangle<L>((L)left, (L)top,
			(L)right, (L)bottom);
		return res;
	}
	inline void Set(T l, T t, T r, T b) {
		left = l; top = t;
		right = r; bottom = b;
	}

	T GetWidth() const { return right - left; }
	T GetHeight() const { return bottom - top; }

	static DxRectangle<T> SetFromIndex(int wd, int ht, int id, int ict, int ox = 0, int oy = 0) {
		int iw = (id % ict) * wd + ox;
		int ih = (id / ict) * ht + oy;
		return SetFromSize(iw, ih, wd, ht);
	}
	static inline DxRectangle<T> SetFromSize(T wd, T ht) {
		return DxRectangle<T>(wd, ht, wd, ht);
	}
	static inline DxRectangle<T> SetFromSize(T x, T y, T wd, T ht) {
		return DxRectangle<T>(x, y, x + wd, y + ht);
	}

#pragma region DxRectangle_Operator
	DxRectangle<T>& operator+=(const DxRectangle<T>& other) {
		left += other.left; top += other.top;
		right += other.right; bottom += other.bottom;
		return *this;
	}
	friend DxRectangle<T> operator+(DxRectangle<T> lc, const DxRectangle<T>& rc) {
		lc += rc; return lc;
	}
	friend DxRectangle<T> operator+(DxRectangle<T> lc, const T& rc) {
		lc += DxRectangle<T>(rc); return lc;
	}

	DxRectangle<T>& operator-=(const DxRectangle<T>& other) {
		left -= other.left; top -= other.top;
		right -= other.right; bottom -= other.bottom;
		return *this;
	}
	friend DxRectangle<T> operator-(DxRectangle<T> lc, const DxRectangle<T>& rc) {
		lc -= rc; return lc;
	}
	friend DxRectangle<T> operator-(DxRectangle<T> lc, const T& rc) {
		lc -= DxRectangle<T>(rc); return lc;
	}

	DxRectangle<T>& operator*=(const DxRectangle<T>& other) {
		left *= other.left; top *= other.top;
		right *= other.right; bottom *= other.bottom;
		return *this;
	}
	friend DxRectangle<T> operator*(DxRectangle<T> lc, const DxRectangle<T>& rc) {
		lc *= rc; return lc;
	}
	friend DxRectangle<T> operator*(DxRectangle<T> lc, const T& rc) {
		lc *= DxRectangle<T>(rc); return lc;
	}

	DxRectangle<T>& operator/=(const DxRectangle<T>& other) {
		left /= other.left; top /= other.top;
		right /= other.right; bottom /= other.bottom;
		return *this;
	}
	friend DxRectangle<T> operator/(DxRectangle<T> lc, const DxRectangle<T>& rc) {
		lc /= rc; return lc;
	}
	friend DxRectangle<T> operator/(DxRectangle<T> lc, const T& rc) {
		lc /= DxRectangle<T>(rc); return lc;
	}
#pragma endregion DxRectangle_Operator
};

//*******************************************************************
//Circle
//*******************************************************************
template<typename T>
class DxCircle : public DxShapeBase {
public:
	T x, y;
	T r;
public:
	DxCircle() : DxCircle(0, 0, 0, 0) {}
	DxCircle(T _x, T _y, T _r) : x(_x), y(_y), r(_r) {}
	DxCircle(const DxCircle<T>& src) {
		x = src.x;
		y = src.y;
		r = src.r;
	}
	template<typename L> DxCircle(const DxCircle<L>& src) {
		x = (T)src.x;
		y = (T)src.y;
		r = (T)src.r;
	}

	//Translates the circle
	friend DxCircle<T> operator+(DxCircle<T> lc, const T& rc) {
		lc.x += rc; lc.y -= rc;
		return lc;
	}
	friend DxCircle<T> operator-(DxCircle<T> lc, const T& rc) {
		lc.x -= rc; lc.y -= rc;
		return lc;
	}

	//Scales the radius
	friend DxCircle<T> operator*(DxCircle<T> lc, const T& rc) {
		lc.r *= rc;
		return lc;
	}
	friend DxCircle<T> operator/(DxCircle<T> lc, const T& rc) {
		lc.r /= rc;
		return lc;
	}
};

class DxCollider {
private:
	DxCollider() {};	//Static class
public:
	template<typename T> static bool Test(const DxRectangle<T>& lt, const DxRectangle<T>& rt) {
		return !(rt.left > lt.right || rt.right < lt.left 
			|| rt.top > lt.bottom || rt.bottom < lt.top);
	}

	template<typename T> static bool Test(const DxCircle<T>& lt, const DxCircle<T>& rt) {
		T dx = rt.x - lt.x; T dy = rt.y - lt.y; T rr = rt.r + lt.r;
		return (dx * dx + dy * dy) <= (rr * rr);
	}
};