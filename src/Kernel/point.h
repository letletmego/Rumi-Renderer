#ifndef _POINT_H
#define _POINT_H

class Vector3;

class Point2
{
public:
	float _x;
	float _y;

public:
	~Point2(void);
	Point2(void);
	Point2(const float &x, const float &y);
};

class Point3
{
public:
	float _x;
	float _y;
	float _z;

public:
	~Point3(void);
	Point3(void);
	Point3(const float &value);
	Point3(const float &x, const float &y, const float &z);

	Point3 operator + (const Vector3 &vector) const;
	Point3 operator - (const Vector3 &vector) const;
	Vector3 operator - (const Point3 &point) const;
	Point3 operator * (const float &scalar) const;
	float operator [] (const int &index) const;
	bool IsNaN(void) const;
	bool IsInf(void) const;
};

#endif