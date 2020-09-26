#ifndef _SPECULAR_H
#define _SPECULAR_H

#include "material.h"

class Specular : public Material
{
private:
	// Perfect reflection coefficient
	float _kr;
	BSDF *_specular_brdf;

public:
	~Specular(void);
	Specular(void);
	Specular(float kr, Color rgb);

public:
	bool IsDirac(const float &xi);
	float Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const;
	Color Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo);
	Color DirectLighting(const Intersection &info);
	Color GlobalIllumination(const Intersection &info);
};

#endif