#include "matrix.h"
#include "clibrary.h"
#include "point.h"
#include "vector.h"

Mat4x4::~Mat4x4(void)
{
}

Mat4x4::Mat4x4(void)
{
	_m[0]  = 1.0f; _m[1]  = 0.0f; _m[2]  = 0.0f; _m[3]  = 0.0f;
	_m[4]  = 0.0f; _m[5]  = 1.0f; _m[6]  = 0.0f; _m[7]  = 0.0f;
	_m[8]  = 0.0f; _m[9]  = 0.0f; _m[10] = 1.0f; _m[11] = 0.0f;
	_m[12] = 0.0f; _m[13] = 0.0f; _m[14] = 0.0f; _m[15] = 1.0f;
}

Mat4x4 Mat4x4::operator * (const Mat4x4 &matrix) const
{
	Mat4x4 output;

	output._m[0]  = _m[0] * matrix._m[0] + _m[1] * matrix._m[4] + _m[2] * matrix._m[8]  + _m[3] * matrix._m[12];
	output._m[1]  = _m[0] * matrix._m[1] + _m[1] * matrix._m[5] + _m[2] * matrix._m[9]  + _m[3] * matrix._m[13];
	output._m[2]  = _m[0] * matrix._m[2] + _m[1] * matrix._m[6] + _m[2] * matrix._m[10] + _m[3] * matrix._m[14];
	output._m[3]  = _m[0] * matrix._m[3] + _m[1] * matrix._m[7] + _m[2] * matrix._m[11] + _m[3] * matrix._m[15];

	output._m[4]  = _m[4] * matrix._m[0] + _m[5] * matrix._m[4] + _m[6] * matrix._m[8]  + _m[7] * matrix._m[12];
	output._m[5]  = _m[4] * matrix._m[1] + _m[5] * matrix._m[5] + _m[6] * matrix._m[9]  + _m[7] * matrix._m[13];
	output._m[6]  = _m[4] * matrix._m[2] + _m[5] * matrix._m[6] + _m[6] * matrix._m[10] + _m[7] * matrix._m[14];
	output._m[7]  = _m[4] * matrix._m[3] + _m[5] * matrix._m[7] + _m[6] * matrix._m[11] + _m[7] * matrix._m[15];

	output._m[8]  = _m[8] * matrix._m[0] + _m[9] * matrix._m[4] + _m[10] * matrix._m[8]  + _m[11] * matrix._m[12];
	output._m[9]  = _m[8] * matrix._m[1] + _m[9] * matrix._m[5] + _m[10] * matrix._m[9]  + _m[11] * matrix._m[13];
	output._m[10] = _m[8] * matrix._m[2] + _m[9] * matrix._m[6] + _m[10] * matrix._m[10] + _m[11] * matrix._m[14];
	output._m[11] = _m[8] * matrix._m[3] + _m[9] * matrix._m[7] + _m[10] * matrix._m[11] + _m[11] * matrix._m[15];

	output._m[12] = _m[12] * matrix._m[0] + _m[13] * matrix._m[4] + _m[14] * matrix._m[8]  + _m[15] * matrix._m[12];
	output._m[13] = _m[12] * matrix._m[1] + _m[13] * matrix._m[5] + _m[14] * matrix._m[9]  + _m[15] * matrix._m[13];
	output._m[14] = _m[12] * matrix._m[2] + _m[13] * matrix._m[6] + _m[14] * matrix._m[10] + _m[15] * matrix._m[14];
	output._m[15] = _m[12] * matrix._m[3] + _m[13] * matrix._m[7] + _m[14] * matrix._m[11] + _m[15] * matrix._m[15];

	return output;
}

Point3 Mat4x4::operator * (const Point3 position) const
{
	Point3 p;

	p._x = _m[0] * position._x + _m[1] * position._y + _m[2]  * position._z + _m[3];
	p._y = _m[4] * position._x + _m[5] * position._y + _m[6]  * position._z + _m[7];
	p._z = _m[8] * position._x + _m[9] * position._y + _m[10] * position._z + _m[11];

	return p;
}

Vector3 Mat4x4::operator * (const Vector3 vector) const
{
	Vector3 v;

	v._x = _m[0] * vector._x + _m[1] * vector._y + _m[2]  * vector._z + _m[3];
	v._y = _m[4] * vector._x + _m[5] * vector._y + _m[6]  * vector._z + _m[7];
	v._z = _m[8] * vector._x + _m[9] * vector._y + _m[10] * vector._z + _m[11];

	return v;
}

Mat4x4 Inverse(const Mat4x4 &matrix)
{
	Mat4x4 src_temp(matrix);
	Mat4x4 inv;
	int row_idx;
	int col_idx;
	float pivot;
	float unknown_x;
	float scalar;

	// Eliminate lower triangle
	for(col_idx = 0; col_idx < 4; ++col_idx)
	{
		int pivot_idx = col_idx * 4 + col_idx;

		for(row_idx = col_idx + 1; row_idx < 4; ++row_idx)
		{
			int scalar_idx = row_idx * 4 + col_idx;
			pivot = src_temp._m[pivot_idx];
			scalar = src_temp._m[scalar_idx];
			unknown_x = -scalar / pivot;
			
			for (int idx = 0; idx < 4; ++idx)
			{
				int add_idx = row_idx * 4 + idx;
				scalar_idx = col_idx * 4 + idx;
				src_temp._m[add_idx] += src_temp._m[scalar_idx] * unknown_x;
				inv._m[add_idx] += inv._m[scalar_idx] * unknown_x;
			}
		}
	}

	// Eliminate upper triangle
	for(col_idx = 0; col_idx < 4; ++col_idx)
	{
		int pivot_idx = col_idx * 4 + col_idx;

		for(row_idx = col_idx - 1; row_idx >= 0; --row_idx)
		{
			int scalar_idx = row_idx * 4 + col_idx;
			pivot = src_temp._m[pivot_idx];
			scalar = src_temp._m[scalar_idx];
			unknown_x = -scalar / pivot;
			
			for (int idx = 0; idx < 4; ++idx)
			{
				int add_idx = row_idx * 4 + idx;
				scalar_idx = col_idx * 4 + idx;
				src_temp._m[add_idx] += src_temp._m[scalar_idx] * unknown_x;
				inv._m[add_idx] += inv._m[scalar_idx] * unknown_x;
			}
		}
	}

	for (int idx = 0; idx < 4; ++idx)
	{
		int pivot_idx = idx * 4 + idx;
		float inv_pivot = 1.0f / src_temp._m[pivot_idx];

		int inv_row_idx = idx * 4;
		inv._m[inv_row_idx]     *= inv_pivot;
		inv._m[inv_row_idx + 1] *= inv_pivot;
		inv._m[inv_row_idx + 2] *= inv_pivot;
		inv._m[inv_row_idx + 3] *= inv_pivot;
	}

	return inv;
}

Mat4x4 Transpose(const Mat4x4 &matrix)
{
	Mat4x4 transpose;

	transpose._m[0]  = matrix._m[0];  transpose._m[1]  = matrix._m[4];  transpose._m[2]  = matrix._m[8];   transpose._m[3]  = matrix._m[12];
	transpose._m[4]  = matrix._m[1];  transpose._m[5]  = matrix._m[5];  transpose._m[6]  = matrix._m[9];   transpose._m[7]  = matrix._m[13];
	transpose._m[8]  = matrix._m[2];  transpose._m[9]  = matrix._m[6];  transpose._m[10] = matrix._m[10];  transpose._m[11] = matrix._m[14];
	transpose._m[12] = matrix._m[3];  transpose._m[13] = matrix._m[7];  transpose._m[14] = matrix._m[11];  transpose._m[15] = matrix._m[15];

	return transpose;
}

Mat4x4 Translation(float x, float y, float z)
{
	Mat4x4 t;

	t._m[0]  = 1.0f; t._m[1]  = 0.0f; t._m[2]  = 0.0f; t._m[3]  = x;
	t._m[4]  = 0.0f; t._m[5]  = 1.0f; t._m[6]  = 0.0f; t._m[7]  = y;
	t._m[8]  = 0.0f; t._m[9]  = 0.0f; t._m[10] = 1.0f; t._m[11] = z;
	t._m[12] = 0.0f; t._m[13] = 0.0f; t._m[14] = 0.0f; t._m[15] = 1.0f;

	return t;
}

Mat4x4 Scaling(float x, float y, float z)
{
	Mat4x4 s;

	s._m[0]  = x;    s._m[1]  = 0.0f; s._m[2]  = 0.0f; s._m[3]  = 0.0f;
	s._m[4]  = 0.0f; s._m[5]  = y;    s._m[6]  = 0.0f; s._m[7]  = 0.0f;
	s._m[8]  = 0.0f; s._m[9]  = 0.0f; s._m[10] = z;    s._m[11] = 0.0f;
	s._m[12] = 0.0f; s._m[13] = 0.0f; s._m[14] = 0.0f; s._m[15] = 1.0f;

	return s;
}

Mat4x4 RotationX(float radian)
{
	Mat4x4 r;

	r._m[0]  = 1.0f;  r._m[1]  = 0.0f;         r._m[2]  = 0.0f;          r._m[3]  = 0.0f;
	r._m[4]  = 0.0f;  r._m[5]  = cos(radian);  r._m[6]  = -sin(radian);  r._m[7]  = 0.0f;
	r._m[8]  = 0.0f;  r._m[9]  = sin(radian);  r._m[10] =  cos(radian);  r._m[11] = 0.0f;
	r._m[12] = 0.0f;  r._m[13] = 0.0f;         r._m[14] = 0.0f;          r._m[15] = 1.0f;

	return r;
}

Mat4x4 RotationY(float radian)
{
	Mat4x4 r;

	r._m[0]  = cos(radian);   r._m[1]  = 0.0f;  r._m[2]  = sin(radian);  r._m[3]  = 0.0f;
	r._m[4]  = 0.0f;          r._m[5]  = 1.0f;  r._m[6]  = 0.0f;         r._m[7]  = 0.0f;
	r._m[8]  = -sin(radian);  r._m[9]  = 0.0f;  r._m[10] = cos(radian);  r._m[11] = 0.0f;
	r._m[12] = 0.0f;          r._m[13] = 0.0f;  r._m[14] = 0.0f;         r._m[15] = 1.0f;

	return r;
}

Mat4x4 RotationZ(float radian)
{
	Mat4x4 r;

	r._m[0]  = cos(radian);  r._m[1]  = -sin(radian);  r._m[2]  = 0.0f;  r._m[3]  = 0.0f;
	r._m[4]  = sin(radian);  r._m[5]  = cos(radian);   r._m[6]  = 0.0f;  r._m[7]  = 0.0f;
	r._m[8]  = 0.0f;         r._m[9]  = 0.0f;          r._m[10] = 1.0f;  r._m[11] = 0.0f;
	r._m[12] = 0.0f;         r._m[13] = 0.0f;          r._m[14] = 0.0f;  r._m[15] = 1.0f;

	return r;
}