#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include "shape.h"
#include "..\\Kernel\\vector.h"

class Triangle : public Shape
{
private:
	// Three vertices are counter clockwise 
	Point3 _v0;
	Point3 _v1;
	Point3 _v2;
	// Normal of triangle
	Vector3 _n0;
	Vector3 _n1;
	Vector3 _n2;

public:
	~Triangle(void);
	Triangle(void);
	Triangle(const Point3 vertex_0, const Point3 vertex_1, const Point3 vertex_2);
	
public:
	void Normal(const Vector3 &normal_0, const Vector3 &normal_1, const Vector3 &normal_2);
	bool Intersect(Ray *ray, Intersection *isect) const;
	BBox Bound(void) const;
};

#endif