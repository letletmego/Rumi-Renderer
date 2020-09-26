#include "material.h"
#include "..\\Kernel\\intersection.h"


Material::~Material(void)
{
}

Material::Material(void)
{
}

Material::Material(bool opaque)
	: _opaque(opaque)
{
}

Color Material::BeerLambertLaw(const Color &sigma_t, const float &distance)
{
	Color beerlaw;
	beerlaw._b = exp(-sigma_t._b * distance);
	beerlaw._g = exp(-sigma_t._g * distance);
	beerlaw._r = exp(-sigma_t._r * distance);

	return beerlaw;
}

bool Material::IsOpaque(void) const
{
	return _opaque;
}

bool Material::IsDirac(const float &xi)
{
	return false;
}

bool Material::IsLe(void)
{
	return false;
}

bool Material::IsSubsurface(void)
{
	return false;
}

float Material::Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	return 1.0f;
}

Color Material::Le(void)
{
	return Color();
}

Vector3 Material::BSDFSampling(const Vector3 &wo, const Vector3 &normal, 
	bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const
{
	return Vector3();
}

bool Material::PhaseSampling(Intersection *info, bool *Le, bool *go_into_object, Vector3 *wi, float *pdf_w)
{
	return false;
}

Color Material::Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo)
{
	return Color();
}

Color Material::Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo)
{
	return Color();
}

Color Material::DirectLighting(const Intersection &info)
{
	return Color();
}

Color Material::GlobalIllumination(const Intersection &info)
{
	return Color();
}