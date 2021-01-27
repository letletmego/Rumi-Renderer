#ifndef _AREALIGHT_H
#define _AREALIGHT_H

#include "light.h"
#include "..\\Kernel\\vector.h"
#include "..\\Kernel\\point.h"

class Shape;
class Material;

class AreaLight : public Light
{
private:
	// Shape of area light
	Shape *_object;
	// Emissive material of area light
	Material *_material_ptr;
	// Sample point on the surface
	Point3 _p;
	// Normal of the area light
	Vector3 _n;
	// Unit vector from intersection point to sample point
	Vector3 _wi;

public:
	~AreaLight(void);
	AreaLight(void);
	AreaLight(Shape *object_ptr, Material *material_ptr);

public:
	Vector3 SolidAngleSampling(float *pdf_w);
	float Pdfw(const Vector3 &w);
	void Sampling(const Point3 &position, const Vector3 &normal);
	Color L(const Intersection &info);
	Point3 P(void);
	Vector3 Wi(const Point3 &position);
	Vector3 N(const Point3 &position);
	float G(const Point3 &position);
	float Pdf(void);
};

#endif