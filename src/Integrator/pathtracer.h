#ifndef _PATH_TRACER
#define _PATH_TRACER

#include "integrator.h"

class Path;

class PathTracer : public Integrator
{
private:
	int _max_depth;
	Path *_paths;

public:
	~PathTracer(void);
	PathTracer(void);
	PathTracer(Scene *scene_ptr, int max_depth);

public:
	Color Trace(Ray *ray, int trace_depth, bool le = true, bool in_object = false);
	Color Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr);
};

#endif