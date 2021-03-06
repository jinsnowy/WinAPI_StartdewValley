#pragma once

#include "Vec2.h"

template<typename T>
class Rect_
{
public:
	Rect_() = default;
	Rect_(T left_in, T top_in, T right_in, T bottom_in)
		:
		left(left_in),
		top(top_in),
		right(right_in),
		bottom(bottom_in)
	{
	}
	Rect_(const Vec2_<T>& topLeft, const Vec2_<T>& bottomRight)
		:
		Rect_(topLeft.x, bottomRight.x, topLeft.y, bottomRight.y)
	{
	}
	Rect_(const Vec2_<T>& topLeft, T width, T height)
		:
		Rect_(topLeft, topLeft + Vec2_<T>(width, height))
	{
	}
	operator RECT()
	{
		return RECT(left, top, right, bottom);
	}
	bool IsCollideRect(const Rect_& other) const
	{
		if (right < other.left) return false;
		if (left > other.right) return false;
		if (bottom < other.top) return false;
		if (top > other.bottom) return false;
		return true;
	}
	bool IsInsideRect(const Rect_& other) const
	{
		return left >= other.left && right <= other.right &&
			top >= other.top && bottom <= other.bottom;
	}
	bool ContainsPoint(const Vec2_<T>& point) const
	{
		return point.x >= left && point.x <= right&& point.y >= top && point.y <= bottom;
	}
	Rect_<T> SubtractOffset(const Vec2_<T>& offset) const
	{
		return Rect_<T>(left - offset.x, top - offset.y, right - offset.x, bottom - offset.y);
	}
	Vec2_<T> GetCenter() const
	{
		return Vec2_<T>((left + right) / 2, (top + bottom) / 2);
	}
	T GetWidth() const
	{
		return right - left;
	}
	T GetHeight() const
	{
		return bottom - top;
	}
	void SetRect(T l, T t, T r, T b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}
public:
	T left;
	T right;
	T top;
	T bottom;
};

typedef Rect_<int> RectI;
typedef Rect_<float> RectF;