#ifndef _RECT_H
#define _RECT_H

#include "shape.h"
#include "..\\Kernel\\vector.h"

class Rect : public Shape
{
private:
	// Start position of ractangle
	Point3 _p;
	// Direction of length
	Vector3 _vect_l;
	// Direction of width
	Vector3 _vect_w;
	// Normal of ractangle
	Vector3 _n;
	// Length of rectangle
	float _length;
	// Width of rectangle
	float _width;

public:
	~Rect(void);
	Rect(const Point3 start_position, const Vector3 vector_length, const Vector3 vector_width);
	Rect(const Point3 start_position, const Vector3 vector_length, const Vector3 vector_width, const int n_samples);

public:
	void GenerateSamples(void);

	bool Intersect(Ray *ray, Intersection *isect) const;

	Point3 Position(void) const;
	Vector3 WidthVector(void) const;
	Vector3 LengthVector(void) const;
	Vector3 Normal(void) const;
	float Width(void) const;
	float Length(void) const;
	float Pdf(void) const;
	BBox Bound(void) const;

	Point3 ShapeSampling(void);
	void TextureMapping(const Point3 &position, float *u, float *v);
};

#endif