#ifndef _BSDF_H
#define _BSDF_H

#include "color.h"

class Vector3;
//class Sampler;

class BSDF
{
public:
	//Sampler *brdf_sampler;

public:
	~BSDF(void);
	BSDF(void);

public:
	virtual Color Rho(const Vector3 &wi, const Vector3 &wo) const = 0;
	virtual Color F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const = 0;
	virtual float Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const = 0;
	virtual Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf) = 0;
};


class Lambertian : public BSDF
{
private:
	Color _cd;

public:
	Lambertian(void);
	Lambertian(const Color cd);

public:
	Color Rho(const Vector3 &wi, const Vector3 &wo) const;
	Color F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const;
	float Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf);
};


class Phong : public BSDF
{
private:
	// Cosine power distribution
	float _e;
	// Glossy normalization
	const float _normalize;
	// Glossy color
	Color _cs;

public:
	Phong(void);
	Phong(const float e, const Color cs);

public:
	Color Rho(const Vector3 &wi, const Vector3 &wo) const;
	Color F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const;
	float Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf);
};

class PerfectReflection : public BSDF
{
private:
	Color _cr;

public:
	PerfectReflection(void);
	PerfectReflection(const Color cr);

public:
	Color Rho(const Vector3 &wi, const Vector3 &wo) const;
	Color F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const;
	float Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf);
};

class FresnelReflection : public BSDF
{
private:
	// Fresnel reflection color
	Color _cr;

public:
	FresnelReflection(void);
	FresnelReflection(const Color cr);

public:
	Color Rho(const Vector3 &wi, const Vector3 &wo) const;
	Color F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const;
	float Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf);

	float Fresnel(const Vector3 &direction, const Vector3 &normal, const float &eta_in, const float &eta_out);
};


class FresnelTransparency : public BSDF
{
private:
	Color _ct;

public:
	const Color _absorption;
	const float _eta_in;
	const float _eta_out;
	float _eta;

public:
	~FresnelTransparency(void);
	FresnelTransparency(void);
	FresnelTransparency(float eta_in, float eta_out, Color absorption);

public:
	Color Rho(const Vector3 &wi, const Vector3 &wo) const;
	Color F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const;
	float Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const;
	Vector3 BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf);
};

#endif