#include "triangle.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Integrator\\path.h"
#include "..\\Material\\material.h"


Triangle::~Triangle(void)
{
}

Triangle::Triangle(void)
	: _v0()
	, _v1()
	, _v2()
	, _n0()
	, _n1()
	, _n2()
{
}

Triangle::Triangle(const Point3 vertex_0, const Point3 vertex_1, const Point3 vertex_2)
	: _v0(vertex_0)
	, _v1(vertex_1)
	, _v2(vertex_2)
	, _n0()
	, _n1()
	, _n2()
{
}

void Triangle::Normal(const Vector3 &normal_0, const Vector3 &normal_1, const Vector3 &normal_2)
{
	_n0 = normal_0;
	_n1 = normal_1;
	_n2 = normal_2;

	return;
}

bool Triangle::Intersect(Ray *ray, Intersection *isect) const
{
	Vector3 edge_1(_v1 - _v0);
	Vector3 edge_2(_v2 - _v0);

	Vector3 rayd_x_edge2(Cross(ray->_d, edge_2));

	// determinant > 0 -> Ray is in front of the triangle
	// determinant < 0 -> Ray is in back of the triangle
	float determinant = Dot(rayd_x_edge2, edge_1);

	// Ray is parallel with the triangle
	if (-1e-8f < determinant && determinant < 1e-8f)
	//if (determinant == 0.0f)
		return false;

	// barycentric coordinate -> P = w*v0 + v*v1 + u*v2
	// u + v + w = 1
	// Point in triangle = (1 - u - v)*v0 + v*v1 + u*v2
	// Point in triangle = v0 - u*v0 - v*v0 + v*v1 + u*v2
	// Point in triangle = v0 + u*(v2 - v0) + v*(v1 - v0)
	// Point in triangle = Ro + t*Rd = v0 + u*(v2 - v0) + v*(v1 - v0)
	// Ro - v0 = -t*d + u*(v2 - v0) + v*(v1 - v0)
	// Ro - v0 = -t*d + u*(edge_2) + v*(edge_1)

	//                               |t|
	// Ro - v0 = [-d, edge_2, edge_1]|u|
	//                               |v|
	// -> Solve by Cramer's rule

	float inv_det = 1.0f / determinant;

	Vector3 t_vec(ray->_o - _v0);

	float u = Dot(rayd_x_edge2, t_vec) * inv_det;

	if (u < 0.0f || u > 1.0f)
		return false;

	Vector3 tvec_x_edge1(Cross(t_vec, edge_1));

	float v = Dot(tvec_x_edge1, ray->_d) * inv_det;

	if (v < 0.0f || u + v > 1.0f)
		return false;

	float t = Dot(tvec_x_edge1, edge_2) * inv_det;

	// t < 0.0f -> Ray direction is away from the triangle
	// Self-intersection
	if (t < ray->_epsilon)
		return false;

	// Check the t value whether is the smallest or not
	if (ray->_tmin >  ray->_epsilon && t > ray->_tmin)
		return false;

	// t > ray->t_max -> The triangle is behind the light source
	//if (t > _ray->t_max)
	if (ray->_tmax - t < ray->_epsilon)
		return false;

	ray->_tmin = t;

	if (isect && isect->_vertex_ptr)
	{
		isect->_object_ptr = (Shape *)this;

		Vertex *vertex_ptr = isect->_vertex_ptr;
		vertex_ptr->_p = ray->_o + ray->_d * t;
		vertex_ptr->_n = _n0 * (1.0f - u - v) + _n1 * u + _n2 * v;

		if (determinant < 0.0f && _material_ptr->IsOpaque() == true)
			vertex_ptr->_n = -vertex_ptr->_n;
	}

	return true;
}

BBox Triangle::Bound(void) const
{
	Point3 p_min;
	p_min._x = _v0._x < _v1._x ? _v0._x : _v1._x;
	if (p_min._x > _v2._x)
		p_min._x = _v2._x;

	p_min._y = _v0._y < _v1._y ? _v0._y : _v1._y;
	if (p_min._y > _v2._y)
		p_min._y = _v2._y;

	p_min._z = _v0._z < _v1._z ? _v0._z : _v1._z;
	if (p_min._z > _v2._z)
		p_min._z = _v2._z;

	Point3 p_max;
	p_max._x = _v0._x > _v1._x ? _v0._x : _v1._x;
	if (p_max._x < _v2._x)
		p_max._x = _v2._x;

	p_max._y = _v0._y > _v1._y ? _v0._y : _v1._y;
	if (p_max._y < _v2._y)
		p_max._y = _v2._y;

	p_max._z = _v0._z > _v1._z ? _v0._z : _v1._z;
	if (p_max._z < _v2._z)
		p_max._z = _v2._z;

	return BBox(_shape_id, p_min, p_max);
}