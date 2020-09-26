#include "bvh.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\shape.h"
#include "..\\Material\\material.h"
#include "..\\Scene\\scene.h"


BVHNode::~BVHNode(void)
{
	if (_bound._id == 0)
	{
		DELETE(_left)
		DELETE(_right)
	}
}

BVHNode::BVHNode(void)
	: _bound()
	, _offset_idx(0)
	, _contain_n(0)
	, _left(0x00)
	, _right(0x00)
{
}

BVHAccel::~BVHAccel(void)
{
	DELETE(_root)
	_object_list = 0x00;
}

BVHAccel::BVHAccel(void)
	: _root(0x00)
	, _object_list(0x00)
	, _extents()
{
}

BVHAccel::BVHAccel(Shape **object, const int &object_count)
	: _root(0x00)
	, _object_list(0x00)
	, _extents()
{
	BuildBVH(object, object_count);
}

BVHNode *BVHAccel::CreateBVHNode(BBox *bounds, const int &start_idx, const int &end_idx, const int pre_dim)
{
	BBox current_extents;

	for (int idx = start_idx; idx <= end_idx; ++idx)
	{
		current_extents = Combine(current_extents, *(bounds + idx));
	}

	int count = end_idx - start_idx + 1;

	// Select the longest side for sorting
	float dx = current_extents._pmax._x - current_extents._pmin._x;
	float dy = current_extents._pmax._y - current_extents._pmin._y;
	float dz = current_extents._pmax._z - current_extents._pmin._z;

	int dim = 0;

	if (dx < dy && dz < dy)
		dim = 1;

	if (dx < dz && dy < dz)
		dim = 2;

	if (dim != pre_dim)
	{
		Shape *temp_s;
		BBox temp_bb;
		float temp_dimension;
		int jump_idx = 0;

		// Shell sort
		for (int gap = count >> 1; gap >= 1; gap >>= 1)
		{
			for (int temp_idx = start_idx + gap; temp_idx <= end_idx; ++temp_idx)
			{
				temp_s = _object_list[temp_idx];
				temp_bb = bounds[temp_idx];
				temp_dimension = temp_bb.Center()[dim];

				for (jump_idx = temp_idx - gap;  jump_idx >= start_idx; jump_idx -= gap)
				{
					if (bounds[jump_idx].Center()[dim] > temp_dimension)
					{
						_object_list[jump_idx + gap] = _object_list[jump_idx];
						bounds[jump_idx + gap] = bounds[jump_idx];

						_object_list[jump_idx] = temp_s;
						bounds[jump_idx] = temp_bb;
					}
					else
						break;
				}
			}
		}
	}

	BVHNode *node = new BVHNode();

	if (count > 6)
	{
		int cut = (start_idx + end_idx) >> 1;

		BVHNode *left = CreateBVHNode(bounds, start_idx, cut, dim);
		BVHNode *right = CreateBVHNode(bounds, cut + 1, end_idx, dim);

		node->_bound = Combine(left->_bound, right->_bound);
		node->_left  = left;
		node->_right = right;
	}
	else
	{
		node->_bound = current_extents;
		node->_bound._id = (bounds + start_idx)->_id;
		node->_offset_idx = start_idx;
		node->_contain_n = count;
	}

	return node;
}

void BVHAccel::BuildBVH(Shape **object, const int &object_count)
{
	_object_list = object;

	BBox *bounds = new BBox [object_count];

	for (int idx = 0; idx < object_count; ++idx)
	{
		bounds[idx] = _object_list[idx]->Bound();
	}

	_root = CreateBVHNode(bounds, 0, object_count - 1);

	_extents = _root->_bound;

	delete [] bounds;
	bounds = 0x00;

	return;
}

int BVHAccel::Travel(Ray *ray, Intersection *isect) const
{	
	BVHNode *todo_stack[256];
	int todo_idx = -1;

	BVHNode *travel_node = _root;
	float tnear = 0.0f;
	float tfar = 0.0f;

	int intersect_idx = -1;

	if (ray->_d._x == 0.0f && ray->_d._y == 0.0f && ray->_d._z == 0.0f)
		return intersect_idx;

	for (;;)
	{
		// Bounding box intersection test
		if (travel_node->_bound.Intersect(*ray, &tnear, &tfar) == true &&
			tfar >= 0.0f && ray->_tmax >= tnear &&
			(ray->_tmin == 0.0f || ray->_tmin > tnear))
		{
			// Leaf node
			if (travel_node->_bound._id > 0)
			{
				int count = travel_node->_offset_idx + travel_node->_contain_n;

				for (int idx = travel_node->_offset_idx; idx < count; ++idx)
				{
					if ((*(_object_list + idx))->Intersect(ray, isect) == true)
						intersect_idx = idx;
				}

				// When _ray->intersect is NULL, it is a shadow ray
				if (isect == 0x00 && ray->_tmin > 0.0f)
					return intersect_idx;
			}
			else
			{
				// Interior node
				// Todo stack record the right node
				if (travel_node->_right)
				{
					++todo_idx;
					*(todo_stack + todo_idx) = travel_node->_right;
				}
				// Trace pointer go to the left node
				if (travel_node->_left)
				{
					travel_node = travel_node->_left;
					continue;
				}
			}
		}

		// Todo stack is empty
		if (todo_idx < 0)
			break;

		travel_node = *(todo_stack + todo_idx);
		--todo_idx;
	}

	return intersect_idx;
}