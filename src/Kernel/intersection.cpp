#include "intersection.h"
#include "ray.h"


Intersection::~Intersection(void)
{
}

Intersection::Intersection(void)
	: _scene_ptr(0x00)
	, _next(0x00)
	, _ray_ptr(0x00)
	, _object_ptr(0x00)
	, _vertex_ptr(0x00)
	, _trace_depth(0)
	, _le(false)
	, _in_object(false)
	, _rand(1.0f)
	, _volumetric_s(0.0f)
	, _volumetric_smax(0.0f)
	, _volumetric_tau(1.0f)
	, _volumetric_pdf(1.0f)
{
}

Intersection::Intersection(Ray *ray, Scene *scene_ptr)
	: _scene_ptr(scene_ptr)
	, _next(0x00)
	, _ray_ptr(ray)
	, _object_ptr(0x00)
	, _vertex_ptr(0x00)
	, _trace_depth(0)
	, _le(false)
	, _in_object(false)
	, _rand(1.0f)
	, _volumetric_s(0.0f)
	, _volumetric_smax(0.0f)
	, _volumetric_tau(1.0f)
	, _volumetric_pdf(1.0f)
{
}

void Intersection::Clean(void)
{
	_object_ptr = 0x00;

	_le = false;
	_in_object = false;
	_rand = 1.0f;
	_volumetric_s = 0.0f;
	_volumetric_smax = 0.0f;
	_volumetric_tau = 1.0f;
	_volumetric_pdf = 1.0f;

	return;
}