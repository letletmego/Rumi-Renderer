#include "light.h"
#include "..\\Kernel\\intersection.h"


Light::~Light(void)
{
}

Light::Light(void)
{
}

Vector3 Light::SolidAngleSampling(float *pdf_w)
{
	return Vector3(0.0f);
}

float Light::Pdfw(const Vector3 &w)
{
	return 1.0f;
}

void Light::Sampling(const Point3 &position, const Vector3 &normal)
{
	return;
}

Color Light::L(const Intersection &info)
{
	return Color();
}

Point3 Light::P(void)
{
	return Point3();
}

Vector3 Light::Wi(const Point3 &position)
{
	return Vector3();
}

Vector3 Light::N(const Point3 &position)
{
	return Vector3();
}

float Light::G(const Point3 &position)
{
	return 1.0f;
}

float Light::Pdf(void)
{
	return 1.0f;
}

