#include "pointLight.h"
#include "..\\Kernel\\intersection.h"


PointLight::~PointLight(void)
{
}

PointLight::PointLight(void)
	: _p()
	, _ls(0.0f)
	, _cl()
{
}

PointLight::PointLight(const Point3 position, const float ls, const Color rgb)
	: _p(position)
	, _ls(ls)
	, _cl(rgb)
{
}

Vector3 PointLight::SolidAngleSampling(float *pdf_w)
{
	Vector3 u(1.0f, 0.0f, 0.0f);
	Vector3 v(0.0f, 1.0f, 0.0f);
	Vector3	w(1.0f, 0.0f, 1.0f);

	float phi = RandomFloat() * 2.0f * PI;
	float theta = acos(RandomFloat() * 2.0f - 1.0f);
	float sintheta = sin(theta);
	float sampling_u = sintheta * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sintheta * cos(phi);

	Vector3 wi(u * sampling_u + v * sampling_v + w * sampling_w);
	*pdf_w = 0.25f * INV_PI; 

	return wi;
}

Color PointLight::L(const Intersection &info)
{
	return _cl * _ls;
}

Point3 PointLight::P(void)
{
	return _p;
}

Vector3 PointLight::Wi(const Point3 &position)
{
	return Normalize(_p - position);
}

Vector3 PointLight::N(const Point3 &position)
{
	return Normalize(position - _p);
}

float PointLight::G(const Point3 &position)
{
	float distance = Length(_p - position);

	return 1.0f / (distance * distance);
}

float PointLight::Pdf(void)
{
	return 1.0f;
}