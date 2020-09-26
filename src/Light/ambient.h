#ifndef _AMBIENT_H
#define _AMBIENT_H

#include "light.h"
#include "..\\Kernel\\color.h"

class Sampler;

class Ambient : public Light
{
private:
	// Light scaling
	float _ls;
	// Color of ambient light
	Color _cl;
	// Ambient occlusion
	float _ao;
	// Samples of phi and theta for ambient occlusion
	Sampler *_ao_sampler;

public:
	~Ambient(void);
	Ambient(void);
	Ambient(const float ls, const Color cl);
	Ambient(const float ls, const Color cl, const bool ambient_occlusion);

private:
	float AmbientOcclusion(const Intersection &info);

public:
	Color L(const Intersection &info);
	Point3 P(void);
	Vector3 Wi(const Point3 &position);
	Vector3 N(const Point3 &position);
	float G(const Point3 &position);
	float Pdf(void);
};

#endif