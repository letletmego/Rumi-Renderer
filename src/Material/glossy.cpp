#include "glossy.h"
#include "..\\Kernel\\bsdf.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Shape\\shape.h"
#include "..\\Light\\light.h"
#include "..\\Volume\\homogeneousvolume.h"
#include "..\\Integrator\\integrator.h"
#include "..\\Integrator\\path.h"
#include "..\\Scene\\scene.h"


Glossy::~Glossy(void)
{
	DELETE(_phong_brdf)
}

Glossy::Glossy(void)
	: Matte()
	, _ks(0.0f)
	, _kd_pdf(0.0f)
	, _phong_brdf(new Phong)
{
}

Glossy::Glossy(const float kd, const float ks, const float e, const Color rgb)
	: Matte(kd, rgb)
	, _ks(ks)
	, _kd_pdf(kd / (kd + ks))
	, _phong_brdf(new Phong(e, rgb))
{
}

bool Glossy::IsDirac(const float &xi)
{
	if (_kd && xi <= _kd_pdf)
		return false;

	return true;
}

float Glossy::Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	float pdf_w = 1.0f;

	if (_kd && info._rand <= _kd_pdf)
		pdf_w = _diffuse_brdf->Pdfw(normal, w, w_pdf);
	else
		pdf_w = _phong_brdf->Pdfw(normal, w, w_pdf);


	return pdf_w;
}

Vector3 Glossy::BSDFSampling(const Vector3 &wo, const Vector3 &normal, 
	bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const
{
	Vector3 wi;
	*xi = RandomFloat();

	if (_kd && *xi <= _kd_pdf)
	{
		wi = _diffuse_brdf->BSDFSampling(wo, normal, costheta, pdf_w);
		*Le = false;
		*go_into_object = false;
		*pdf_path = _kd_pdf;
	}
	else
	{
		wi = _phong_brdf->BSDFSampling(wo, normal, costheta, pdf_w);
		*Le = true;
		*go_into_object = false;
		*pdf_path = 1.0f - _kd_pdf;
	}

	return wi;
}

Color Glossy::Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo)
{
	Color f;

	if (_kd && info._rand <= _kd_pdf)
		f = _diffuse_brdf->F(normal, wi, wo) * _kd;
	else
		f = _phong_brdf->F(normal, wi, wo) * _ks;

	return f;
}

Color Glossy::DirectLighting(const Intersection &info)
{
#if 1
	Color lo;

	if (_kd && info._rand <= _kd_pdf)
		lo = Matte::DirectLighting(info) / _kd_pdf;
	else
		lo = Color();

	return lo;
#else
	float lights_sampling_pdf;
	Light *light_ptr;
	Point3 p = info._vertex_ptr->_p;
	Vector3 n = info._vertex_ptr->_n;
	Vector3 wi;
	Vector3 wo(-info._ray_ptr->_d);
	Ray shadow_ray;
	
	if (info._scene_ptr->VisibilityTest(p, n, &light_ptr, &lights_sampling_pdf, &wi, &shadow_ray) == false)
		return Color(0.0f);

	Color f = _diffuse_brdf->F(n, wi, wo) * _kd + _phong_brdf->F(n, wi, wo) * _ks;
	Color Li = light_ptr->L(info);
	float costheta = __max(Dot(wi, n), 0.0f);
	float g = light_ptr->G(p);

	Color lo = f * Li * costheta * g / lights_sampling_pdf;

	if (info._scene_ptr->_volume_ptr)
		lo *= info._scene_ptr->_volume_ptr->Tr(shadow_ray);

	return lo;
#endif
}

Color Glossy::GlobalIllumination(const Intersection &info)
{
	Color direct_lo;
	Color indirect_lo;
	Color f;
	Color Li;
	Point3 p = info._vertex_ptr->_p;
	Vector3 n = info._vertex_ptr->_n;
	Vector3 wo = -info._ray_ptr->_d;
	Vector3 wi;

	float costheta = 1.0f;
	float pdf_w = 1.0f;
	float inv_path_pdf = 1.0f;

	// Path sampling
	if (_kd && RandomFloat() <= _kd_pdf)
	{
		inv_path_pdf = 1.0f / _kd_pdf;

		wi = _diffuse_brdf->BSDFSampling(wo, n, &costheta, &pdf_w);
		Ray scattering(p, wi);
		f = _diffuse_brdf->F(n, wi, wo) * _kd;
		Li = info._scene_ptr->_tracer_ptr->Trace(&scattering, info._trace_depth + 1, false);
		indirect_lo = f * Li * costheta / pdf_w;
		//direct_lo = Matte::DirectLighting(info);
	}
	else
	{
		inv_path_pdf = 1.0f / (1.0f - _kd_pdf);

		wi = _phong_brdf->BSDFSampling(wo, n, &costheta, &pdf_w);
		Ray reflection(p, wi);
		f = _phong_brdf->F(n, wi, wo) * _ks;
		Li = info._scene_ptr->_tracer_ptr->Trace(&reflection, info._trace_depth + 1, true);
		indirect_lo = f * Li * costheta / pdf_w;
	}

	return (direct_lo + indirect_lo) * inv_path_pdf;
}