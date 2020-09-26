#ifndef _HOMOGENEOUSVOLUME_H
#define _HOMOGENEOUSVOLUME_H

#include "..\\Kernel\\color.h"
#include "..\\Kernel\\bbox.h"

class Intersection;
class Phase;

class HomogeneousVolume
{
private:
	Phase *_phase;
	BBox _extents;
	Color _sigma_s;
	Color _sigma_a;
	Color _sigma_t;
	float _avg_sigma_t;
	float _absorb;

public:
	~HomogeneousVolume(void);
	HomogeneousVolume(void);
	HomogeneousVolume(BBox extents, Color sigma_s, Color sigma_a);

public:
	Color Sigma_t(void);
	Color ShadowRayTr(const Ray &ray);
	Color Tr(const Ray &ray, const float &tnear, const float &tfar);
	float Pdfw(const Point3 &position, const Vector3 &w, const Vector3 &w_pdf);
	void PdfDistance(const float &distance, Color *tau, Color *pdf);

	bool Intersect(const Ray &ray, float *tnear, float *tfar) const;
	float MaxDistance(const float &thit, const float &tnear, const float &tfar);
	bool DistanceSampling(const float &max_distance, float *s, bool *absorb, Color *tau, Color *pdf);
	Vector3 PhaseSampling(Intersection *info, bool *Le, bool *in_object, float *pdf_w);
	Color Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo);
	Color DirectLighting(const Intersection &info);
	Color GlobalIllumination(const Intersection &info, const float &s,  const float &smax);
};

#endif