#include "integrator.h"
#include "..\\Kernel\\color.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"


Integrator::~Integrator(void)
{	
	_scene_ptr = 0x00;
}

Integrator::Integrator(void)
	: _scene_ptr(0x00)
{
}

Integrator::Integrator(Scene *scene_ptr)
	: _scene_ptr(scene_ptr)
{
}

Color Integrator::Trace(Ray *ray, int trace_depth, bool le, bool in_object)
{
	return Color();
}

Color Integrator::Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr)
{
	return Color();
}