#ifndef _SUBSURFACE_H
#define _SUBSURFACE_H

#include "material.h"
#include "..\\Kernel\\color.h"

class Vector3;
class FresnelReflection;
class FresnelTransparency;
class Isotropic;


class Subsurface : public Material
{
private:
	FresnelReflection *_fresnel_brdf;
	FresnelTransparency *_fresnel_btdf;
	Isotropic *_phase;
	Color _sigma_s;
	Color _sigma_a;
	Color _sigma_t;
	float _avg_sigmat;
	float _absorb;

public:
	~Subsurface(void);
	Subsurface(void);
	Subsurface(float eta_in, float eta_out, Color sigma_s, Color sigma_a, float density = 1.0f);

private:
	bool DistanceSampling(const float &tnear, const float &tfar, bool *absorb, float *s, float *smax, Color *tau, Color *pdf);

public:
	bool IsSubsurface(void);
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const;
	bool PhaseSampling(Intersection *info, bool *Le, bool *go_into_object, Vector3 *wi, float *pdf_w);
	Color Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo);
	Color Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo);
	Color DirectLighting(const Intersection &info);
	Color GlobalIllumination(const Intersection &info);
};

#endif