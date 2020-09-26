#include "raytracer.h"
#include "path.h"
#include "..\\Camera\\pinhole.h"
#include "..\\Camera\\film.h"
#include "..\\Sample\\sampler.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\shape.h"
#include "..\\Light\\light.h"
#include "..\\Light\\ambient.h"
#include "..\\Material\\material.h"
#include "..\\Material\\specular.h"
#include "..\\Material\\dielectric.h"
#include "..\\Texture\\texture.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Scene\\scene.h"


RayTracer::~RayTracer(void)
{
	DELETE(_scene_ptr->_ambient)
}

RayTracer::RayTracer(void)
	: Integrator()
	, _max_depth(0)
{
}

RayTracer::RayTracer(Scene *scene_ptr, int max_depth)
	: Integrator(scene_ptr)
	, _max_depth(max_depth)
{
	_scene_ptr->_ambient = new Ambient(0.35f, Color(1.0f, 1.0f, 1.0f), true);
}

Color RayTracer::Trace(Ray *ray, int trace_depth, bool le, bool in_object)
{
	Color lo;

	if (trace_depth > _max_depth)
		return lo;

	Vertex vertex;
	Intersection info(ray, _scene_ptr);
	info._vertex_ptr = &vertex;
	info._trace_depth = trace_depth;
	info._le = le;
	info._in_object = in_object;

	int intersect_idx = _scene_ptr->_bvh_ptr->Travel(ray, &info);

	if (info._object_ptr == 0x00)
		return Color();

	Shape *hit_object = info._object_ptr;
	Material *material_ptr = hit_object->MaterialPtr();
	Texture *texture_ptr = hit_object->TexturePtr();

	if (material_ptr)
	{
		if (typeid(Specular) == typeid(*material_ptr) ||
			typeid(Dielectric) == typeid(*material_ptr))
			lo = material_ptr->GlobalIllumination(info);
		else
			lo = material_ptr->DirectLighting(info);		
	}

	if (texture_ptr)
		lo *= texture_ptr->Pattern(hit_object, vertex._p);

	return lo;
}

Color RayTracer::Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr)
{
	Ray cast;
	
	camera_ptr->CastViewRay(
		image_ptr->Width(), image_ptr->Height(), width_idx, height_idx, 
		image_ptr->PixelSampler()->Sampling(), &cast._o, &cast._d, 0x00, 0x00);

	Color lo = Trace(&cast, 0);

	return lo;
}