
// Vec.h
//
// Copyright (c) 2013-2015 Michael Imamura.
//
// Licensed under GrokkSoft HoverRace SourceCode License v1.0(the "License");
// you may not use this file except in compliance with the License.
//
// A copy of the license should have been attached to the package from which
// you have taken this file. If you can not find the license you can not use
// this file.
//
//
// The author makes no representations about the suitability of
// this software for any purpose.  It is provided "as is" "AS IS",
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
//
// See the License for the specific language governing permissions
// and limitations under the License.

#pragma once

#if defined(_WIN32) && defined(HR_ENGINE_SHARED)
#	ifdef MR_ENGINE
#		define MR_DllDeclare   __declspec( dllexport )
#	else
#		define MR_DllDeclare   __declspec( dllimport )
#	endif
#else
#	define MR_DllDeclare
#endif

namespace HoverRace {

struct Vec3;

struct Vec2
{
	constexpr Vec2(double x, double y) noexcept : x(x), y(y) { }

	constexpr Vec3 Promote(double z = 0) const noexcept;

	double x;
	double y;
};

inline std::ostream &operator<<(std::ostream &os, const Vec2 &v)
{
	os << '<' << v.x << ", " << v.y << '>';
	return os;
}

inline constexpr bool operator==(const Vec2 &lhs, const Vec2 &rhs) noexcept
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y;
}

inline constexpr bool operator!=(const Vec2 &lhs, const Vec2 &rhs) noexcept
{
	return !operator==(lhs, rhs);
}

inline constexpr Vec2 operator+(const Vec2 &lhs, const Vec2 &rhs) noexcept
{
	return { lhs.x + rhs.x, lhs.y + rhs.y };
}

inline Vec2 &operator+=(Vec2 &lhs, const Vec2 &rhs) noexcept
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

inline constexpr Vec2 operator-(const Vec2 &lhs, const Vec2 &rhs) noexcept
{
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}

inline Vec2 &operator-=(Vec2 &lhs, const Vec2 &rhs) noexcept
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

inline constexpr Vec2 operator*(const Vec2 &lhs, double scale) noexcept
{
	return { lhs.x * scale, lhs.y * scale };
}

inline Vec2 &operator*=(Vec2 &lhs, double scale) noexcept
{
	lhs.x *= scale;
	lhs.y *= scale;
	return lhs;
}

inline constexpr Vec2 operator/(const Vec2 &lhs, double scale) noexcept
{
	return { lhs.x / scale, lhs.y / scale };
}

inline Vec2 &operator/=(Vec2 &lhs, double scale) noexcept
{
	lhs.x /= scale;
	lhs.y /= scale;
	return lhs;
}

struct Vec3
{
	constexpr Vec3(double x, double y, double z) noexcept : x(x), y(y), z(z) { }

	double x;
	double y;
	double z;
};

inline std::ostream &operator<<(std::ostream &os, const Vec3 &v)
{
	os << '<' << v.x << ", " << v.y << ", " << v.z << '>';
	return os;
}

inline constexpr bool operator==(const Vec3 &lhs, const Vec3 &rhs) noexcept
{
	return
		lhs.x == rhs.x &&
		lhs.y == rhs.y &&
		lhs.z == rhs.z;
}

inline constexpr bool operator!=(const Vec3 &lhs, const Vec3 &rhs) noexcept
{
	return !operator==(lhs, rhs);
}

inline constexpr Vec3 operator+(const Vec3 &lhs, const Vec3 &rhs) noexcept
{
	return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

inline Vec3 &operator+=(Vec3 &lhs, const Vec3 &rhs) noexcept
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

inline constexpr Vec3 operator+(const Vec3 &lhs, const Vec2 &rhs) noexcept
{
	return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z };
}

inline Vec3 &operator+=(Vec3 &lhs, const Vec2 &rhs) noexcept
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

inline constexpr Vec3 operator-(const Vec3 &lhs, const Vec3 &rhs) noexcept
{
	return Vec3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline Vec3 &operator-=(Vec3 &lhs, const Vec3 &rhs) noexcept
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	return lhs;
}

inline constexpr Vec3 operator*(const Vec3 &lhs, double scale) noexcept
{
	return { lhs.x * scale, lhs.y * scale, lhs.z * scale };
}

inline Vec3 &operator*=(Vec3 &lhs, double scale) noexcept
{
	lhs.x *= scale;
	lhs.y *= scale;
	lhs.z *= scale;
	return lhs;
}

inline constexpr Vec3 operator/(const Vec3 &lhs, double scale) noexcept
{
	return { lhs.x / scale, lhs.y / scale, lhs.z / scale };
}

inline Vec3 &operator/=(Vec3 &lhs, double scale) noexcept
{
	lhs.x /= scale;
	lhs.y /= scale;
	lhs.z /= scale;
	return lhs;
}

inline constexpr Vec3 Vec2::Promote(double z) const noexcept
{
	return { x, y, z };
}

}  // namespace HoverRace

#undef MR_DllDeclare
