#ifndef _RAY_H
#define _RAY_H

#include "vector.h"
#include "point.h"

class Intersection;

class Ray
{
public:
	// Origin of ray
	Point3 _o;
	// Direction of ray
	Vector3 _d;
	// t_min is the solution for Ro + (t) * Rd
	float _tmin;
	// t_max is the limitation of ray
	float _tmax;
	// epsilon is a bias
	static float _epsilon;

public:
	~Ray(void);
	Ray(void);
	Ray(const Point3 &origin, const Vector3 &direction);
	Ray(const Point3 &origin, const Vector3 &direction, const Point3 &light_position);

public:
	void Return(void);
};

#endif