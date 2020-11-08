#pragma once

#define CD3DXVECTOR2 const D3DXVECTOR2&
#define CD3DXVECTOR3 const D3DXVECTOR3&

#define GET_INSTANCE(_type, _name) _type* _name = _type::GetBase();

class DxResourceManagerBase {
public:
	virtual ~DxResourceManagerBase() {}

	virtual void OnLostDevice() {}
	virtual void OnRestoreDevice() {}
};

//*******************************************************************
//Rect utilities
//*******************************************************************
template<typename T>
class DxRect {
public:
	DxRect() : DxRect(0, 0, 0, 0) {}
	DxRect(T v) : DxRect(v, v, v, v) {}
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
		return SetFromSize(iw, ih, wd, ht);
	}
	static inline DxRect<T> SetFromSize(T wd, T ht) {
		return DxRect<T>(wd, ht, wd, ht);
	}
	static inline DxRect<T> SetFromSize(T x, T y, T wd, T ht) {
		return DxRect<T>(x, y, x + wd, y + ht);
	}

	bool IsIntersected(const DxRect<T>& other) const {
		return !(other.left > right || other.right < left
			|| other.top > bottom || other.bottom < top);
	}

#pragma region DxRect_Operator
	DxRect<T>& operator+=(const DxRect<T>& other) {
		left += other.left; top += other.top;
		right += other.right; bottom += other.bottom;
		return *this;
	}
	friend DxRect<T> operator+(DxRect<T> lc, const DxRect<T>& rc) {
		lc += rc; return lc;
	}
	friend DxRect<T> operator+(DxRect<T> lc, const T& rc) {
		lc += DxRect<T>(rc); return lc;
	}

	DxRect<T>& operator-=(const DxRect<T>& other) {
		left -= other.left; top -= other.top;
		right -= other.right; bottom -= other.bottom;
		return *this;
	}
	friend DxRect<T> operator-(DxRect<T> lc, const DxRect<T>& rc) {
		lc -= rc; return lc;
	}
	friend DxRect<T> operator-(DxRect<T> lc, const T& rc) {
		lc -= DxRect<T>(rc); return lc;
	}

	DxRect<T>& operator*=(const DxRect<T>& other) {
		left *= other.left; top *= other.top;
		right *= other.right; bottom *= other.bottom;
		return *this;
	}
	friend DxRect<T> operator*(DxRect<T> lc, const DxRect<T>& rc) {
		lc *= rc; return lc;
	}
	friend DxRect<T> operator*(DxRect<T> lc, const T& rc) {
		lc *= DxRect<T>(rc); return lc;
	}

	DxRect<T>& operator/=(const DxRect<T>& other) {
		left /= other.left; top /= other.top;
		right /= other.right; bottom /= other.bottom;
		return *this;
	}
	friend DxRect<T> operator/(DxRect<T> lc, const DxRect<T>& rc) {
		lc /= rc; return lc;
	}
	friend DxRect<T> operator/(DxRect<T> lc, const T& rc) {
		lc /= DxRect<T>(rc); return lc;
	}
#pragma endregion DxRect_Operator
public:
	T left, top, right, bottom;
};