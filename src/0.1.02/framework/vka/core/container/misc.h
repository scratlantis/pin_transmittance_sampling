#pragma once


template <typename T>
struct Rect2D
{
	T x;
	T y;
	T width;
	T height;
	bool operator==(const Rect2D& other) const
	{
		return x == other.x && y == other.y && width == other.width && height == other.height;
	}
};