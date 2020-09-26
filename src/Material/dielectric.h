#ifndef _DIELECTRIC_H
#define _DIELECTRIC_H

#include "material.h"

class FresnelReflection;
class FresnelTransparency;

class Dielectric : public Material
{
private:
	FresnelReflection *_fresnel_brdf;
	FresnelTransparency *_fresnel_btdf;

public:
	~Dielectric(void);
	Dielectric(void);
	Dielectric(float eta_in, float eta_out, Color absorption);

public:
	bool IsDirac(const float &xi);
	float Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const;
	Color Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo);
	Color DirectLighting(const Intersection &info);
	Color GlobalIllumination(const Intersection &info);
};

#endif