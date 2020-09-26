#include "pinhole.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\rect.h"
#include "..\\Integrator\\path.h"


Pinhole::~Pinhole(void)
{
	DELETE(_film_plane)
}

Pinhole::Pinhole(void)
	: _p()
	, _dir()
	, _up()
	, _right()
	, _basis_transform()
	, _fov(0.0f)
	, _tanfov(0.0f)
	, _y_ratio(0.0f)
	, _film_area(0.0f)
	, _plane_dist(0.0f)
	, _focal_distance(0.0f)
	, _lens_radius(0.0f)
	, _film_plane(0x00)
{
}

Pinhole::Pinhole(const Point3 position, const Mat4x4 basis_transform, const float y_ratio)
	: _p(position)
	, _dir(0.0f, 0.0f, 1.0f)
	, _up(0.0f, 1.0f, 0.0f)
	, _right(1.0f, 0.0f, 0.0f)
	, _basis_transform(basis_transform)
	, _fov(20.0f)
	, _tanfov(0.0f)
	, _plane_dist(0.0f)
	, _y_ratio(y_ratio)
	, _film_area(0.0f)
	, _focal_distance(0.0f)
	, _lens_radius(0.0f)
	, _film_plane(0x00)
{
	_tanfov = tan(PI / 180.f * _fov * 0.5f);
	_plane_dist = 1.0f;
	_film_area = (2.0f * _tanfov) * (2.0f * _tanfov * _y_ratio);
	_dir = _basis_transform * _dir;
	_up = _basis_transform * _up;
	_right = _basis_transform * _right;
}

float Pinhole::PositionPdf(void)
{
	return 1.0f;
}

float Pinhole::Pdfw(const Vector3 &w)
{
	float costheta = Dot(w, _dir);
	return _plane_dist * _plane_dist / (costheta * costheta * costheta * _film_area);
}

void Pinhole::AddFilmPlane(void)
{
	if (_film_plane)
		return;

	float width = 2.0f * _tanfov;
	float height = 2.0f * _tanfov * _y_ratio;

	Point3 start_p(_p);
	start_p = start_p - _right * (0.5f * width);
	start_p = start_p - _up * (0.5f * height);
	start_p = start_p + _dir * _plane_dist;
	Vector3 length_vect(_right * width);
	Vector3 width_vect(_up * height);

	_film_plane = new Rect(start_p, length_vect, width_vect);

	return;
}

float Pinhole::We(const float &costheta_i)
{
	return _plane_dist * _plane_dist / (_film_area * costheta_i * costheta_i * costheta_i * costheta_i);
}

void Pinhole::DepthOfField(const float focal_distance, const float lens_radius)
{
	_focal_distance = focal_distance;
	_lens_radius = lens_radius;

	return;
}

void Pinhole::CastViewRay(const int &img_width, const int &img_height, const int &width_idx, const int &height_idx, const Point2 sample,
	Point3 *position, Vector3 *direction, float *costheta, float *pdf_w) const
{
	// Transform the window coordinate 0,0 ~ width,height to -1,1 ~ 1,-1
	float half_width = img_width * 0.5f;
	float half_height = img_height * 0.5f;

	float sampling_x = -(((float)width_idx - half_width) + sample._x) / half_width * _tanfov;
	float sampling_y = -(((float)height_idx - half_height) + sample._y) / half_height * _tanfov * _y_ratio;
	Point3  cast_p(_p);
	Vector3 worldspace_d = Normalize(sampling_x, sampling_y, _plane_dist);
	Vector3 cast_d = _basis_transform * worldspace_d;

	// Generate the distribution view ray
	if (_focal_distance > 0.0f)
	{
		// Set up the focal point on focal plane
		Point3 focal_p = _p + cast_d * (_focal_distance / worldspace_d._z);

		// Disk sampling CDF -> P(r) = r^2 Inverse sqrt(P(r)) = r
		float rand_r = sqrt(RandomFloat()) * _lens_radius;
		float rand_theta = RandomFloat() * 2.0f * PI;
		cast_p = _p + (_up * rand_r * sin(rand_theta)) + (_right * rand_r * cos(rand_theta));		
		cast_d = Normalize(focal_p - cast_p);
	}

	if (position)
		*position = cast_p;

	if (direction)
		*direction = cast_d;

	if (costheta)
		*costheta = worldspace_d._z;

	if (pdf_w)
		*pdf_w = _plane_dist * _plane_dist / (*costheta * *costheta * *costheta * _film_area);

	return;
}

bool Pinhole::FilmPlaneIntersection(const Point3 &position, const int &image_width, const int &image_height,
	int *img_widx, int *img_hidx)
{
	Ray r(position, Normalize(_p - position));

	if (_film_plane)
		_film_plane->Intersect(&r, 0x00);

	if (r._tmin == 0.0f)
		return false;

	Point3 intersect_p = r._o + r._d * r._tmin;
	Vector3 on_plane(intersect_p - _film_plane->Position());
	float u = Dot(on_plane, _film_plane->LengthVector()) / (_film_plane->Length() * _film_plane->Length());
	float v = Dot(on_plane, _film_plane->WidthVector()) / (_film_plane->Width() * _film_plane->Width());

	u = (float)image_width * (1.0f - u);
	v = (float)image_height * (1.0f - v);

	if (u == image_width)
		u -= 1.0f;
	if (v == image_height)
		v -= 1.0f;
	if (img_widx)
		*img_widx = u;
	if (img_hidx)
		*img_hidx = v;

	return true;
}