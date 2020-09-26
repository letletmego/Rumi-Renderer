#ifndef _GLOSSY_H
#define _GLOSSY_H

#include "matte.h"

class Glossy : public Matte
{
private:
	float _ks;
	float _kd_pdf;
	BSDF *_phong_brdf;

public:
	~Glossy(void);
	Glossy(void);
	Glossy(const float kd, const float ks, const float e, const Color rgb);

public:
	bool IsDirac(const float &xi);
	float Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const;
	Color Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo);
	Color DirectLighting(const Intersection &info);
	Color GlobalIllumination(const Intersection &info);
};

#endif