#ifndef _Intersection_H
#define _Intersection_H

#include "color.h"
#include "vector.h"
#include "point.h"

class Scene;
class Ray;
class Shape;
class Vertex;

class Intersection
{
public:
	// Connect to the scene
	Scene *_scene_ptr;
	// Next intersection
	Intersection *_next;
	// Connect to the ray
	Ray *_ray_ptr;
	// Object of Intersection
	Shape *_object_ptr;
	// Vertex of Intersection
	Vertex *_vertex_ptr;
	// Recurive depth
	int _trace_depth;
	// Rendering equation Le
	bool _le;
	// Inside the object
	bool _in_object;
	// Sampling
	float _rand;
	// Volume factor
	struct
	{
		float _volumetric_s;
		float _volumetric_smax;
		Color _volumetric_tau;
		Color _volumetric_pdf;
	};


public:
	~Intersection(void);
	Intersection(void);
	Intersection(Ray *ray, Scene *scene_ptr);

public:
	void Clean(void);
};

#endif