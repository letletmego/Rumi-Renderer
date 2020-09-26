#ifndef _MATERIAL_H
#define _MATERIAL_H

class Point3;
class Vector3;
class Color;
class Intersection;
class BSDF;

class Material
{
private:
	bool _opaque;

public:
	virtual ~Material(void);
	Material(void);
	Material(bool through);

protected:
	Color BeerLambertLaw(const Color &sigma_t, const float &distance);

public:
	bool IsOpaque(void) const;
	virtual bool IsDirac(const float &xi);
	virtual bool IsLe(void);
	virtual bool IsSubsurface(void);
	virtual float Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	virtual Color Le(void);
	virtual Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const;
	virtual bool PhaseSampling(Intersection *info, bool *Le, bool *go_into_object, Vector3 *wi, float *pdf_w);
	virtual Color Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo);
	virtual Color Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo);
	virtual Color DirectLighting(const Intersection &info);
	virtual Color GlobalIllumination(const Intersection &info);
};

#endif