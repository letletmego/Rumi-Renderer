#include "ray.h"


float Ray::_epsilon = 1e-4f;

Ray::~Ray(void)
{
}

Ray::Ray(void)
	: _o()
	, _d()
	, _tmin(0.0f)
	, _tmax(FLT_MAX)
{
}

Ray::Ray(const Point3 &origin, const Vector3 &direction)
	: _o(origin)
	, _d(direction)
	, _tmin(0.0f)
	, _tmax(FLT_MAX)
{
}

Ray::Ray(const Point3 &origin, const Vector3 &direction, const Point3 &light_position)
	: _o(origin)
	, _d(direction)
	, _tmin(0.0f)
	, _tmax(Length(light_position - origin) - 1e-4f)
{
}

void Ray::Return(void)
{
	_o._x = 0.0f;
	_o._y = 0.0f;
	_o._z = 0.0f;

	_d._x = 0.0f;
	_d._y = 0.0f;
	_d._z = 0.0f;

	_tmin = 0.0f;
	_tmax = FLT_MAX;

	return;
}