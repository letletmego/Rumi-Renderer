#include "rect.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Sample\\rectsample.h"
#include "..\\Material\\material.h"
#include "..\\Integrator\\path.h"


Rect::~Rect(void)
{
}

Rect::Rect(const Point3 start_position, const Vector3 vector_length, const Vector3 vector_width)
	: _p(start_position)
	, _vect_l(vector_length)
	, _vect_w(vector_width)
	, _n()
{
	_n = Normalize(Cross(_vect_l, _vect_w));
	_length = ::Length(_vect_l);
	_width = ::Length(_vect_w);
	_inv_area = 1.0f / (_length * _width);
}

Rect::Rect(const Point3 start_position, const Vector3 vector_length, const Vector3 vector_width, const int n_samples)
	: _p(start_position)
	, _vect_l(vector_length)
	, _vect_w(vector_width)
	, _n()
{
	_n = Normalize(Cross(vector_length, vector_width));
	_length = ::Length(_vect_l);
	_width = ::Length(_vect_w);
	_inv_area = 1.0f / (_length * _width);

	if (n_samples > 0)
		_sampler_ptr = new RectSample(n_samples, _length, _width);
}

void Rect::GenerateSamples(void)
{
	if (_sampler_ptr == 0x00)
		return;
	
	_sampler_ptr->GenerateSamples();
	_sampler_ptr->ShuffleSamples();

	return;
}

bool Rect::Intersect(Ray *ray, Intersection *isect) const
{
	float costheta = Dot(ray->_d, _n);

	if (costheta == 0.0f || costheta == -0.0f)
		return false;

	// (RayOrigin + t * RayDirection - PlanePoint) Dot PlaneNormal = 0
	// (RayOrigin Dot PlaneNormal) + (t * RayDirection Dot PlaneNormal) - (PlanePoint Dot PlaneNormal) = 0
	// t * RayDirection Dot PlaneNormal = -(RayOrigin Dot PlaneNormal) + (PlanePoint Dot PlaneNormal)
	// t * RayDirection Dot PlaneNormal = (-RayOrigin + PlanePoint) Dot PlaneNormal
	// t = (-RayOrigin + PlanePoint) Dot PlaneNormal / (RayDirection Dot PlaneNormal)
	// t = (-(RayOriginal - PlanePoint) Dot PlaneNormal) / (RayDirection Dot PlaneNormal)
	float t = -Dot(ray->_o - _p, _n) / costheta;

	// t < 0 -> the plane is behind of the ray direction
	if (t < 0.0f)
		return false;

	// Self-intersection
	if (t < ray->_epsilon)
		return false;

	// t > ray->t_max -> The plane is behind the light source
	//if (t > _ray->t_max)
	if (ray->_tmax - t < ray->_epsilon)
		return false;

	// 0 < cos_theta < 1 -> Ray is behind the plane
	if (_material_ptr &&_material_ptr->Le() == false && (0.0f < costheta && costheta < 1.0f))
		return false;

	// Shadow ray hit the light source
	//if (!_ray->intersect && material_ptr->Le() != Color())
	//	return false;

	// t > ray->t_min > 0 -> It is not the most closest intersection
	if (t > ray->_tmin && ray->_tmin > 0.0f)
		return false;

	// t > 0.0f means the plane is in front of the ray direction
	//if (t > 0.0f && (_ray->t_min == 0.0f || t < _ray->t_min))

	Point3 intersect_p(ray->_o + ray->_d * t);

	Vector3 on_plane(intersect_p - _p);

	float isect_l = Dot(on_plane, _vect_l) * (_inv_area * _width);
	if (isect_l < 0.0f || isect_l > _length)
		return false;

	float isect_w = Dot(on_plane, _vect_w) * (_inv_area * _length);
	if (isect_w < 0.0f || isect_w > _width)
		return false;

	ray->_tmin = t;

	if (isect && isect->_vertex_ptr)
	{
		isect->_object_ptr = (Shape *)this;

		Vertex *vertex_ptr = isect->_vertex_ptr;
		vertex_ptr->_p  = intersect_p;

		//if (material_ptr->Le() == Color())
		//	v->n = costheta < 0.0f ? n : -n;
		//else
			vertex_ptr->_n = _n;
	}

	return true;
}

Point3 Rect::Position(void) const
{
	return _p;
}

Vector3 Rect::WidthVector(void) const
{
	return _vect_w;
}

Vector3 Rect::LengthVector(void) const
{
	return _vect_l;
}

Vector3 Rect::Normal(void) const
{
	return _n;
}

float Rect::Width(void) const
{
	return _width;
}

float Rect::Length(void) const
{
	return _length;
}

float Rect::Pdf(void) const
{
	return _inv_area;
}

BBox Rect::Bound(void) const
{	
	Point3 another_p = _p + _vect_l + _vect_w;

	Point3 bbox_min;
	bbox_min._x = _p._x < another_p._x ? _p._x : another_p._x;
	bbox_min._y = _p._y < another_p._y ? _p._y : another_p._y;
	bbox_min._z = _p._z < another_p._z ? _p._z : another_p._z;

	Point3 bbox_max;
	bbox_max._x = _p._x > another_p._x ? _p._x : another_p._x;
	bbox_max._y = _p._y > another_p._y ? _p._y : another_p._y;
	bbox_max._z = _p._z > another_p._z ? _p._z : another_p._z;

	return BBox(_shape_id, bbox_min, bbox_max);
}

Point3 Rect::ShapeSampling(void)
{
	Point2 sample = _sampler_ptr->Sampling();

	return _p + _vect_l * sample._x + _vect_w * sample._y;
}

void Rect::TextureMapping(const Point3 &position, float *u, float *v)
{
	Vector3 on_plane(position - _p);

	*u = Dot(on_plane, _vect_l) / (_length * _length);
	*v = Dot(on_plane, _vect_w) / (_width * _width);

	if (*u >= 1.0f) *u = 1.0f;
	if (*u <= 0.0f) *u = 0.0f;

	if (*v >= 1.0f) *v = 1.0f;
	if (*v <= 0.0f) *v = 0.0f;

	return;
}