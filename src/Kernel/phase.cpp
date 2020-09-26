#include "phase.h"
#include "vector.h"


Phase::~Phase(void)
{
}

Phase::Phase(void)
{
}

float Phase::Pdfw(const Point3 &position, const Vector3 &w, const Vector3 &pdf_w) const
{
	return 1.0f;
}

Vector3 Phase::PhaseSampling(const Vector3 &direction, float *pdf_w) const
{
	return Vector3(0.0f);
}

Color Phase::Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo) const
{
	return Color(1.0f);
}


// Isotropic phase
Isotropic::~Isotropic(void)
{
}

Isotropic::Isotropic(void)
	: _fp(1.0f / (4.0f * PI))
	, _pdf_w(1.0f / (4.0f * PI))
{
}

Vector3 Isotropic::IsotropicSampling(const Vector3 &wo) const
{
	Vector3 v(-wo);
	Vector3 u, w;
	OrthonormalBasis(v, &u, &w);

	float phi = RandomFloat() * 2.0f * PI;
	float theta = acos(RandomFloat() * 2.0f - 1.0f);
	float sintheta = sin(theta);
	float sampling_u = sintheta * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sintheta * cos(phi);

	return u * sampling_u + v * sampling_v + w * sampling_w;
}

float Isotropic::Pdfw(const Point3 &position, const Vector3 &w, const Vector3 &pdf_w) const
{
	return _pdf_w;
}

Vector3 Isotropic::PhaseSampling(const Vector3& direction, float *pdf_w) const
{
	*pdf_w = _pdf_w;

	return IsotropicSampling(direction);
}

Color Isotropic::Fp(const Point3& position, const Vector3& wi, const Vector3& wo) const
{
	return _fp;
}