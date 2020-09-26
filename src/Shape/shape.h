#ifndef _SHAPE_H
#define _SHAPE_H

#include "..\\Kernel\\bbox.h"
#include "..\\Kernel\\matrix.h"

class Point3;
class Vector3;
class Intersection;
class Material;
class Texture;
class Sampler;

class Shape
{
private:
	static unsigned int _next_shape_id;

protected:
	// Id of shape
	const unsigned int _shape_id;
	// Material of shape
	Material *_material_ptr;
	// Texture of shape
	Texture *_texture_ptr;
	// Sampling of shape
	Sampler *_sampler_ptr;
	// Probability
	float _inv_area;

public:
	// Transform
	Mat4x4 _transform;
	Mat4x4 _transform_inv;

public:
	virtual ~Shape(void);
	Shape(void);
	Shape(const Mat4x4 &transform);

public:
	virtual bool AddingMaterial(Material *material_ptr);
	virtual bool AddingTexture(Texture *texture_ptr);
	virtual void GenerateSamples(void);

	virtual Material *MaterialPtr(void) const;
	virtual Texture *TexturePtr(void) const;

	virtual bool Intersect(Ray *ray, Intersection *isect) const = 0;

	virtual float Pdf(void) const;
	virtual BBox Bound(void) const;
	virtual Vector3 Normal(void) const;

	virtual Point3 ShapeSampling(void);
	virtual void TextureMapping(const Point3 &position, float *u, float *v);
};

#endif