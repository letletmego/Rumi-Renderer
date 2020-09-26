#ifndef _SPHERE_H
#define _SPHERE_H

#include "shape.h"

class Sphere : public Shape
{
private:
	// Radius of sphere
	float _r;
	// Center of sphere
	Point3 _c;
	// Sampling position of sphere
	Point3 _p;

public:
	~Sphere(void);
	Sphere(void);
	Sphere(const Point3 center, const float radius);
	Sphere(const Point3 center, const float radius, const int n_samples);

public:
	void GenerateSamples(void);

	bool Intersect(Ray *ray, Intersection *isect) const;

	float Pdf(void) const;
	BBox Bound(void) const;
	Vector3 Normal(void) const;

	Point3 ShapeSampling(void);
	void TextureMapping(const Point3 &position, float *u, float *v);
};

#endif