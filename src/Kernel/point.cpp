#include "point.h"
#include "vector.h"

Point2::~Point2(void)
{
}

Point2::Point2(void)
	: _x(0.0f)
	, _y(0.0f)
{
}

Point2::Point2(const float &x, const float &y)
	: _x(x)
	, _y(y)
 {
 }



Point3::~Point3(void)
{
}

Point3::Point3(void)
	: _x(0.0f)
	, _y(0.0f)
	, _z(0.0f)
{
}

Point3::Point3(const float &value)
	: _x(value)
	, _y(value)
	, _z(value)
{
}

Point3::Point3(const float &x, const float &y, const float &z)
	: _x(x)
	, _y(y)
	, _z(z)
{
}

Point3 Point3::operator + (const Vector3 &vector) const
{
	return Point3(_x + vector._x, _y + vector._y, _z + vector._z);
}

Point3 Point3::operator - (const Vector3 &vector) const
{
	return Point3(_x - vector._x, _y - vector._y, _z - vector._z);
}

Vector3 Point3::operator - (const Point3 &point) const
{
	return Vector3(_x - point._x, _y - point._y, _z - point._z);
}

Point3 Point3::operator * (const float &scalar) const
{
	return Point3(_x * scalar, _y * scalar, _z * scalar);
}

float Point3::operator [] (const int &index) const
{
	if (index == 0)
		return _x;
	if (index == 1)
		return _y;
	if (index == 2)
		return _z;

	return 0.0f;
}

bool Point3::IsNaN(void) const
{
	return _x != _x || _y != _y || _z != _z;
}

bool Point3::IsInf(void) const
{
	return !_finite(_x) || !_finite(_y) || !_finite(_z);
}