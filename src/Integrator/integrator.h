#ifndef _TRACER_H
#define _TRACER_H


class Point3;
class Vector3;
class Color;
class Ray;
class Pinhole;
class Light;
class Film;
class Scene;
class Intersection;

class Integrator
{
protected:
	Scene *_scene_ptr;

public:
	virtual ~Integrator(void);
	Integrator(void);
	Integrator(Scene *scene_ptr);

public:
	virtual Color Trace(Ray *ray, int trace_depth, bool le = true, bool in_object = false);
	virtual Color Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr);
};


#endif // _TRACER_H