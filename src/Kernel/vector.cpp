#include "vector.h"


Vector3::~Vector3(void)
{
}

Vector3::Vector3(void)
	: _x(0.0f)
	, _y(0.0f)
	, _z(0.0f)
{
}

Vector3::Vector3(const float &value)
	: _x(value)
	, _y(value)
	, _z(value)
{
}

Vector3::Vector3(const float &x, const float &y, const float &z)
	: _x(x)
	, _y(y)
	, _z(z)
{
} 

Vector3 Vector3::operator + (const Vector3 &vector) const
{
	return Vector3(_x + vector._x, _y + vector._y, _z + vector._z);
}

Vector3 Vector3::operator - (const Vector3 &vector) const
{
	return Vector3(_x - vector._x, _y - vector._y, _z - vector._z);
}

Vector3 Vector3::operator - (void) const
{
	return Vector3(-_x, -_y, -_z);
}

Vector3 Vector3::operator * (const float &scalar) const
{
	return Vector3(_x * scalar, _y * scalar, _z * scalar);
}

float Vector3::operator [] (const int &index) const
{
	if (index == 0)
		return _x;
	if (index == 1)
		return _y;
	if (index == 2)
		return _z;

	return 0.0f;
}

bool Vector3::IsNaN(void) const
{
	return _x != _x || _y != _y || _z != _z; 
}

bool Vector3::IsInf(void) const
{
	return !_finite(_x) || !_finite(_y) || !_finite(_z);
}