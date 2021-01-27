#include "arealight.h"
#include "..\\Shape\\shape.h"
#include "..\\Material\\material.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Texture\\texture.h"

AreaLight::~AreaLight(void)
{
	_object = 0x00;
	_material_ptr= 0x00;
}

AreaLight::AreaLight(void)
	: _object(0x00)
	, _material_ptr(0x00)
	, _p()
	, _n()
	, _wi()
{
}

AreaLight::AreaLight(Shape *object_ptr, Material *material_ptr)
	: _object(object_ptr)
	, _material_ptr(material_ptr)
	, _p()
	, _n()
	, _wi()
{
}

Vector3 AreaLight::SolidAngleSampling(float *pdf_w)
{
	_p = _object->ShapeSampling();
	_n = _object->Normal();

	Vector3 v(_n);
	Vector3 u;
	Vector3 w;
	OrthonormalBasis(v, &u, &w);

	float phi = RandomFloat() * 2.0f * PI;
	//float theta = acos(sqrt(1.0f - RandomFloat()));
	float theta = acos(1.0f - RandomFloat() * 2.0f) * 0.5f;
	float sintheta = sin(theta);
	float sampling_u = sintheta * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sintheta * cos(phi);

	Vector3 wi(u * sampling_u + v * sampling_v + w * sampling_w);

	float costheta = Dot(wi, _n);
	if (costheta <= 0.0f)
		costheta = 1e-4f;

	*pdf_w = INV_PI * costheta;
	if (*pdf_w <= 0.0f)
		*pdf_w = 1e-4f;

	return wi;
}

float AreaLight::Pdfw(const Vector3 &w)
{
	float costheta = Dot(w, _n);
	if (costheta <= 0.0f)
		costheta = 1e-4f;

	float pdf = INV_PI * costheta;
	if (pdf <= 0.0f)
		pdf = 1e-4f;

	return pdf;
}

void AreaLight::Sampling(const Point3 &position, const Vector3 &normal)
{
	_p = _object->ShapeSampling();

	return;
}

Color AreaLight::L(const Intersection &info)
{
	return _material_ptr->Le();
}

Point3 AreaLight::P(void)
{
	return _p;
}

Vector3 AreaLight::Wi(const Point3 &position)
{
	_wi = Normalize(_p - position);

	return _wi;
}

Vector3 AreaLight::N(const Point3 &position)
{
	_n  = _object->Normal();

	return _n;
}

float AreaLight::G(const Point3 &position)
{
	float distance = Length(position - _p);
	float costheta = __max(-Dot(_wi, _n), 0.0f);

	return costheta / (distance * distance);
}

float AreaLight::Pdf(void)
{
	return _object->Pdf();
}