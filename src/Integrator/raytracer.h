#ifndef _WHITTED_H
#define _WHITTED_H

#include "integrator.h"

class RayTracer : public Integrator
{
private:
	int _max_depth;

public:
	~RayTracer(void);
	RayTracer(void);
	RayTracer(Scene *scene_ptr, int max_depth);

public:
	Color Trace(Ray *ray, int trace_depth, bool le = true, bool in_object = false);
	Color Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr);
};

#endif