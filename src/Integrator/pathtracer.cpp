#include "pathtracer.h"
#include "path.h"
#include "..\\Camera\\pinhole.h"
#include "..\\Camera\\film.h"
#include "..\\Sample\\sampler.h"
#include "..\\Shape\\shape.h"
#include "..\\Light\\light.h"
#include "..\\Material\\material.h"
#include "..\\Texture\\texture.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Volume\\homogeneousvolume.h"
#include "..\\Scene\\scene.h"


PathTracer::~PathTracer(void)
{
	DELETE_ARRAY(_paths)
}

PathTracer::PathTracer(void)
	: Integrator()
	, _max_depth(0)
	, _paths(0x00)
{
}

PathTracer::PathTracer(Scene *scene_ptr, int max_depth)
	: Integrator(scene_ptr)
	, _max_depth(max_depth)
	, _paths(0x00)
{
	_paths = new Path[_max_depth];

	for (int idx = 0; idx < _max_depth; ++idx)
	{
		(_paths + idx)->_info._trace_depth = idx;
		(_paths + idx)->_info._scene_ptr = _scene_ptr;

		if (idx < _max_depth - 1)
			(_paths + idx)->_info._next = &(_paths + idx + 1)->_info;
		else
			(_paths + idx)->_info._next = &(_paths + idx)->_info; // End of path
	}
}

Color PathTracer::Trace(Ray *ray, int trace_depth, bool le, bool in_object)
{
	if (trace_depth >= _max_depth)
		return Color();

	Color lo;

	Vertex vertex;
	Intersection info(ray, _scene_ptr);
	info._vertex_ptr = &vertex;
	info._trace_depth = trace_depth;
	info._le = le;
	info._in_object = in_object;

	int intersect_idx = _scene_ptr->_bvh_ptr->Travel(ray, &info);

	HomogeneousVolume *volume_ptr = _scene_ptr->_volume_ptr;
	float tnear = 0.0f;
	float tfar = 0.0f;
	float s = 0.0f;
	float smax = 0.0f;
	Color tau = 1.0f;
	bool absorb = false;

	if (volume_ptr && in_object == false)
	{
		if (volume_ptr->Intersect(*ray, &tnear, &tfar) == true)
		{
			smax = volume_ptr->MaxDistance(info._ray_ptr->_tmin, tnear, tfar);

			if (volume_ptr->DistanceSampling(smax, &s, &absorb, &info._volumetric_tau, &info._volumetric_pdf) == true)
			{
				// In volume
				if (tnear < 0.0f)
					vertex._p = ray->_o + ray->_d * s;
				else
					vertex._p = ray->_o + ray->_d * (tnear + s);

				Color vol(info._volumetric_tau / info._volumetric_pdf);
				lo = vol * volume_ptr->GlobalIllumination(info, s, smax);

				return lo;
			}
		}

		if (absorb == true)
			return Color();

		tau = volume_ptr->Tr(*ray, 0.0f, smax);
	}


	if (info._object_ptr == 0x00)
		return Color();

	Shape *hit_object = info._object_ptr;
	Material *material_ptr = hit_object->MaterialPtr();
	Texture *texture_ptr = hit_object->TexturePtr();

	if (material_ptr)
	{
		lo = material_ptr->DirectLighting(info);
		lo += material_ptr->GlobalIllumination(info) * tau;
	}

	if (texture_ptr)
		lo *= texture_ptr->Pattern(hit_object, vertex._p);

	return lo;
}

Color PathTracer::Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr)
{
	Path *path_ptr = _paths;

	Ray *tracing = &path_ptr->_tracing;
	camera_ptr->CastViewRay(
		image_ptr->Width(), image_ptr->Height(), width_idx, height_idx, 
		image_ptr->PixelSampler()->Sampling(), &tracing->_o, &tracing->_d, 0x00, 0x00);

	Intersection *info_ptr = &path_ptr->_info;
	bool le = true;
	bool in_object = false;
	info_ptr->_le = le;
	info_ptr->_in_object = in_object;

	Vertex *vertex_ptr = &path_ptr->_end;

	Shape *object_ptr = 0x00;
	Material *material_ptr = 0x00;
	Texture *texture_ptr = 0x00;

	float xi = 1.0f;
	float costheta = 1.0f;
	float pdf_wi = 1.0f;
	float pdf_path = 1.0f;
	Vector3 w;
	Vector3 wo;
	Vector3 n;

	HomogeneousVolume *volume_ptr = _scene_ptr->_volume_ptr;
	float tnear = 0.0f;
	float tfar  = 0.0f;
	bool absorb = false;

	int depth_idx = 0;
	for ( ; depth_idx < _max_depth; ++depth_idx)
	{
		_scene_ptr->_bvh_ptr->Travel(tracing, info_ptr);

		if (volume_ptr && info_ptr->_in_object == false)
		{
			if (volume_ptr->Intersect(*tracing, &tnear, &tfar) == true)
			{
				info_ptr->_volumetric_smax = volume_ptr->MaxDistance(tracing->_tmin, tnear, tfar);

				if (volume_ptr->DistanceSampling(info_ptr->_volumetric_smax, &info_ptr->_volumetric_s, &absorb, 
					&info_ptr->_volumetric_tau, &info_ptr->_volumetric_pdf) == true)
				{
					if (tnear < 0.0f)
						vertex_ptr->_p = tracing->_o + tracing->_d * info_ptr->_volumetric_s;
					else
						vertex_ptr->_p = tracing->_o + tracing->_d * (tnear + info_ptr->_volumetric_s);

					vertex_ptr->_onsurface = false;
				}
			}

			if (absorb == true)
			{
				break;
			}
		}

		xi = 1.0f;
		costheta = 1.0f;
		pdf_wi = 1.0f;
		pdf_path = 1.0f;

		if (volume_ptr && info_ptr->_volumetric_s > 0.0f && info_ptr->_in_object == false)
		{
			w = volume_ptr->PhaseSampling(info_ptr, &le, &in_object, &pdf_wi);
		}
		else
		{
			if (info_ptr->_object_ptr == 0x00)
				break;

			object_ptr = info_ptr->_object_ptr;
			material_ptr = object_ptr->MaterialPtr();
			texture_ptr = object_ptr->TexturePtr();

			if (material_ptr->IsLe() == true)
			{
				vertex_ptr->_pdf_pbackward = object_ptr->Pdf();
				break;
			}

			wo = -tracing->_d;
			n = vertex_ptr->_n;

			if (material_ptr->IsSubsurface() == false)
			{
				w = material_ptr->BSDFSampling(wo, n, 
					&le, &in_object, &xi, &pdf_path, &costheta, &pdf_wi);
			}
			else
			{
				if (in_object == false)
				{
					w = material_ptr->BSDFSampling(wo, n, 
						&le, &in_object, &xi, &pdf_path, &costheta, &pdf_wi);
				}
				else 
				{
					if (material_ptr->PhaseSampling(info_ptr, &le, &in_object, &w, &pdf_wi) == false)
					{
						// absorb
						if (info_ptr->_volumetric_s < 0.0f)
							break;
						else
						{
							w = material_ptr->BSDFSampling(wo, n, 
								&le, &in_object, &xi, &pdf_path, &costheta, &pdf_wi);
						}
					}
				}
			}
		}

		info_ptr->_rand = xi;
		vertex_ptr->_costheta_i = costheta;
		vertex_ptr->_pdf_wi = pdf_wi;
		vertex_ptr->_pdf_path = pdf_path;

		if (vertex_ptr->_onsurface == true)
			vertex_ptr->_isdirac = material_ptr->IsDirac(xi);

		if (depth_idx + 1 > _max_depth - 1)
			break;

		path_ptr = (_paths + depth_idx + 1);
		path_ptr->_start = *vertex_ptr;
		vertex_ptr = &path_ptr->_end;

		tracing = &path_ptr->_tracing;
		tracing->_o = path_ptr->_start._p;
		tracing->_d = w;

		info_ptr = &path_ptr->_info;
		info_ptr->_le = le;
		info_ptr->_in_object = in_object;
	}

	// Compute light transport contribution
	Color L;
	Color direct_lo;
	Color indirect_lo;
	Color fr;
	Color vol;
	costheta = 1.0f;
	float pdf = 1.0f;

	for (; depth_idx >= 0; --depth_idx)
	{
		object_ptr = 0x00;
		material_ptr = 0x00;
		texture_ptr = 0x00;

		path_ptr = (_paths + depth_idx);

		tracing = &path_ptr->_tracing;
		info_ptr = &path_ptr->_info;
		vertex_ptr = info_ptr->_vertex_ptr;

		n = vertex_ptr->_n;
		wo = -tracing->_d;

		if (depth_idx == _max_depth - 1)
			w = 0.0f;
		else
			w = (path_ptr + 1)->_tracing._d;
		
		direct_lo = 0.0f;
		indirect_lo = 0.0f;
		fr = 1.0f;
		vol = 1.0f;
		costheta = vertex_ptr->_costheta_i;
		pdf = Average(vertex_ptr->_pdf_wi * vertex_ptr->_pdf_path);

		if (volume_ptr && info_ptr->_volumetric_s != 0.0f && info_ptr->_in_object == false)
		{
			// Absorb
			if (info_ptr->_volumetric_s == -1.0f)
				continue;

			if (info_ptr->_volumetric_s > 0.0f)
			{
				direct_lo = volume_ptr->DirectLighting(*info_ptr);
				indirect_lo = L * volume_ptr->Fp(vertex_ptr->_p, w, wo) / pdf;
				vol = info_ptr->_volumetric_tau / info_ptr->_volumetric_pdf;
				// radiance * distance_tau / distance_pdf -> radiance / sigma_t
				L = (direct_lo + indirect_lo) * vol;
			}
		}
		else
		{
			object_ptr = info_ptr->_object_ptr;
			if (object_ptr == 0x00)
				continue;

			material_ptr = object_ptr->MaterialPtr();
			texture_ptr = object_ptr->TexturePtr();

			if (material_ptr->IsSubsurface() == false)
			{
				fr = material_ptr->Fr(*info_ptr, n, w, wo);
			}
			else
			{
				if (info_ptr->_volumetric_s == 0.0f)
					fr = material_ptr->Fr(*info_ptr, n, w, wo);
				else // In subsurface
				{
					vol = info_ptr->_volumetric_tau / info_ptr->_volumetric_pdf;
					vol *= material_ptr->Fp(vertex_ptr->_p, w, wo);
				}
			}
				
			direct_lo = material_ptr->DirectLighting(*info_ptr);

			indirect_lo = vol * fr * L * costheta / pdf;
			L = direct_lo + indirect_lo;

			if (volume_ptr && info_ptr->_in_object == false)
				L *= (info_ptr->_volumetric_tau / info_ptr->_volumetric_pdf);

			if (texture_ptr)
				L *= texture_ptr->Pattern(object_ptr, vertex_ptr->_p);
		}

		if (depth_idx < _max_depth - 1)
			(_paths + depth_idx + 1)->Clean();
	}

	(_paths + 0)->Clean();

	return L;
}