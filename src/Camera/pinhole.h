#ifndef _PINHOLE_H
#define _PINHOLE_H

#include "..\\Kernel\\point.h"
#include "..\\Kernel\\vector.h"
#include "..\\Kernel\\matrix.h"

class Ray;
class Film;
class Intersection;
class Rect;

class Pinhole
{
public:
	// Local position
	Point3 _p;
	// Local orthonormal basis
	Vector3 _dir;
	Vector3 _up;
	Vector3 _right;
	Mat4x4 _basis_transform;
	// Field of view
	float _fov;
	float _tanfov;
	float _plane_dist;
	float _y_ratio;
	float _film_area;
	// Set the depth of field
	float _focal_distance;
	float _lens_radius;
	// light tracing
	Rect *_film_plane;

public:
	~Pinhole(void);
	Pinhole(void);
	//Pinhole(const Point3 position, const Point3 target, const Vector3 up, const float y_ratio = 1.0f, Mat4x4 transform = Mat4x4());
	Pinhole(const Point3 position, const Mat4x4 basis_transform = Mat4x4(), const float y_ratio = 1.0f);
	void AddFilmPlane(void);

public:
	float PositionPdf(void);
	float Pdfw(const Vector3 &w);
	float We(const float &costheta_i);
	void DepthOfField(const float focal_distance, const float lens_radius);
	void CastViewRay(const int &img_width, const int &img_height, const int &width_idx, const int &height_idx, const Point2 sample, Point3 *position, Vector3 *direction, float *costheta, float *pdf_w) const;
	bool FilmPlaneIntersection(const Point3 &position, const int &image_width, const int &image_height, int *img_widx, int *img_hidx);
};

#endif