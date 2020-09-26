#ifndef _MATTE_H
#define _MATTE_H

#include "material.h"

class Matte : public Material
{
protected:
	// Coefficient of matte
	float _kd;
	// Lambertian brdf
	BSDF *_diffuse_brdf;

public:
	~Matte(void);
	Matte(void);
	Matte(const float kd, const Color cd);

public:
	float Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const;
	Color Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo);
	Color DirectLighting(const Intersection &info);
	Color GlobalIllumination(const Intersection &info);
};

#endif