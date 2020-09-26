#include "sphere.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Sample\\spheresample.h"
#include "..\\Material\\material.h"
#include "..\\Integrator\\path.h"


Sphere::~Sphere(void)
{
}

Sphere::Sphere(void)
	: _r(0.0f)
	, _c()
	, _p()
{
}

Sphere::Sphere(const Point3 center, const float radius)
	: _r(radius)
	, _c(center)
	, _p()
{
	if (_r > 0.0f)
		_inv_area = 1.0f / (4.0f * PI * radius * radius);
}

Sphere::Sphere(const Point3 center, const float radius, const int n_samples)
	: _c(center)
	, _r(radius)
{
	if (_r > 0.0f)
		_inv_area = 1.0f / (4.0f * PI * radius * radius);

	if (n_samples > 0)
		_sampler_ptr = new SphereSample(n_samples);
}

void Sphere::GenerateSamples(void)
{
	if (_sampler_ptr == 0x00)
		return;
	
	_sampler_ptr->GenerateSamples();
	_sampler_ptr->ShuffleSamples();

	return;
}

bool Sphere::Intersect(Ray *ray, Intersection *isect) const
{
	// RoToSc -> Ray original position to sphere center
	Vector3 ro_to_sc(ray->_o - _c);
	// (SphereSurfacePoint - SphereCenter)^2 = SphereRadius^2
	// -> (RayOrigin + t*RayDirection - SphereCenter)^2 = SphereRadius^2
	// -> (RayOrigin + t*RayDirection - SphereCenter)^2 - SphereRadius^2 = 0
	// -> A*t^2 + B*t + C = 0
	// Direction of ray is Normalized
	float A = 1.0f; //Dot(ray->_d, ray->_d);
	float B = 2.0f * Dot(ray->_d, ro_to_sc);
	float C = Dot(ro_to_sc, ro_to_sc) - _r * _r;
	float t0 = 0.0f;
	float t1 = 0.0f;

	if (SolveQuadratic(A, B, C, &t0, &t1) == false)
		return false;

	// Sphere is behind the ray
	if (t0 < 0.0f && t1 < 0.0f)
		return false;

	// Refractive ray travel in the sphere
	// Ray is forward to the sphere and the origin of ray is very close to the sphere
	//if (t0 < ray->_epsilon && t1 > ray->_epsilon && 
	//	Dot(ray->_d, Normalize(ro_to_sc)) < 1e-4f)
	if (isect && isect->_in_object == true) 
	//if ((-ray->_epsilon < t0 && t0 < ray->_epsilon) && t1 > ray->_epsilon)
	{
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	// Image based light
	if (_material_ptr->IsLe() == true && _texture_ptr &&
		Length(ray->_o - _c) < _r) 
	{
		float temp = t0;
		t0 = t1;
		t1 = temp;
	}

	// Self-intersection
	if (t0 < ray->_epsilon)
		return false;

	// The sphere is behind the light source
	//if (t0 > _ray->t_max)
	if (ray->_tmax - t0 < ray->_epsilon)
		return false;

	// ray->t_min == 0.0f is first hit
	// t0 < ray->t_min is the ray not the first hit
	if (t0 > ray->_tmin && ray->_tmin > 0.0f)
		return false;

	ray->_tmin = t0;

	// Record the result of intersection
	if (isect && isect->_vertex_ptr)
	{
		isect->_object_ptr = (Shape *)this;
		
		Vertex *vertex_ptr = isect->_vertex_ptr;
		vertex_ptr->_p = ray->_o + ray->_d * ray->_tmin;

		if (_material_ptr->IsLe() == true)
		{
			if (Length(ray->_o - _c) > _r)
				vertex_ptr->_n = Normalize(vertex_ptr->_p - _c);
			else
				vertex_ptr->_n = Normalize(_c - vertex_ptr->_p);
		}
		else
		{
			vertex_ptr->_n = Normalize(vertex_ptr->_p - _c);
		}
	}
	
	return true;
}

float Sphere::Pdf(void) const
{
	return _inv_area;
}

BBox Sphere::Bound(void) const
{	
	Point3 p_min(_c._x - _r, _c._y - _r, _c._z - _r);
	Point3 p_max(_c._x + _r, _c._y + _r, _c._z + _r);
	
	return BBox(_shape_id, p_min, p_max);
}

Vector3 Sphere::Normal(void) const
{
	return Normalize(_p - _c);
}

Point3 Sphere::ShapeSampling(void)
{
	Point2 sample(_sampler_ptr->Sampling());

	float phi = sample._x;
	float theta = sample._y;
	float sintheta = sin(theta);
	float sampling_u = sintheta * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sintheta * cos(phi);

	Vector3 u(_transform._m[0], _transform._m[1], _transform._m[2]);
	Vector3 v(_transform._m[4], _transform._m[5], _transform._m[6]);
	Vector3 w(_transform._m[8], _transform._m[9], _transform._m[10]);

	_p = _c + (u * sampling_u + v * sampling_v + w * sampling_w) * _r;

	return _p;
}

void Sphere::TextureMapping(const Point3 &position, float *u, float *v)
{
	//Vector3 p_c(_transform * Normalize(position - _c));
	//float theta = acos(p_c._y);
	//float phi = atan2f(p_c._x , p_c._z);

	Vector3 p_c(Normalize(position - _c));
	Vector3 uu(_transform._m[0], _transform._m[1], _transform._m[2]);
	Vector3 vv(_transform._m[4], _transform._m[5], _transform._m[6]);
	Vector3 ww(_transform._m[8], _transform._m[9], _transform._m[10]);
	float theta = acos(Dot(p_c, vv));

	Vector3 pcxz(p_c * sin(theta));
	float phi = atan2f(Dot(pcxz, uu) , Dot(pcxz, ww));

	if (phi >= 0.0f)
		*u = phi / (2.0f * PI);
	else
		*u = (phi + 2.0f * PI) / (2.0f * PI);

	*v = 1.0f - theta / PI;

	return;
}