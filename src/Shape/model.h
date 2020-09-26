#ifndef _TriObjModel_H
#define _TriObjModel_H

#include "shape.h"

class Triangle;
class BVHAccel;

class Model : public Shape
{
private:
	// Count of triangle
	int _n_triangles;
	// Object of triangle
	Triangle **_triangles;
	// BVH of model
	BVHAccel *_bvh_ptr;

public:
	~Model(void);
	Model(void);
	Model(const char *file, const Mat4x4 &transform = Mat4x4());

public:
	bool AddingMaterial(Material *material_ptr);
	bool AddingTexture(Texture *texture_ptr);

	bool Intersect(Ray *ray, Intersection *isect) const;

	void LoadObjectFile(const char *file);
    void BuildAccel(void);
	BBox Bound(void) const;
};

#endif