#ifndef _BBOX_H
#define _BBOX_H

#include "point.h"

class Ray;

class BBox
{
public:
	// Id of bounding box
	unsigned int _id;
	// Region of bounding box
	Point3 _pmin;
	Point3 _pmax;

public:
	~BBox(void);
	BBox(void);
	BBox(Point3 pmin, Point3 pmax);
	BBox(const unsigned int id, const Point3 &pmin, const Point3 &pmax);

public:
	bool Intersect(const Ray &ray, float *tnear, float *tfar) const;
	float Area(void) const;
	Point3 Center(void) const;
	int MaxDim(void);
};

BBox Combine(const BBox &bbox_1, const BBox &bbox_2);

#endif