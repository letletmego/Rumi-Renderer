#include "ambient.h"
#include "..\\Kernel\\point.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\shape.h"
#include "..\\Material\\material.h"
#include "..\\Sample\\hemispheresample.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Integrator\\path.h"
#include "..\\Scene\\scene.h"


Ambient::~Ambient(void)
{
	DELETE(_ao_sampler)
}

Ambient::Ambient(void)
	: _ls(0.0f)
	, _cl()
	, _ao(0.0f)
	, _ao_sampler(0x00)
{
}

Ambient::Ambient(const float ls, const Color cl)
	: _ls(ls)
	, _cl(cl)
	, _ao(1.0f)
	, _ao_sampler(new HemisphereSample())
{
}

Ambient::Ambient(const float ls, const Color cl, const bool  ambient_occlusion)
	: _ls(ls)
	, _cl(cl)
	, _ao(1.0f)
{
	_ao_sampler = ambient_occlusion ? new HemisphereSample(1, 0) : new HemisphereSample();
}

float Ambient::AmbientOcclusion(const Intersection &info)
{
	if (_ao_sampler->SampleCount() < 1)
		return 1.0f;

	Vector3 v(info._vertex_ptr->_n);
	Vector3 u;
	Vector3 w;
	OrthonormalBasis(v, &u, &w);

	// Hemisphere Sampling
	_ao_sampler->GenerateSamples();
	Point2 sample = _ao_sampler->Sampling();
	float phi = sample._x;
	float theta = sample._y;

	float sampling_u = sin(theta) * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sin(theta) * cos(phi);

	Vector3 d = Normalize(u * sampling_u + v * sampling_v + w * sampling_w);
	Ray ao_ray(info._vertex_ptr->_p, d);
	ao_ray._tmax = Dot(d, info._vertex_ptr->_n);

	info._scene_ptr->_bvh_ptr->Travel(&ao_ray, 0x00);

	_ao = 1.0f;
	if (ao_ray._tmin != 0.0f)
		_ao = 0.0f;

	return _ao;
}

Color Ambient::L(const Intersection &info)
{
	return _cl * _ls * AmbientOcclusion(info);
}

Point3 Ambient::P(void)
{
	return Point3();
}

Vector3 Ambient::Wi(const Point3 &position)
{
	return Vector3();
}

Vector3 Ambient::N(const Point3 &position)
{
	return Vector3();
}

float Ambient::G(const Point3 &position)
{
	return 1.0f;
}

float Ambient::Pdf(void)
{
	return 1.0f;
}