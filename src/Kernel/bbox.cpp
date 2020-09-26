#include "bbox.h"
#include "ray.h"


BBox::~BBox(void)
{
}

BBox::BBox(void)
	: _id(0)
	, _pmin(Point3(FLT_MAX, FLT_MAX, FLT_MAX))
	, _pmax(Point3(-FLT_MAX, -FLT_MAX, -FLT_MAX))
{
}

BBox::BBox(Point3 pmin, Point3 pmax)
	: _id(0)
	, _pmin(pmin)
	, _pmax(pmax)
{
}

BBox::BBox(const unsigned int id, const Point3 &pmin, const Point3 &pmax)
	: _id(id)
	, _pmin(pmin)
	, _pmax(pmax)
{
}

bool BBox::Intersect(const Ray &ray, float *tnear, float *tfar) const
{
	// Test intersection with bounding box x axis
	// Rox + t * Rdx = BBx_axis
	// t = (BBx_axis - Rox) / Rdx
	float inv_ray_x_dir = 1.0f / ray._d._x;
	float inv_ray_y_dir = 1.0f / ray._d._y;
	float inv_ray_z_dir = 1.0f / ray._d._z;

	float t0 = (_pmin._x - ray._o._x) * inv_ray_x_dir;
	float t1 = (_pmax._x - ray._o._x) * inv_ray_x_dir;
	float temp;

	if (t0 > t1)
	{
		temp = t0;
		t0 = t1;
		t1 = temp;
	}

	*tnear = t0;
	*tfar  = t1;

	// Test intersection with bounding box y axis
	t0 = (_pmin._y - ray._o._y) * inv_ray_y_dir;
	t1 = (_pmax._y - ray._o._y) * inv_ray_y_dir;
	
	if (t0 > t1)
	{
		temp = t0;
		t0 = t1;
		t1 = temp;
	}

	if (*tnear > t1 || t0 > *tfar)
		return false;

	if (*tnear < t0)
		*tnear = t0;
	if (*tfar  > t1)
		*tfar  = t1;

	// Test intersection with bounding box z axis
	t0 = (_pmin._z - ray._o._z) * inv_ray_z_dir;
	t1 = (_pmax._z - ray._o._z) * inv_ray_z_dir;
	
	if (t0 > t1)
	{
		temp = t0;
		t0 = t1;
		t1 = temp;
	}

	if (*tnear > t1 || t0 > *tfar)
		return false;

	if (*tnear < t0)
		*tnear = t0;
	if (*tfar  > t1)
		*tfar  = t1;

	return true;
}

float BBox::Area(void) const
{
	float edge_1 = _pmax._x - _pmin._x;
	float edge_2 = _pmax._y - _pmin._y;
	float edge_3 = _pmax._z - _pmin._z;

	float sub_area_1 = edge_1 * edge_2;
	float sub_area_2 = edge_1 * edge_3;
	float sub_area_3 = edge_2 * edge_3;

	return (sub_area_1 + sub_area_2 + sub_area_3) * 2.0f;
}

Point3 BBox::Center(void) const
{
	return Point3((_pmax._x + _pmin._x) * 0.5f, (_pmax._y + _pmin._y) * 0.5f, (_pmax._z + _pmin._z) * 0.5f);
}

int BBox::MaxDim(void)
{
	float x_dim = _pmax._x - _pmin._x;
	float y_dim = _pmax._y - _pmin._y;
	float z_dim = _pmax._z - _pmin._z;

	if (x_dim > y_dim && x_dim > z_dim)
		return 0;

	if (y_dim > z_dim)
		return 1;

	return 2;
}

BBox Combine(const BBox &bbox_1, const BBox &bbox_2)
{
	Point3 pmin;
	pmin._x = __min(bbox_1._pmin._x, bbox_2._pmin._x);
	pmin._y = __min(bbox_1._pmin._y, bbox_2._pmin._y);
	pmin._z = __min(bbox_1._pmin._z, bbox_2._pmin._z);

	Point3 pmax;
	pmax._x = __max(bbox_1._pmax._x, bbox_2._pmax._x);
	pmax._y = __max(bbox_1._pmax._y, bbox_2._pmax._y);
	pmax._z = __max(bbox_1._pmax._z, bbox_2._pmax._z);

	return BBox(0, pmin, pmax);
}