#ifndef _POINTLIGHT_H
#define _POINTLIGHT_H

#include "light.h"
#include "..\\Kernel\\color.h"
#include "..\\Kernel\\vector.h"
#include "..\\Kernel\\point.h"

class Ray;

class PointLight : public Light
{
private:
	// Position of point light
	Point3 _p;
	// Light scaling
	float _ls;
	// Color of the point light
	Color _cl;


public:
	~PointLight(void);
	PointLight(void);
	PointLight(const Point3 position, const float ls, const Color rgb);

public:
	Vector3 SolidAngleSampling(float *pdf_w);
	Color L(const Intersection &info);
	Point3 P(void);
	Vector3 Wi(const Point3 &position);
	Vector3 N(const Point3 &position);
	float G(const Point3 &position);
	float Pdf(void);
};

#endif