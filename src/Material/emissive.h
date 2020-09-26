#ifndef _EMISSIVE_H
#define _EMISSIVE_H

#include "material.h"
#include "..\\Kernel\\color.h"

class Emissive : public Material
{
private:
	// Light scailing
	float _ls;
	// Emissive radiance
	Color _ce;
	// Inside or outside
	bool _in_object;

public:
	~Emissive(void);
	Emissive(void);
	Emissive(const float ls, const Color ce);

	bool IsLe(void);
	Color Le(void);
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const;
	Color Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo);
	Color DirectLighting(const Intersection &info);
};

#endif