#ifndef _PHASE_H
#define _PHASE_H

#include "color.h"

class Point3;
class Vector3;

class Phase
{
public:
	~Phase(void);
	Phase(void);

public:
	virtual float Pdfw(const Point3 &position, const Vector3 &w, const Vector3 &pdf_w) const;
	virtual Vector3 PhaseSampling(const Vector3 &direction, float *pdf_w) const;
	virtual Color Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo) const;
};


class Isotropic : public Phase
{
private:
	Color _fp;
	float _pdf_w;

public:
	~Isotropic(void);
	Isotropic(void);

private:
	Vector3 IsotropicSampling(const Vector3 &wo) const;

public:
	float Pdfw(const Point3 &position, const Vector3 &w, const Vector3 &pdf_w) const;
	Vector3 PhaseSampling(const Vector3 &direction, float *pdf_w) const;
	Color Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo) const;
};

#endif