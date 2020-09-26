#ifndef _IMAGEBASEDLIGHT_H
#define _IMAGEBASEDLIGHT_H

#include "light.h"
#include "..\\Kernel\\vector.h"
#include "..\\Kernel\\point.h"
#include "..\\Kernel\\color.h"

class Shape;
class Material;
class Texture;

class ImageBasedLight : public Light
{
private:
	// Shape of light
	Shape *_object;
	// Emissive material of light
	Material *_material_ptr;
	// HDR Texture of light
	Texture *_texture_ptr;
	// Sample point on the surface
	Point3 _p;
	// Normal of the image-based light
	Vector3 _n;
	// Unit vector from intersection point to sample point
	Vector3 _wi;
	// Radiance Sampling
	Color _radiance;
	// Sampling pdf
	float _pdf;

public:
	~ImageBasedLight(void);
	ImageBasedLight(void);
	ImageBasedLight(Shape *object_ptr, Material *material_ptr, Texture *texture_ptr);

private:
	void UniformSampling(const Point3 &position, const Vector3 &normal);

public:
	void Sampling(const Point3 &position, const Vector3 &normal);
	Color L(const Intersection &info);
	Point3 P(void);
	Vector3 Wi(const Point3 &position);
	Vector3 N(const Point3 &position);
	float G(const Point3 &position);
	float Pdf(void);
};

#endif