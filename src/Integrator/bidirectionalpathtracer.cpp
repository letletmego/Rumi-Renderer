#include "bidirectionalpathtracer.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\bsdf.h"
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


BidirectionalPathTracer::~BidirectionalPathTracer(void)
{
	DELETE_ARRAY(_s_path)
	DELETE_ARRAY(_t_path)
	DELETE_ARRAY(_alphaL)
	DELETE_ARRAY(_alphaE)
	DELETE_ARRAY(_s_pdfforward)
	DELETE_ARRAY(_s_pdfbackward)
	DELETE_ARRAY(_t_pdfforward)
	DELETE_ARRAY(_t_pdfbackward)
}

BidirectionalPathTracer::BidirectionalPathTracer(void)
	: Integrator()
	, _nL(0)
	, _nE(0)
	, _s_path(0x00)
	, _t_path(0x00)
	, _alphaL(0x00)
	, _alphaE(0x00)
	, _s_pdfforward(0x00)
	, _s_pdfbackward(0x00)
	, _t_pdfforward(0x00)
	, _t_pdfbackward(0x00)
{
}

BidirectionalPathTracer::BidirectionalPathTracer(Scene *scene_ptr, int nL, int nE)
	: Integrator(scene_ptr)
	, _nL(nL)
	, _nE(nE)
	, _s_path(0x00)
	, _t_path(0x00)
	, _alphaL(0x00)
	, _alphaE(0x00)
	, _s_pdfforward(0x00)
	, _s_pdfbackward(0x00)
	, _t_pdfforward(0x00)
	, _t_pdfbackward(0x00)
{
	scene_ptr->_camera->DepthOfField(0.0f, 0.0f);
	scene_ptr->_camera->AddFilmPlane();
	scene_ptr->_image->AddLightBuffer();

	_s_path = new Path [_nL - 1];
	_t_path = new Path [_nE - 1];

	_alphaL = new Color [_nL + 1];
	_alphaE = new Color [_nE + 1];

	_s_pdfforward = new Color [_nL];
	_s_pdfbackward = new Color [_nL];
	_t_pdfforward = new Color [_nE];
	_t_pdfbackward = new Color [_nE];

	// Construct s_path
	int s_depth = _nL - 1;
	for (int idx = 0; idx < s_depth; ++idx)
	{
		(_s_path + idx)->_info._trace_depth = idx;
		(_s_path + idx)->_info._scene_ptr   = _scene_ptr;
		
		if (idx < s_depth - 1)
			(_s_path + idx)->_info._next = &(_s_path + idx + 1)->_info;
		else
			(_s_path + idx)->_info._next = &(_s_path + idx)->_info; // End of path
	}

	// Construct t_path
	int t_depth = _nE - 1;
	for (int idx = 0; idx < t_depth; ++idx)
	{
		(_t_path + idx)->_info._trace_depth = idx;
		(_t_path + idx)->_info._scene_ptr   = _scene_ptr;

		if (idx < t_depth - 1)
			(_t_path + idx)->_info._next = &(_t_path + idx + 1)->_info;
		else
			(_t_path + idx)->_info._next = &(_t_path + idx)->_info; // End of path
	}
}

void BidirectionalPathTracer::CleanPath(const int trace_depth, Path *path, Color *alpha)
{
	if (trace_depth == 0)
	{
		path->Clean();
		*alpha = Color();
	}
	else
	{
		for (int depth_idx = 0; depth_idx < trace_depth; ++depth_idx)
		{
			(path + depth_idx)->Clean();
		}

		for (int depth_idx = 0; depth_idx <= trace_depth; ++depth_idx)
		{
			*(alpha + depth_idx) = Color();
		}
	}

	return;
}

Color BidirectionalPathTracer::Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr)
{
	Color L;

	if (_scene_ptr == 0x00)
		return L;

	_t_path->_start._pdf_pforward = 1.0f;
	HomogeneousVolume *volume_ptr = _scene_ptr->_volume_ptr;

	Light *light_ptr = 0x00;
	float pdf = 1.0f;
	_scene_ptr->LightsUniformSampling(&light_ptr, &pdf);
	_s_path->_start._pdf_pforward = pdf;

	int n_view_vertex = ConstructViewPath(_nE, width_idx, height_idx, camera_ptr, light_ptr);
	int n_light_vertex = ConstructLightPath(_nL, camera_ptr, light_ptr);

	float costheta_i;
	float costheta_o;
	float distance;

	float tG;
	Color alpha_E;
	Vector3 t_wi, t_wo;
	Vertex *t_start = 0x00;
	Vertex *t_end = 0x00;
	Ray *view_ray = 0x00;
	Path *t_path_ptr = 0x00;
	Intersection *t_info_ptr = 0x00;
	Shape *t_object_ptr = 0x00;
	Material *t_material_ptr = 0x00;
	Texture *t_texture_ptr = 0x00;
	Vertex *t_connect = 0x00;

	float sG;
	Color alpha_L;
	Vector3 s_wi, s_wo;
	Vertex *s_start = 0x00;
	Vertex *s_end = 0x00;
	Ray *light_ray = 0x00;
	Path *s_path_ptr = 0x00;
	Intersection *s_info_ptr = 0x00;
	Shape *s_object_ptr = 0x00;
	Material *s_material_ptr = 0x00;
	Texture *s_texture_ptr = 0x00;
	Vertex *s_connect = 0x00;

	Color lo;
	Vector3 connect_wi;
	Vector3 connect_wo;
	Color connect_alpha_E;
	Color connect_alpha_L;
	Color connect_tau(1.0f);
	Color connect_pdf_s(1.0f);
	float s = 0.0f;

	int max_path_length = n_view_vertex + n_light_vertex - 1;
	for (int path_length = 1; path_length <= max_path_length; ++path_length)
	{	
		// Construct view sub-path
		for (int t_idx = -1; t_idx <= path_length; ++t_idx)
		{
			t_object_ptr = 0x00;
			t_material_ptr = 0x00;
			t_connect = 0x00;

			if (t_idx >= n_view_vertex)
				break;

			if (t_idx <= -1) // && camera is dirac
				continue;

			if (t_idx == 0)
			{
				t_start = &_t_path->_start;
				t_end = &_t_path->_end;
				t_object_ptr = _t_path->_info._object_ptr;
				t_connect = t_start;
			}

			if (t_idx > 0)
			{
				if (t_idx >= n_view_vertex - 1)
					t_wi = 0.0f;
				else
				{
					t_path_ptr = _t_path + t_idx;
					view_ray = &t_path_ptr->_tracing;
					t_wi = view_ray->_d;
				}

				t_path_ptr = _t_path + t_idx - 1;

				t_wo = -t_path_ptr->_tracing._d;
				t_info_ptr = &t_path_ptr->_info;

				if (t_info_ptr->_object_ptr)
				{
					t_object_ptr = t_info_ptr->_object_ptr;
					t_material_ptr = t_object_ptr->MaterialPtr();
					t_texture_ptr = t_object_ptr->TexturePtr();
				}

				t_start = &t_path_ptr->_start;
				t_end = &t_path_ptr->_end;
				t_connect = t_end;

				if (t_end->_onsurface == true && t_material_ptr->IsLe() == true)
				{
					if (path_length == n_view_vertex - 1)
					{
						alpha_E = *(_alphaE + t_idx + 1);
						alpha_E *= MISWeight(camera_ptr, light_ptr, 0, t_idx + 1, Vector3(), Vector3());
						L += alpha_E;
					}

					t_connect = 0x00;
					break;
				}
			}

			if (t_idx > 0 && t_connect->_isdirac == true)
				continue;

			// Construct light sub-path
			s_object_ptr = 0x00;
			s_material_ptr = 0x00;
			s_connect = 0x00;
			s_wo = 0.0f;
			s_wi = 0.0f;
			int light_length = path_length - (t_idx + 1);
			// Don't have enough vertex to contruct the light path
			if (light_length >= n_light_vertex)
				continue;

			int s_idx = light_length;
			if (s_idx <= -1)
				continue;

			if (s_idx == 0)
			{
				s_start = &_s_path->_start;
				s_end = &_s_path->_end;
				s_object_ptr = _s_path->_info._object_ptr;
				s_info_ptr = 0x00;
				s_connect = s_start;
			}

			if (s_idx > 0)
			{
				if (s_idx >= n_light_vertex - 1)
					s_wo = 0.0f;
				else
				{
					s_path_ptr = _s_path + s_idx;
					light_ray = &s_path_ptr->_tracing;
					s_wo = light_ray->_d;
				}

				s_path_ptr = _s_path + s_idx - 1;

				s_wi = -s_path_ptr->_tracing._d;
				s_info_ptr = &s_path_ptr->_info;

				if (s_info_ptr->_object_ptr)
				{
					s_object_ptr = s_info_ptr->_object_ptr;
					s_material_ptr = s_object_ptr->MaterialPtr();
					s_texture_ptr = s_object_ptr->TexturePtr();
				}

				s_start = &s_path_ptr->_start;
				s_end = &s_path_ptr->_end;
				s_connect = s_end;

				if (s_end->_onsurface == true && s_material_ptr->IsLe() == true)
				{
					s_connect = 0x00;
					continue;
				}
			}

			if (s_idx > 0 && s_connect->_isdirac == true)
				continue;

			if (VisibilityTest(s_idx + 1, t_idx + 1, s_connect->_p, t_connect->_p) == false)
				continue;

			s = 0.0f;
			connect_tau = 1.0f;
			if (VolumetricTest(s_idx + 1, t_idx + 1, s_connect->_p, t_connect->_p, &s, &connect_tau) == false)
				continue;

			connect_wi = Normalize(s_connect->_p - t_connect->_p);
			connect_wo = Normalize(t_connect->_p - s_connect->_p);

			float costheta_i = 1.0f;
			if (t_connect->_onsurface == true)
				costheta_i = Dot(connect_wi, t_connect->_n);
			if (costheta_i < 1e-4f)
				continue;

			float costheta_o = 1.0f;
			if (s_connect->_onsurface == true)
				costheta_o = Dot(connect_wo, s_connect->_n);
			if (costheta_o < 1e-4f)
				continue;

			distance = Length(t_connect->_p - s_connect->_p);
			if (distance <= 0.0f)
				continue;

			float G = costheta_i * costheta_o / (distance * distance);
			if (G <= 0.0f)
				continue;

			alpha_E = *(_alphaE + t_idx);
			alpha_L = *(_alphaL + s_idx);

			if (t_idx == 0)
				connect_alpha_E = camera_ptr->We(Dot(t_connect->_n, connect_wi)) / Average(t_connect->_pdf_pforward);
			else
			{
				if (t_connect->_onsurface ==true)
					connect_alpha_E = t_material_ptr->Fr(*t_info_ptr, t_connect->_n, connect_wi, t_wo) / (t_connect->_pdf_pforward * t_connect->_pdf_path);
				else
					connect_alpha_E = volume_ptr->Fp(t_connect->_p, connect_wi, t_wo) / (t_connect->_pdf_pforward * t_connect->_pdf_path);

				if (t_texture_ptr)
					connect_alpha_E *= t_texture_ptr->Pattern(t_object_ptr, t_connect->_p);
			}

			if (s_idx == 0)
				connect_alpha_L = light_ptr->L(*s_info_ptr) / s_connect->_pdf_pforward;
			else
			{
				if (s_connect->_onsurface ==true)
					connect_alpha_L = s_material_ptr->Fr(*s_info_ptr, s_connect->_n, s_wi, connect_wo) / (s_connect->_pdf_pforward * s_connect->_pdf_path);
				else
					connect_alpha_L = volume_ptr->Fp(s_connect->_p, s_wi, connect_wo) / (s_connect->_pdf_pforward * s_connect->_pdf_path);

				if (s_texture_ptr)
					connect_alpha_L *= s_texture_ptr->Pattern(s_object_ptr, s_connect->_p);
			}

			if (t_idx == 0)
			{
				int w_idx = 0;
				int h_idx = 0;
				if (camera_ptr->FilmPlaneIntersection(s_connect->_p, image_ptr->Width(), image_ptr->Height(), &w_idx, &h_idx) == false)
					continue;

				if (w_idx == image_ptr->Width() || h_idx == image_ptr->Height())
					continue;

				lo = alpha_E * connect_alpha_E * G * connect_alpha_L * alpha_L;
				lo *= MISWeight(camera_ptr, light_ptr, s_idx + 1, 1, connect_wi, connect_wo);
				image_ptr->LightPixel(w_idx, h_idx, lo);
			}
			else
			{
				lo = alpha_E * connect_alpha_E * G * connect_alpha_L * alpha_L;
				lo *= MISWeight(camera_ptr, light_ptr, s_idx + 1, t_idx + 1, connect_wi, connect_wo);
				L += lo;
			}
		}
	}

	CleanPath(n_view_vertex - 1, _t_path, _alphaE);
	CleanPath(n_light_vertex - 1, _s_path, _alphaL);

	return L;
}

int BidirectionalPathTracer::ConstructViewPath(const int &max_depth, const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Light *light_ptr)
{
	Path *path_ptr = _t_path;
	Ray *tracing = &path_ptr->_tracing;
	Vertex *start = &path_ptr->_start;
	Vertex *end = &path_ptr->_end;

	float pdf_wi = 1.0f;
	Film *image_ptr = _scene_ptr->_image;
	camera_ptr->CastViewRay(
		image_ptr->Width(), image_ptr->Height(), width_idx, height_idx,image_ptr->PixelSampler()->Sampling(),
		&tracing->_o, &tracing->_d, &start->_costheta_i, &pdf_wi);
	
	start->_pdf_wi = pdf_wi;
	start->_p = tracing->_o;
	start->_n = camera_ptr->_dir;
	start->_isdirac = true;

	Intersection *info_ptr = &path_ptr->_info;
	bool le = true;
	bool in_object = false;
	info_ptr->_le = le;
	info_ptr->_in_object = in_object;
	*_alphaE = 1.0f;

	HomogeneousVolume *volume_ptr = _scene_ptr->_volume_ptr;
	float tnear = 0.0f;
	float tfar = 0.0f;
	float smax = 0.0f;
	float s = 0.0f;
	bool absorb = false;
	Color tau;
	Color pdf_s;
	Ray scattering;

	Shape *object_ptr = 0x00;
	Material *material_ptr = 0x00;
	Vector3 w;
	Vector3 wo;
	Vector3 n;
	float d;
	float dw_divide_da;
	float costheta_i;
	float costheta_o;
	float G;
	Color prev_alpha;
	Vector3 prev_wo;
	Intersection *prev_info_ptr = 0x00;
	Material *prev_material_ptr = 0x00;
	Texture *prev_texture_ptr = 0x00;

	int n_vertex = 1;
	for (int depth_idx = 0; depth_idx < max_depth; ++depth_idx)
	{
		_scene_ptr->_bvh_ptr->Travel(tracing, info_ptr);

		if (volume_ptr && info_ptr->_in_object == false)
		{
			if (volume_ptr->Intersect(*tracing, &tnear, &tfar) == true)
			{
				smax = volume_ptr->MaxDistance(tracing->_tmin, tnear, tfar);
				info_ptr->_volumetric_smax = smax;

				if (volume_ptr->DistanceSampling(smax, 
					&info_ptr->_volumetric_s, &absorb, &info_ptr->_volumetric_tau, &info_ptr->_volumetric_pdf) == true)
				{
					if (tnear < 0.0f)
						end->_p = tracing->_o + tracing->_d * info_ptr->_volumetric_s;
					else
						end->_p = tracing->_o + tracing->_d * (tnear + info_ptr->_volumetric_s);

					end->_onsurface = false;
				}
			}

			// Absorb
			if (absorb == true)
			{
				if (depth_idx > 0)
					path_ptr->Clean();

				break;
			}
		}

		// No intersection and no volume scattering
		if (info_ptr->_object_ptr == 0x00 && info_ptr->_volumetric_s == 0.0f)
		{
			if (depth_idx > 0)
				path_ptr->Clean();

			break;
		}

		++n_vertex;

		object_ptr = info_ptr->_object_ptr;
		if (object_ptr)
			material_ptr = object_ptr->MaterialPtr();

		pdf_wi = 1.0f;
		wo = -tracing->_d;
		// Sampling on end vertex
		if (end->_onsurface == true)
		{
			n = end->_n;
			w = material_ptr->BSDFSampling(wo, n, &le, &in_object, &info_ptr->_rand, &end->_pdf_path, &end->_costheta_i, &pdf_wi);
			end->_pdf_wi = pdf_wi;
			end->_isdirac = material_ptr->IsDirac(info_ptr->_rand);
		}
		else
		{
			n = 0.0f;
			w = volume_ptr->PhaseSampling(info_ptr, &le, &in_object, &pdf_wi);
			end->_pdf_wi = pdf_wi;
			end->_isdirac = false;
		}

		// Compute costheta_i of start vertex
		if (start->_onsurface == true)
		{
			if (depth_idx == 0 || start->_isdirac == false)
				costheta_i = start->_costheta_i;
			else
			{
				if (info_ptr->_in_object == false)
					costheta_i = Dot(tracing->_d, start->_n);
				else
					costheta_i = -Dot(tracing->_d, start->_n);

				if (costheta_i <= 0.0f)
					costheta_i = 1e-4f;

				start->_costheta_i = costheta_i;
			}
		}
		else
		{
			costheta_i = 1.0f;
			start->_costheta_i = costheta_i;
		}

		// Compute costheta_o of end vertex
		if (end->_onsurface == true)
		{
			if (info_ptr->_in_object == false)
				costheta_o = -Dot(tracing->_d, end->_n);
			else
				costheta_o = Dot(tracing->_d, end->_n);

			if (costheta_o <= 0.0f)
				costheta_o = 1e-4f;

			end->_costheta_o = costheta_o;
		}
		else
		{
			costheta_o = 1.0f;
			end->_costheta_o = costheta_o;
		}

		// Calculate forward pdf of end vertex
		d = Length(end->_p - start->_p);
		dw_divide_da = costheta_o / (d * d);
		if (start->_onsurface == true)
		{
			if (depth_idx == 0 || start->_isdirac == false)
				end->_pdf_pforward = start->_pdf_wi * dw_divide_da;
			else
				end->_pdf_pforward = start->_pdf_wi * dw_divide_da * start->_costheta_i;
		}
		else
		{
			end->_pdf_pforward = start->_pdf_wi * dw_divide_da * start->_costheta_i;
		}

		if (volume_ptr && info_ptr->_in_object == false)
			end->_pdf_pforward *= info_ptr->_volumetric_pdf;

		// Calculate backward pdf of start vertex
		dw_divide_da = costheta_i / (d * d);
		if (start->_onsurface == true)
		{
			if (end->_onsurface == true)
			{
				if (material_ptr->IsLe() == true)
					start->_pdf_pbackward = light_ptr->Pdfw(wo) * dw_divide_da;
				else
				{
					if (end->_isdirac == false)
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, n, w, wo) * dw_divide_da;
					else
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, n, w, wo) * dw_divide_da * end->_costheta_o;
				}

				if (volume_ptr && info_ptr->_in_object == false)
					start->_pdf_pbackward *= info_ptr->_volumetric_pdf;
			}
			else
			{
				// Start vertex is always in volume
				// Always no intersection between start vertex and end vertex
				scattering._o = end->_p;
				scattering._d = -tracing->_d;
				scattering._tmax = Length(start->_p - end->_p);

				tnear = tfar = 0.0f;
				volume_ptr->Intersect(scattering, &tnear, &tfar);
				s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);

				pdf_s = volume_ptr->Tr(scattering, 0.0f, s);
				start->_pdf_pbackward = volume_ptr->Pdfw(end->_p, w, wo) * dw_divide_da * end->_costheta_o * pdf_s;
			}
		}
		else
		{
			// Start vertex is always in volume
			// Always no intersection between start vertex and end vertex
			scattering._o = end->_p;
			scattering._d = -tracing->_d;
			scattering._tmax = Length(start->_p - end->_p);

			tnear = tfar = 0.0f;
			volume_ptr->Intersect(scattering, &tnear, &tfar);
			s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);

			pdf_s = 1.0f;
			tau = 1.0f;
			volume_ptr->PdfDistance(s, &tau, &pdf_s);

			if (end->_onsurface == true)
			{
				if (material_ptr->IsLe() == true)
					start->_pdf_pbackward = light_ptr->Pdfw(wo) * dw_divide_da * pdf_s;
				else
				{
					if (end->_isdirac == false)
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, end->_n, w, wo) * dw_divide_da * pdf_s;
					else
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, end->_n, w, wo) * dw_divide_da * end->_costheta_o * pdf_s;
				}
			}
			else
			{
				start->_pdf_pbackward = volume_ptr->Pdfw(end->_p, w, wo) * dw_divide_da * end->_costheta_o * pdf_s;
			}
		}

		if (depth_idx > 0)
			(_t_path + depth_idx - 1)->_end._pdf_pbackward = start->_pdf_pbackward;

		// Compute contribution
		G = costheta_i * costheta_o / (d * d);
		*(_alphaE + depth_idx + 1) = *(_alphaE + depth_idx);
		if (start->_onsurface == true)
		{
			if (depth_idx == 0)
			{
				*(_alphaE + depth_idx + 1) *=
					camera_ptr->We(start->_costheta_i) / Average(start->_pdf_pforward) * G;
			}
			else
			{
				*(_alphaE + depth_idx + 1) *=
					prev_material_ptr->Fr(*prev_info_ptr, start->_n, tracing->_d, prev_wo) / (start->_pdf_pforward * start->_pdf_path) * G;

				if (prev_texture_ptr)
					*(_alphaE + depth_idx + 1) *= prev_texture_ptr->Pattern(prev_info_ptr->_object_ptr, start->_p);
			}
		}
		else
		{
			*(_alphaE + depth_idx + 1) *=
				volume_ptr->Fp(start->_p, tracing->_d, prev_wo) / (start->_pdf_pforward * start->_pdf_path) * G;
		}

		if (volume_ptr && info_ptr->_in_object == false)
			*(_alphaE + depth_idx + 1) *= info_ptr->_volumetric_tau;

		if (material_ptr && material_ptr->IsLe() == true)
		{
			*(_alphaE + depth_idx + 2) = *(_alphaE + depth_idx + 1);
			*(_alphaE + depth_idx + 2) *= (material_ptr->Fr(*info_ptr, n, w, wo) / (end->_pdf_pforward * end->_pdf_path));
			end->_pdf_pbackward = light_ptr->Pdf() * _scene_ptr->LightCount();

			break;
		}

		if (info_ptr->_next == info_ptr)
			break;

		prev_material_ptr = material_ptr;
		if (prev_material_ptr)
			prev_texture_ptr = object_ptr->TexturePtr();
		prev_info_ptr = info_ptr;
		prev_wo = wo;

		path_ptr = (_t_path + depth_idx + 1);
		start = &path_ptr->_start;
		*start = *end;
		end = &path_ptr->_end;

		tracing = &path_ptr->_tracing;
		tracing->_o = start->_p;
		tracing->_d = w;

		info_ptr = &path_ptr->_info;
		info_ptr->_le = le;
		info_ptr->_in_object = in_object;

		object_ptr = 0x00;
		material_ptr = 0x00;
	}

	return n_vertex;
}

int BidirectionalPathTracer::ConstructLightPath(const int &max_depth, Pinhole *camera_ptr, Light *light_ptr)
{
	Path *path_ptr = _s_path;
	Ray *tracing = &path_ptr->_tracing;
	Vertex *start = &path_ptr->_start;
	Vertex *end = &path_ptr->_end;

	float pdf_wo = 1.0f;
	tracing->_d = light_ptr->SolidAngleSampling(&pdf_wo);
	tracing->_o = light_ptr->P();

	start->_pdf_wo = pdf_wo;
	start->_p = tracing->_o;
	start->_n = light_ptr->N(start->_p);
	start->_costheta_o = Dot(tracing->_d, start->_n);

	Intersection *info_ptr = &path_ptr->_info;
	bool le = true;
	bool in_object = false;
	info_ptr->_le = le;
	info_ptr->_in_object = in_object;
	*_alphaL = 1.0f;

	HomogeneousVolume *volume_ptr = _scene_ptr->_volume_ptr;
	float tnear = 0.0f;
	float tfar = 0.0f;
	float smax = 0.0f;
	float s = 0.0f;
	bool absorb = false;
	Color tau;
	Color pdf_s;
	Ray scattering;

	Shape *object_ptr = 0x00;
	Material *material_ptr = 0x00;
	Vector3 w;
	Vector3 wi;
	Vector3 n;
	float d;
	float dw_divide_da;
	float costheta_i;
	float costheta_o;
	float G;
	Vector3 prev_wi;
	Intersection *prev_info_ptr = 0x00;
	Material *prev_material_ptr = 0x00;
	Texture *prev_texture_ptr = 0x00;

	int n_vertex = 1;
	for (int depth_idx = 0; depth_idx < max_depth; ++depth_idx)
	{
		_scene_ptr->_bvh_ptr->Travel(tracing, info_ptr);

		if (volume_ptr && info_ptr->_in_object == false)
		{
			if (volume_ptr->Intersect(*tracing, &tnear, &tfar) == true)
			{
				smax = volume_ptr->MaxDistance(tracing->_tmin, tnear, tfar);
				info_ptr->_volumetric_smax = smax;

				if (volume_ptr->DistanceSampling(smax, 
					&info_ptr->_volumetric_s, &absorb, &info_ptr->_volumetric_tau, &info_ptr->_volumetric_pdf) == true)
				{
					if (tnear < 0.0f)
						end->_p = tracing->_o + tracing->_d * info_ptr->_volumetric_s;
					else
						end->_p = tracing->_o + tracing->_d * (tnear + info_ptr->_volumetric_s);

					end->_onsurface = false;
				}
			}

			// Absorb
			if (absorb == true)
			{
				if (depth_idx > 0)
					path_ptr->Clean();

				break;
			}
		}

		if (info_ptr->_object_ptr == 0x00 && info_ptr->_volumetric_s == 0.0f)
		{
			if (depth_idx > 0)
				path_ptr->Clean();

			break;
		}

		++n_vertex;
		object_ptr = info_ptr->_object_ptr;
		if (object_ptr)
			material_ptr = object_ptr->MaterialPtr();

		pdf_wo = 1.0f;
		wi = -tracing->_d;
		// Sampling on end vertex
		if (end->_onsurface == true)
		{
			n = end->_n;
			w = material_ptr->BSDFSampling(wi, n, &le, &in_object, &info_ptr->_rand, &end->_pdf_path, &end->_costheta_o, &pdf_wo);
			end->_pdf_wo = pdf_wo;
			end->_isdirac = material_ptr->IsDirac(info_ptr->_rand);
		}
		else
		{
			n = 0.0f;
			w = volume_ptr->PhaseSampling(info_ptr, &le, &in_object, &pdf_wo);
			end->_pdf_wo = pdf_wo;
			end->_isdirac = false;
		}

		// Compute costheta_o of start vertex
		if (start->_onsurface == true)
		{
			if (start->_isdirac == false)
				costheta_o = start->_costheta_o;
			else
			{
				if (info_ptr->_in_object == false)
					costheta_o = Dot(tracing->_d, start->_n);
				else
					costheta_o = -Dot(tracing->_d, start->_n);

				if (costheta_o <= 0.0f)
					costheta_o = 1e-4f;

				start->_costheta_o = costheta_o;
			}
		}
		else
		{
			 costheta_o = 1.0f;
			 start->_costheta_o = costheta_o;
		}

		// Compute costheta_i of end vertex
		if (end->_onsurface == true)
		{
			if (info_ptr->_in_object == false)
				costheta_i = -Dot(tracing->_d, end->_n);
			else
				costheta_i = Dot(tracing->_d, end->_n);

			if (costheta_i <= 0.0f)
				costheta_i = 1e-4f;

			end->_costheta_i = costheta_i;
		}
		else
		{
			costheta_i = 1.0f;
			end->_costheta_i = costheta_i;
		}

		// Calculate forward pdf of end vertex
		d = Length(end->_p - start->_p);
		dw_divide_da = costheta_i / (d * d);
		if (start->_onsurface == true)
		{
			if (start->_isdirac == false)
				end->_pdf_pforward = start->_pdf_wo * dw_divide_da;
			else
				end->_pdf_pforward = start->_pdf_wo * dw_divide_da * start->_costheta_o;
		}
		else
		{
			end->_pdf_pforward = start->_pdf_wo * dw_divide_da * start->_costheta_o;
		}

		if (volume_ptr && info_ptr->_in_object == false)
			end->_pdf_pforward *= info_ptr->_volumetric_pdf;

		// Calculate backward pdf of start vertex
		dw_divide_da = costheta_o / (d * d);
		if (start->_onsurface == true)
		{
			if (end->_onsurface == true)
			{
				if (material_ptr->IsLe() == true)
					start->_pdf_pbackward = light_ptr->Pdfw(wi) * dw_divide_da;
				else
				{
					if (end->_isdirac == false)
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, n, w, wi) * dw_divide_da;
					else
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, n, w, wi) * dw_divide_da * end->_costheta_i;
				}
				
				if (volume_ptr && info_ptr->_in_object == false)
					start->_pdf_pbackward *= info_ptr->_volumetric_pdf;
			}
			else
			{
				// Start vertex is always in volume
				// Always no intersection between start vertex and end vertex
				scattering._o = end->_p;
				scattering._d = -tracing->_d;
				scattering._tmax = Length(start->_p - end->_p);

				tnear = tfar = 0.0f;
				volume_ptr->Intersect(scattering, &tnear, &tfar);
				s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
				pdf_s = volume_ptr->Tr(scattering, 0.0f, s);
				start->_pdf_pbackward = volume_ptr->Pdfw(end->_p, w, wi) * dw_divide_da * end->_costheta_i * pdf_s;
			}
		}
		else
		{
			// Start vertex is always in volume
			// Always no intersection between start vertex and end vertex
			scattering._o = end->_p;
			scattering._d = -tracing->_d;
			scattering._tmax = Length(start->_p - end->_p);

			tnear = tfar = 0.0f;
			volume_ptr->Intersect(scattering, &tnear, &tfar);
			s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);

			pdf_s = 1.0f;
			tau = 1.0f;
			volume_ptr->PdfDistance(s, &tau, &pdf_s);

			if (end->_onsurface == true)
			{
				if (material_ptr->IsLe() == true)
					start->_pdf_pbackward = light_ptr->Pdfw(wi) * dw_divide_da * pdf_s;
				else
				{
					if (end->_isdirac == false)
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, n, w, wi) * dw_divide_da * pdf_s;
					else
						start->_pdf_pbackward = material_ptr->Pdfw(*info_ptr, n, w, wi) * dw_divide_da * end->_costheta_i * pdf_s;
				}
			}
			else
			{
				start->_pdf_pbackward = volume_ptr->Pdfw(end->_p, w, wi) * dw_divide_da * end->_costheta_i * pdf_s;
			}
		}

		if (depth_idx > 0)
			(_s_path + depth_idx - 1)->_end._pdf_pbackward = start->_pdf_pbackward;

		// Compute contribution
		G = costheta_o * costheta_i / (d * d);
		*(_alphaL + depth_idx + 1) = *(_alphaL + depth_idx);
		if (start->_onsurface == true)
		{
			if (depth_idx == 0)
			{
				*(_alphaL + depth_idx + 1) *= 
					light_ptr->L(*info_ptr) / start->_pdf_pforward * G;
			}
			else
			{
				*(_alphaL + depth_idx + 1) *=
					prev_material_ptr->Fr(*prev_info_ptr, start->_n, prev_wi, tracing->_d) / (start->_pdf_pforward * start->_pdf_path) * G;

				if (prev_texture_ptr)
					*(_alphaL + depth_idx + 1) *= prev_texture_ptr->Pattern(prev_info_ptr->_object_ptr, start->_p);
			}
		}
		else
		{
			*(_alphaL + depth_idx + 1) *=
				volume_ptr->Fp(start->_p, prev_wi, tracing->_d) / (start->_pdf_pforward * start->_pdf_path) * G;
		}

		if (volume_ptr && info_ptr->_in_object == false)
			*(_alphaL + depth_idx + 1) *= info_ptr->_volumetric_tau;

		if (end->_onsurface == true && material_ptr->IsLe() == true)
			break;

		if (info_ptr->_next == info_ptr)
			break;

		prev_material_ptr = material_ptr;
		if (prev_material_ptr)
			prev_texture_ptr = object_ptr->TexturePtr();
		prev_info_ptr = info_ptr;
		prev_wi = wi;

		path_ptr = (_s_path + depth_idx + 1);
		path_ptr->_start = *end;
		start = &path_ptr->_start;
		end = &path_ptr->_end;

		tracing = &path_ptr->_tracing;
		tracing->_o = start->_p;
		tracing->_d = w;

		info_ptr = &path_ptr->_info;
		info_ptr->_le = le;
		info_ptr->_in_object = in_object;

		object_ptr = 0x00;
		material_ptr = 0x00;
	}

	return n_vertex;
}

bool BidirectionalPathTracer::VisibilityTest(const int ns, const int nt, const Point3 &s_vertex, const Point3 &t_vertex)
{
	Ray test;

	if (ns == 0 || nt == 0)
		return false;

	if (nt == 1)
	{
		test._o = s_vertex;
		test._d = Normalize(t_vertex - s_vertex);
		test._tmax = Length(t_vertex - s_vertex);
	}
	if (ns == 1 && nt > 1)
	{
		test._o = t_vertex;
		test._d = Normalize(s_vertex - t_vertex);
		test._tmax = Length(s_vertex - t_vertex);
	}
	if (ns > 1 && nt > 1)
	{
		test._o = t_vertex;
		test._d = Normalize(s_vertex - t_vertex);
		test._tmax = Length(s_vertex - t_vertex);
	}

	_scene_ptr->_bvh_ptr->Travel(&test, 0x00);

	if (test._tmin > 0.0f && test._tmax - test._tmin > 1e-4f)
		return false;

	return true;
}

bool BidirectionalPathTracer::VolumetricTest(const int ns, const int nt, const Point3 &s_vertex, const Point3 &t_vertex, 
	float *s, Color *tau)
{
	if (_scene_ptr->_volume_ptr == 0x00)
		return true;

	HomogeneousVolume *volume_ptr = _scene_ptr->_volume_ptr;

	Ray scattering;
	scattering._o = s_vertex;
	scattering._d = Normalize(t_vertex - s_vertex);
	scattering._tmax = Length(t_vertex - s_vertex);

	float smax = 0.0f;
	float xi = 1.0f;
	float tnear = 0.0f;
	float tfar = 0.0f;

	if (volume_ptr->Intersect(scattering, &tnear, &tfar) == false)
		return true;

	*s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
	xi = RandomFloat();
	if (xi < 1.0f - exp(Average(volume_ptr->Sigma_t()) * -*s))
		return false;

	if (tnear > 0.0f)
	{
		if (scattering._tmax > tfar)
			*tau = volume_ptr->Tr(scattering, tnear, tfar);
		else
			*tau = volume_ptr->Tr(scattering, tnear, scattering._tmax);
	}
	else
	{
		if (scattering._tmax > tfar)
			*tau = volume_ptr->Tr(scattering, 0.0f, tfar);
		else
			*tau = volume_ptr->Tr(scattering, 0.0f, scattering._tmax);
	}

	return true;
}

float BidirectionalPathTracer::MISWeight(Pinhole *camera_ptr, Light *light_ptr, 
	const int ns, const int nt, const Vector3 &connect_wi, const Vector3 &connect_wo)
{
	if (ns + nt <= 2)
		return 1.0f;

	Path *path_ptr = 0x00;
	Vertex *s_prev = 0x00;
	Vertex *s_connect = 0x00;
	Intersection *s_info_ptr = 0x00;
	Material *s_material_ptr = 0x00;

	Vector3 s_wi;
	Vertex *t_prev = 0x00;
	Vertex *t_connect = 0x00;
	Intersection *t_info_ptr = 0x00;
	Material *t_material_ptr = 0x00;

	Vector3 t_wo;
	float pdf_w = 1.0f;
	float dw_divide_da = 1.0f;
	float costheta = 1.0f;
	float sd = 1.0f;
	float td = 1.0f;
	float connect_d = 1.0f;

	HomogeneousVolume *volume_ptr = _scene_ptr->_volume_ptr;
	Ray scattering;
	float tnear;
	float tfar;
	float s;
	Color tau;
	Color pdf_s;

	if (ns > 0)
	{
		if (ns == 1)
			s_connect = &_s_path->_start;
		else
		{
			path_ptr = _s_path + ns - 2;
			s_prev = &path_ptr->_start;
			s_connect = &path_ptr->_end;
			s_wi = -(path_ptr->_tracing._d);
			s_info_ptr = &path_ptr->_info;
			if (s_info_ptr->_object_ptr)
				s_material_ptr = s_info_ptr->_object_ptr->MaterialPtr();
		}
	}

	if (nt > 0)
	{
		if (nt == 1)
			t_connect = &_t_path->_start;
		else
		{
			path_ptr = _t_path + nt - 2;
			t_prev = &path_ptr->_start;
			t_connect = &path_ptr->_end;
			t_wo = -(path_ptr->_tracing._d);
			t_info_ptr = &path_ptr->_info;
			if (t_info_ptr->_object_ptr)
				t_material_ptr = t_info_ptr->_object_ptr->MaterialPtr();
		}
	}

	if (s_connect && t_connect)
		connect_d = Length(s_connect->_p - t_connect->_p);
	if (s_connect && s_prev)
		sd = Length(s_connect->_p - s_prev->_p);
	if (t_connect && t_prev)
		td = Length(t_connect->_p - t_prev->_p);

	if (s_connect)
	{
		if (s_connect->_onsurface == true)
		{
			pdf_s = 1.0f;
			if (volume_ptr)
			{
				scattering._o = t_connect->_p;
				scattering._d = connect_wi;
				scattering._tmax = connect_d;

				tnear = tfar = 0.0f;
				if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
				{
					s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
					pdf_s = volume_ptr->Tr(scattering, 0.0f, s);
				}
			}

			costheta = Dot(s_connect->_n, connect_wo);
			dw_divide_da = costheta / (connect_d * connect_d);

			if (t_connect->_onsurface == true)
			{
				if (nt > 1)
					pdf_w = t_material_ptr->Pdfw(*t_info_ptr, t_connect->_n, t_wo, connect_wi);
				else
					pdf_w = camera_ptr->Pdfw(connect_wi);

				*(_s_pdfbackward + ns - 1) = pdf_w * dw_divide_da * pdf_s;
			}
			else
			{
				pdf_w = volume_ptr->Pdfw(t_connect->_p, t_wo, connect_wi);
				*(_s_pdfbackward + ns - 1) = pdf_w * dw_divide_da * 1.0f * pdf_s;
			}

			*(_s_pdfforward + ns - 1) = s_connect->_pdf_pforward;
		}
		else
		{
			costheta = 1.0f;
			dw_divide_da = costheta / (connect_d * connect_d);

			scattering._o = t_connect->_p;
			scattering._d = connect_wi;
			scattering._tmax = connect_d;

			tau = 1.0f;
			pdf_s = 1.0f;
			tnear = tfar = 0.0f;
			if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
			{
				s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
				volume_ptr->PdfDistance(s, &tau, &pdf_s);
			}

			if (t_connect->_onsurface == true)
			{
				if (nt > 1)
					pdf_w = t_material_ptr->Pdfw(*t_info_ptr, t_connect->_n, t_wo, connect_wi);
				else
					pdf_w = camera_ptr->Pdfw(connect_wi);

				*(_s_pdfbackward + ns - 1) = pdf_w * dw_divide_da * pdf_s;
			}
			else
			{
				pdf_w = volume_ptr->Pdfw(t_connect->_p, t_wo, connect_wi);
				*(_s_pdfbackward + ns - 1) = pdf_w * dw_divide_da * 1.0f * pdf_s;
			}

			*(_s_pdfforward + ns - 1) = s_connect->_pdf_pforward;
		}

		if (s_prev)
		{
			if (s_prev->_onsurface == true)
			{
				if (s_prev->_isdirac == true)
				{
					*(_s_pdfbackward + ns - 2) = 1.0f;
					*(_s_pdfforward + ns - 2) = 1.0f;
				}
				else
				{
					pdf_s = 1.0f;
					if (volume_ptr)
					{
						scattering._o = s_connect->_p;
						scattering._d = s_wi;
						scattering._tmax = sd;

						tnear = tfar = 0.0f;
						if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
						{
							s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
							pdf_s = volume_ptr->Tr(scattering, 0.0f, s);
						}
					}

					costheta = -Dot(s_prev->_n, s_wi);
					dw_divide_da = costheta / (sd * sd);

					if (s_connect->_onsurface == true)
					{
						pdf_w = s_material_ptr->Pdfw(*s_info_ptr, s_connect->_n, connect_wo, s_wi);
						*(_s_pdfbackward + ns - 2) = pdf_w * dw_divide_da * pdf_s;
					}
					else
					{
						pdf_w = volume_ptr->Pdfw(s_connect->_p, connect_wo, s_wi);
						*(_s_pdfbackward + ns - 2) = pdf_w * dw_divide_da * 1.0f * pdf_s;
					}

					*(_s_pdfforward + ns - 2) = s_prev->_pdf_pforward;
				}
			}
			else
			{
				costheta = 1.0f;
				dw_divide_da = costheta / (sd * sd);

				scattering._o = s_connect->_p;
				scattering._d = s_wi;
				scattering._tmax = sd;

				tau = 1.0f;
				pdf_s = 1.0f;
				tnear = tfar = 0.0f;
				if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
				{
					s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
					volume_ptr->PdfDistance(s, &tau, &pdf_s);
				}

				if (s_connect->_onsurface == true)
				{
					pdf_w = s_material_ptr->Pdfw(*s_info_ptr, s_connect->_n, connect_wo, s_wi);
					*(_s_pdfbackward + ns - 2) = pdf_w * dw_divide_da * pdf_s;
				}
				else
				{
					pdf_w = volume_ptr->Pdfw(s_connect->_p, connect_wo, s_wi);
					*(_s_pdfbackward + ns - 2) = pdf_w * dw_divide_da * 1.0f * pdf_s;
				}

				*(_s_pdfforward + ns - 2) = s_prev->_pdf_pforward;
			}
		}
	}

	if (t_connect)
	{
		if (ns > 0)
		{
			if (t_connect->_onsurface == true)
			{
				pdf_s = 1.0f;
				if (volume_ptr)
				{
					scattering._o = s_connect->_p;
					scattering._d = connect_wo;
					scattering._tmax = connect_d;

					tnear = tfar = 0.0f;
					if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
					{
						s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
						pdf_s = volume_ptr->Tr(scattering, 0.0f, s);
					}
				}

				costheta = Dot(t_connect->_n, connect_wi);
				dw_divide_da = costheta / (connect_d * connect_d);

				if (s_connect->_onsurface == true)
				{
					if (ns > 1)
						pdf_w = s_material_ptr->Pdfw(*s_info_ptr, s_connect->_n, s_wi, connect_wo);
					else
						pdf_w = light_ptr->Pdfw(connect_wo);

					*(_t_pdfbackward + nt - 1) = pdf_w * dw_divide_da * pdf_s;
				}
				else
				{
					pdf_w = volume_ptr->Pdfw(s_connect->_p, s_wi, connect_wo);
					*(_t_pdfbackward + nt - 1) = pdf_w * dw_divide_da * 1.0f * pdf_s;
				}

				*(_t_pdfforward + nt - 1) = t_connect->_pdf_pforward;
			}
			else
			{
				costheta = 1.0f;
				dw_divide_da = costheta / (connect_d * connect_d);

				scattering._o = s_connect->_p;
				scattering._d = connect_wo;
				scattering._tmax = connect_d;

				tau = 1.0f;
				pdf_s = 1.0f;
				tnear = tfar = 0.0f;
				if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
				{
					s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
					volume_ptr->PdfDistance(s, &tau, &pdf_s);
				}

				if (s_connect->_onsurface == true)
				{
					if (ns > 1)
						pdf_w = s_material_ptr->Pdfw(*s_info_ptr, s_connect->_n, s_wi, connect_wo);
					else
						pdf_w = light_ptr->Pdfw(connect_wo);

					*(_t_pdfbackward + nt - 1) = pdf_w * dw_divide_da * pdf_s;
				}
				else
				{
					pdf_w = volume_ptr->Pdfw(s_connect->_p, s_wi, connect_wo);
					*(_t_pdfbackward + nt - 1) = pdf_w * dw_divide_da * 1.0f * pdf_s;
				}

				*(_t_pdfforward + nt - 1) = t_connect->_pdf_pforward;
			}

			if (t_prev)
			{
				if (t_prev->_onsurface == true)
				{
					if (t_prev->_isdirac == true)
					{
						*(_t_pdfbackward + nt - 2) = 1.0f;
						*(_t_pdfforward + nt - 2) = 1.0f;
					}
					else
					{
						pdf_s = 1.0f;
						if (volume_ptr)
						{
							scattering._o = t_connect->_p;
							scattering._d = t_wo;
							scattering._tmax = td;

							tnear = tfar = 0.0f;
							if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
							{
								s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
								pdf_s = volume_ptr->Tr(scattering, 0.0f, s);
							}
						}

						costheta = -Dot(t_prev->_n, t_wo);
						dw_divide_da = costheta / (td * td);

						if (t_connect->_onsurface == true)
						{
							pdf_w = t_material_ptr->Pdfw(*t_info_ptr, t_connect->_n, connect_wi, t_wo);
							*(_t_pdfbackward + nt - 2) = pdf_w * dw_divide_da * pdf_s;
						}
						else
						{
							pdf_w = volume_ptr->Pdfw(t_connect->_p, connect_wi, t_wo);
							*(_t_pdfbackward + nt - 2) = pdf_w * dw_divide_da * 1.0f * pdf_s; 
						}

						*(_t_pdfforward + nt - 2) = t_prev->_pdf_pforward;
					}
				}
				else
				{
					costheta = 1.0f;
					dw_divide_da = costheta / (td * td);

					scattering._o = t_connect->_p;
					scattering._d = t_wo;
					scattering._tmax = td;

					tau = 1.0f;
					pdf_s = 1.0f;
					tnear = tfar = 0.0f;
					if (volume_ptr->Intersect(scattering, &tnear, &tfar) == true)
					{
						s = volume_ptr->MaxDistance(scattering._tmax, tnear, tfar);
						volume_ptr->PdfDistance(s, &tau, &pdf_s);
					}

					if (t_connect->_onsurface == true)
					{
						pdf_w = t_material_ptr->Pdfw(*t_info_ptr, t_connect->_n, connect_wi, t_wo);
						*(_t_pdfbackward + nt - 2) = pdf_w * dw_divide_da * pdf_s;
					}
					else
					{
						pdf_w = volume_ptr->Pdfw(t_connect->_p, connect_wi, t_wo);
						*(_t_pdfbackward + nt - 2) = pdf_w * dw_divide_da * 1.0f * pdf_s;
					}

					*(_t_pdfforward + nt - 2) = t_prev->_pdf_pforward;
				}
			}
		}
		else
		{
			// kajiya path tracing
			*(_t_pdfbackward + nt - 1) = (_t_path + nt - 2)->_end._pdf_pbackward;
			*(_t_pdfforward + nt - 1) = (_t_path +  nt - 2)->_end._pdf_pforward;

			if (t_prev->_isdirac == true)
			{
				*(_t_pdfbackward + nt - 2) = 1.0f;
				*(_t_pdfforward + nt - 2) = 1.0f;
			}
			else
			{
				*(_t_pdfbackward + nt - 2) = (_t_path + nt - 2)->_start._pdf_pbackward;
				*(_t_pdfforward + nt - 2) = (_t_path +  nt - 2)->_start._pdf_pforward;
			}
		}
	}

	// s vertice pdf
	for (int idx = 0; idx < ns - 2; ++idx)
	{
		if ((_s_path + idx)->_start._isdirac)
		{
			*(_s_pdfforward + idx) = 1.0f;
			*(_s_pdfbackward + idx) = 1.0f;
		}
		else
		{
			*(_s_pdfforward + idx) = (_s_path + idx)->_start._pdf_pforward;
			*(_s_pdfbackward + idx) = (_s_path + idx)->_start._pdf_pbackward;
		}
	}

	// t vertice pdf
	for (int idx = 0; idx < nt - 2; ++idx)
	{
		if ((_t_path + idx)->_start._isdirac)
		{
			*(_t_pdfforward + idx) = 1.0f;
			*(_t_pdfbackward + idx) = 1.0f;
		}
		else
		{
			*(_t_pdfforward + idx) = (_t_path + idx)->_start._pdf_pforward;
			*(_t_pdfbackward + idx) = (_t_path + idx)->_start._pdf_pbackward;
		}
	}

	int locked = -1;
	if ((_t_path + 0)->_start._isdirac == true && 
		(_t_path + 0)->_end._isdirac == true)
	{
		int idx = 0;
		for (; idx < nt - 1; ++idx)
		{
			if ((_t_path + idx)->_start._isdirac == true ||
				(_t_path + idx)->_end._isdirac == true)
				continue;

			locked = idx;
			break;
		}

		if (idx == nt - 1)
			locked = idx;
	}

	float sum_pi = 1.0f;
	float pi = 1.0f;
	for (int idx = nt - 1; idx > 0; --idx)
	{
		pi *= Average(*(_t_pdfbackward + idx) / *(_t_pdfforward + idx));

		if (locked != -1 && idx <= locked)
			break;

		if (idx < nt - 1 && (_t_path + idx)->_start._isdirac == true)
			continue;

		if (idx == 0 && _t_path->_start._isdirac == true)
			continue;

		sum_pi += pi * pi;
	}

	pi = 1.0f;
	for (int idx = ns - 1; idx > 0; --idx)
	{
		pi *= Average(*(_s_pdfbackward + idx) / *(_s_pdfforward + idx));

		if (0 < idx && idx < ns - 1 && (_s_path + idx)->_start._isdirac == true)
			continue;

		if (idx == 0 && _s_path->_start._isdirac == true)
			continue;

		sum_pi += pi * pi;
	}

	if (ns > 0)
	{
		memset(_s_pdfforward, 0, sizeof(float) * ns);
		memset(_s_pdfbackward, 0, sizeof(float) * ns);
	}
	if (nt > 0)
	{
		memset(_t_pdfforward, 0, sizeof(float) * nt);
		memset(_t_pdfbackward, 0, sizeof(float) * nt);
	}

	return 1.0f / sum_pi;
}