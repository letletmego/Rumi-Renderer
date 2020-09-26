#ifndef _BVH_H
#define _BVH_H

#include "..\\Kernel\\bbox.h"

class Shape;
class Intersection;

class BVHNode
{
public:
	// Bounding box of bvh node
	BBox _bound;
	// Connected information
	union
	{
		// Interior node
		struct
		{
			BVHNode *_left;
			BVHNode *_right;
		};
		// Leaf node
		struct
		{
			// Index of geometry objects
			unsigned int _offset_idx;
			// Number of geometry objects
			unsigned int _contain_n;
		};
	};

public:
	~BVHNode(void);
	BVHNode(void);
};


class BVHAccel
{
private:
	BVHNode *_root;
	Shape **_object_list;

public:
	BBox _extents;

public:
	~BVHAccel(void);
	BVHAccel(void);
	BVHAccel(Shape **object, const int &object_count);

private:
	BVHNode *CreateBVHNode(BBox *bounds, const int &start_idx, const int &end_idx, const int pre_dim = 3);

public:
	void BuildBVH(Shape **object, const int &object_count);
	int Travel(Ray *ray, Intersection *isect) const;
};

#endif