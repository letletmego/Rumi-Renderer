#include "imagebasedlight.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\shape.h"
#include "..\\Texture\\texture.h"

ImageBasedLight::~ImageBasedLight(void)
{
}

ImageBasedLight::ImageBasedLight(void)
	: _object(0x00)
	, _material_ptr(0x00)
	, _texture_ptr(0x00)
	, _p()
	, _n()
	, _wi()
	, _radiance()
	, _pdf(0.0f)
{
}

ImageBasedLight::ImageBasedLight(Shape *object_ptr, Material *material_ptr, Texture *texture_ptr)
	: _object(object_ptr)
	, _material_ptr(material_ptr)
	, _texture_ptr(texture_ptr)
	, _p()
	, _n()
	, _wi()
	, _radiance()
	, _pdf(0.0f)
{
}

void ImageBasedLight::UniformSampling(const Point3 &position, const Vector3 &normal)
{
	Vector3 v(normal), u, w;
	OrthonormalBasis(v, &u, &w);

	float phi = RandomFloat() * 2.0f * PI;
	float theta = acos(sqrt(1.0f - RandomFloat()));
	float sintheta = sin(theta);
	float sampling_u = sintheta * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sintheta * cos(phi);
	_wi = u * sampling_u + v * sampling_v + w * sampling_w;

	Ray r(position, _wi);
	_object->Intersect(&r, 0x00);

	_p = r._o + r._d * r._tmin;
	_n = -_wi;
	
	_radiance = _texture_ptr->Pattern(_object, _p);

	float costheta = Dot(_wi, normal);
	if (costheta <= 0.0f)
		costheta = 1e-4f;

	_pdf = INV_PI * costheta;
	if (_pdf <= 0.0f)
		_pdf = 1e-4f;

	return;
}

void ImageBasedLight::Sampling(const Point3 &position, const Vector3 &normal)
{
	UniformSampling(position, normal);

	return;
}

Color ImageBasedLight::L(const Intersection &info)
{
	return _radiance;
}

Point3 ImageBasedLight::P(void)
{
	return _p;
}

Vector3 ImageBasedLight::Wi(const Point3 &position)
{
	return _wi;
}

Vector3 ImageBasedLight::N(const Point3 &position)
{
	return _n;
}

float ImageBasedLight::G(const Point3 &position)
{
	return 1.0f;
}

float ImageBasedLight::Pdf(void)
{
	return _pdf;
}