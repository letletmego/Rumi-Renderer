#ifndef _VECTOR_H
#define _VECTOR_H

#include "clibrary.h"

class Vector3
{
public:
	float _x;
	float _y;
	float _z;
	
public:
	~Vector3(void);
	Vector3(void);
	Vector3(const float &value);
	Vector3(const float &x, const float &y, const float &z);

public:
	Vector3 operator + (const Vector3 &vector) const;
	Vector3 operator - (const Vector3 &vector) const;
	Vector3 operator - (void) const;
	Vector3 operator * (const float &scalar) const;
	float operator [] (const int &index) const;
	bool IsNaN(void) const;
	bool IsInf(void) const;
};

const Vector3 X_AXIS = Vector3(1.0f, 0.0f, 0.0f);
const Vector3 Y_AXIS = Vector3(0.0f, 1.0f, 0.0f);
const Vector3 Z_AXIS = Vector3(0.0f, 0.0f, 1.0f);
//const Vector3 YY = Vector3(0.0034f, 1.0f, 0.0071f);

inline float Dot(const Vector3 &u, const Vector3 &v)
{
	return u._x * v._x + u._y * v._y + u._z * v._z;
}

inline float Length(const Vector3 &vector)
{
	return sqrt(vector._x * vector._x + vector._y * vector._y + vector._z * vector._z);
}

inline Vector3 Normalize(const Vector3 &vector)
{
	float inv_length = 1.0f / sqrtf(vector._x * vector._x + vector._y * vector._y + vector._z * vector._z);
	
	return Vector3(vector._x * inv_length, vector._y * inv_length, vector._z * inv_length);
}

inline Vector3 Normalize(const float &x, const float &y, const float &z)
{
	float inv_length = 1.0f / sqrtf(x * x + y * y + z * z);
	
	return Vector3(x * inv_length, y * inv_length, z * inv_length);
}

inline Vector3 Cross(const Vector3 &u, const Vector3 &v)
{
	// |_u.x, _u.y, _u.z|
	// |_v.x, _v.y, _v.z|
	// |0.0f, 0.0f, 0.0f|
	float cx = u._y * v._z - v._y * u._z;
	float cy = u._z * v._x - v._z * u._x;
	float cz = u._x * v._y - v._x * u._y;

	return Vector3(cx, cy, cz);
}

inline void OrthonormalBasis(const Vector3 &v0, Vector3 *v1, Vector3 *v2)
{
	if (v1 == 0x00 || v2 == 0x00)
		return;

	if (v0._x == 0.0f && v0._z == 0.0f &&
		(v0._y == 1.0f || v0._y == -1.0f))
	{
		*v1 = X_AXIS;

		if (v0._y == 0.0f)
			*v2 = Z_AXIS;
		else
			*v2 = -Z_AXIS;
	}
	else
	{
		*v1 = Normalize(Cross(v0, Y_AXIS));
		if (v1->IsNaN() == true)
			*v1 = X_AXIS;

		*v2 = Cross(v0, *v1);
	}

	return;
}

#endif