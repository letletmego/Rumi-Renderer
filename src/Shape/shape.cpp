#include "shape.h"
#include "..\\Kernel\\vector.h"
#include "..\\Material\\material.h"


unsigned int Shape::_next_shape_id = 1;

Shape::~Shape(void)
{
	DELETE(_texture_ptr)
	DELETE(_material_ptr)
	DELETE(_sampler_ptr)

	--_next_shape_id;
}

Shape::Shape(void)
	: _shape_id(_next_shape_id)
	, _sampler_ptr(0x00)
	, _inv_area(0.0)
	, _material_ptr(0x00)
	, _texture_ptr(0x00)
	, _transform()
	, _transform_inv()
{
	++_next_shape_id;
}

Shape::Shape(const Mat4x4 &transform)
	: _shape_id(_next_shape_id)
	, _sampler_ptr(0x00)
	, _inv_area(0.0)
	, _material_ptr(0x00)
	, _texture_ptr(0x00)
	, _transform(transform)
	, _transform_inv(Transpose(Inverse(transform)))
{
	++_next_shape_id;
}

bool Shape::AddingMaterial(Material *material_ptr)
{
	return _material_ptr = material_ptr;
}

bool Shape::AddingTexture(Texture *texture_ptr)
{
	return _texture_ptr = texture_ptr;
}

void Shape::GenerateSamples(void)
{
	return;
}

Material *Shape::MaterialPtr(void) const
{
	return _material_ptr;
}

Texture *Shape::TexturePtr(void) const
{
	return _texture_ptr;
}

float Shape::Pdf(void) const
{
	return 1.0f;
}

BBox Shape::Bound(void) const
{
	return BBox();
}

Vector3 Shape::Normal(void) const
{
	return Vector3();
}

Point3 Shape::ShapeSampling(void)
{
	return Point3();
}

void Shape::TextureMapping(const Point3 &position, float *u, float *v)
{
	*u = 0.0f;
	*v = 0.0f;

	return;
}