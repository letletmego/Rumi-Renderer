#ifndef _MATRIX_H
#define _MATRIX_H

class Point3;
class Vector3;

class Mat4x4
{
public:
	float _m[16];

public:
	~Mat4x4(void);
	Mat4x4(void);

public:
	Mat4x4 operator * (const Mat4x4 &matrix) const;
	Point3 operator * (const Point3 position) const;
	Vector3 operator * (const Vector3 vector) const;

	friend Mat4x4 Inverse(const Mat4x4 &matrix);
	friend Mat4x4 Transpose(const Mat4x4 &matrix);
	friend Mat4x4 Translation(float x, float y, float z);
	friend Mat4x4 Scaling(float x, float y, float z);
	friend Mat4x4 RotationX(float radian);
	friend Mat4x4 RotationY(float radian);
	friend Mat4x4 RotationZ(float radian);
};

#endif