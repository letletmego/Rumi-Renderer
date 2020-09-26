 #include "emissive.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Integrator\\path.h"


Emissive::~Emissive(void)
{
}

Emissive::Emissive(void)
	: Material(true)
	, _ls(0.0f)
	, _ce()
	, _in_object(false)
{
}

Emissive::Emissive(const float ls, const Color ce)
	: Material(true)
	, _ls(ls) 
	, _ce(ce)
	, _in_object(false)
{
}

bool Emissive::IsLe(void)
{
	return true;
}

Color Emissive::Le(void)
{
	return _ce * _ls;
}

Vector3 Emissive::BSDFSampling(const Vector3 &wo, const Vector3 &normal, 
	bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const
{
	*Le = false;
	*go_into_object = false;
	*xi = 1.0f;
	*pdf_path = 1.0f;
	*costheta = 1.0f;
	*pdf_w = 1.0f;

	return Vector3();
}

Color Emissive::Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo)
{
	if (info._in_object != _in_object)
		return Color();

	if (Dot(normal, wo) <= 0.0f)
		return Color();

	return _ce * _ls;
}

Color Emissive::DirectLighting(const Intersection &info)
{
	if (info._in_object != _in_object)
		return Color();

	if (info._le == false)
		return Color();

	// costheta test
	if (-Dot(info._ray_ptr->_d, info._vertex_ptr->_n) <= 0.0f)
		return Color();

	return _ce * _ls;
}