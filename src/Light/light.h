#ifndef _LIGHT_H
#define _LIGHT_H

class Color;
class Point3;
class Vector3;
class Intersection;

class Light
{
public:
	virtual ~Light(void);
	Light(void);

public:
	virtual Vector3 SolidAngleSampling(float *pdf_w);
	virtual float Pdfw(const Vector3 &w);
	virtual void Sampling(const Point3 &position, const Vector3 &normal);
	virtual Color L(const Intersection &info);
	virtual Point3 P(void);
	virtual Vector3 Wi(const Point3 &position);
	virtual Vector3 N(const Point3 &position);
	virtual float G(const Point3 &position);
	virtual float Pdf(void);
};

#endif