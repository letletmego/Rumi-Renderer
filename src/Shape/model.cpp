#include "Model.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"
#include "..\\Kernel\\vector.h"
#include "..\\Kernel\\matrix.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\triangle.h"
#include "..\\Acceleration\\bvh.h"


Model::~Model(void)
{
	for (int tri_idx = 0; tri_idx < _n_triangles; ++tri_idx)
	{
		(*(_triangles + tri_idx))->AddingMaterial(0x00);
		(*(_triangles + tri_idx))->AddingTexture(0x00);

		DELETE(*(_triangles + tri_idx))
	}

	DELETE_ARRAY(_triangles)

	DELETE(_bvh_ptr)
}

Model::Model(void)
	: Shape()
	, _triangles(0x00)
	, _n_triangles(0)
	, _bvh_ptr(0x00)
{
}

Model::Model(const char *file, const Mat4x4 &transform)
	: Shape(transform)
	, _triangles(0x00)
	, _n_triangles(0)
	, _bvh_ptr(0x00)
{
	LoadObjectFile(file);
	BuildAccel();
}

bool Model::AddingMaterial(Material *material_ptr)
{
	if (_material_ptr)
		return false;

	for (int idx = 0; idx < _n_triangles; ++idx)
	{
		(*(_triangles + idx))->AddingMaterial(material_ptr);
	}

	return _material_ptr = material_ptr;
}

bool Model::AddingTexture(Texture *texture_ptr)
{
	if (_texture_ptr)
		return false;

	for (int idx = 0; idx < _n_triangles; ++idx)
	{
		(*(_triangles + idx))->AddingTexture(texture_ptr);
	}

	return _texture_ptr = texture_ptr;
}

bool Model::Intersect(Ray *ray, Intersection *isect) const
{
	return _bvh_ptr->Travel(ray, isect) >= 0 ? true : false;
}

void Model::LoadObjectFile(const char *file)
{
	FILE *file_ptr = fopen(file, "r");

	if (file_ptr == 0x00)
	{
		printf("%s is not existent.\n", file);
		return;
	}

	printf("Loading Model %s\n", file);

	int v_count  = 0;
	int vt_count = 0;
	int vn_count = 0;
	int f_count  = 0;

	char buffer[256] = { 0x00 };

	while (fgets(buffer, 256, file_ptr) != NULL)
	{
		if (buffer[0] == 'v' && buffer[1] == ' ')
			++v_count;
		if (buffer[0] == 'v' && buffer[1] == 't')
			++vt_count;
		if (buffer[0] == 'v' && buffer[1] == 'n')
			++vn_count;
		if (buffer[0] == 'f' && buffer[1] == ' ')
			++++++f_count;
	}

	fseek(file_ptr, 0, SEEK_SET);

	_n_triangles = f_count / 3;
	_triangles = new Triangle *[_n_triangles];
	printf("Model Triangle Count : %d\n", _n_triangles);

	Point3 *v = 0x00;
	int *v_idx_list = 0x00;
	if (v_count > 0)
	{
		v = new Point3 [v_count];
		v_idx_list = (int *)malloc(f_count * sizeof(int));
		memset(v_idx_list, 0, f_count * sizeof(int));
	}

	Point2 *vt = 0x00;
	int *vt_idx_list = 0x00;
	if (vt_count > 0)
	{
		vt = new Point2 [vt_count];
		vt_idx_list = (int *)malloc(f_count * sizeof(int));
		memset(vt_idx_list, 0, f_count * sizeof(int));
	}

	Vector3 *vn = 0x00;
	int *vn_idx_list = 0x00;
	if (vn_count > 0)
	{
		vn = new Vector3 [vn_count];
		vn_idx_list = (int *)malloc(f_count * sizeof(int));
		memset(vn_idx_list, 0, f_count * sizeof(int));
	}

	int v_idx  = 0;
	int vt_idx = 0;
	int vn_idx = 0;
	int f_idx  = 0;
	float min_y = FLT_MAX;

	char data_type[4] = { 0x00 };

	while (true)
	{
		for (int idx = 0; idx < 4; ++idx)
		{
			data_type[idx] = fgetc(file_ptr);

			if (data_type[idx] == '\n')
				break;
			if (data_type[idx] == ' ')
				break;
		}

		if (data_type[0] == EOF)
			break;

		if (strcmp("v ", data_type) == 0)
		{
			Point3 vertex;
			fscanf(file_ptr, "%f %f %f", &vertex._x, &vertex._y, &vertex._z);
			v[v_idx] = vertex;
			++v_idx;
			
			if (v[v_idx]._y < min_y)
				min_y = v[v_idx]._y;
		}

		if (strcmp("vt ", data_type) == 0)
		{
			Point2 uv;
			fscanf(file_ptr, "%f %f", &uv._x, &uv._y);
			vt[vt_idx] = uv;
			++vt_idx;
		}

		if (strcmp("vn ", data_type) == 0)
		{
			Vector3 normal;
			fscanf(file_ptr, "%f %f %f", &normal._x, &normal._y, &normal._z);
			vn[vn_idx] = normal;
			++vn_idx;
		}

		if (strcmp("f ", data_type) == 0 && fgets(buffer , 256 , file_ptr) != NULL)
		{
			int input_idx = 0;
			int idx_v[3]  = { 0, 0, 0 };
			int idx_vt[3] = { 0, 0, 0 };
			int idx_vn[3] = { 0, 0, 0 };

			char check_word;
			int data_idx = 0;
			float data[3] = { 0.0f, 0.0f, 0.0f };
			float scalar = 10.0f;

			for (int idx = 0; idx < 256; ++idx)
			{
				switch (buffer[idx])
				{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						data[data_idx] = data[data_idx] * scalar + (buffer[idx] - 0x30);
						break;

					case '/':
						++data_idx;
						break;

					case ' ' :
					case '\n':
					case '\0':
						idx_v[input_idx]  = data[0] - 1;
						idx_vt[input_idx] = data[1] - 1;
						idx_vn[input_idx] = data[2] - 1;
						++input_idx;

						data_idx = 0;
						data[0] = 0.0f;
						data[1] = 0.0f;
						data[2] = 0.0f;
						break;

					default:
						break;
				}

				if (input_idx == 3)
					break;
			}

			*(v_idx_list + f_idx + 0) = idx_v[0];
			*(v_idx_list + f_idx + 1) = idx_v[1];
			*(v_idx_list + f_idx + 2) = idx_v[2];

			if (vt_count > 0)
			{
				*(vt_idx_list + f_idx + 0) = idx_vt[0];
				*(vt_idx_list + f_idx + 1) = idx_vt[1];
				*(vt_idx_list + f_idx + 2) = idx_vt[2];
			}

			if (vn_count > 0)
			{
				*(vn_idx_list + f_idx + 0) = idx_vn[0];
				*(vn_idx_list + f_idx + 1) = idx_vn[1];
				*(vn_idx_list + f_idx + 2) = idx_vn[2];
			}

			++++++f_idx;
		}

		memset(data_type, 0x00, sizeof(data_type));
		memset(buffer, 0x00, sizeof(buffer));
	}

	Point3 v0;
	Point3 v1;
	Point3 v2;
	Vector3 n0;
	Vector3 n1;
	Vector3 n2;
	int tri_idx = 0;

	for (int f_idx = 0; f_idx < f_count; ++++++f_idx)
	{
		v0 = *(v + *(v_idx_list + f_idx + 0));
		v1 = *(v + *(v_idx_list + f_idx + 1));
		v2 = *(v + *(v_idx_list + f_idx + 2));

		v0 = _transform * v0;
		v1 = _transform * v1;
		v2 = _transform * v2;

		*(_triangles + tri_idx) = new Triangle(v0, v1, v2);

		if (vn_count > 0)
		{
			n0 = *(vn + *(vn_idx_list + f_idx + 0));
			n1 = *(vn + *(vn_idx_list + f_idx + 1));
			n2 = *(vn + *(vn_idx_list + f_idx + 2));

			n0 = Normalize(_transform_inv * n0);
			n1 = Normalize(_transform_inv * n1);
			n2 = Normalize(_transform_inv * n2);
		}
		else
		{
			n0 = n1 = n2 = Normalize(Cross(v1 - v0, v2 - v0));
		}

		(*(_triangles + tri_idx))->Normal(n0, n1, n2);

		++tri_idx;
	}

	free(vn_idx_list); vn_idx_list = 0x00;
	free(vt_idx_list); vt_idx_list = 0x00;
	free(v_idx_list);  v_idx_list  = 0x00;

	if (vn) delete [] vn;
	if (vt) delete [] vt;
	if (v)  delete [] v;

	fclose(file_ptr);
	printf("Loading Model Success.\n\n");

	return;
 }

void Model::BuildAccel(void)
{
	_bvh_ptr = new BVHAccel((Shape **)_triangles, _n_triangles);

	return;
}

BBox Model::Bound(void) const
{
	BBox model_bound;
	BBox tri_bound;

	for (int tri_idx = 0; tri_idx < _n_triangles; ++tri_idx)
	{
		tri_bound = _triangles[tri_idx]->Bound();
		model_bound = Combine(model_bound, tri_bound);
	}

	model_bound._id = _shape_id;

	return model_bound;
}