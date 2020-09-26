#include "matte.h"
#include "..\\Kernel\\bsdf.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\shape.h"
#include "..\\Light\\light.h"
#include "..\\Volume\\homogeneousvolume.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Integrator\\integrator.h"
#include "..\\Integrator\\path.h"
#include "..\\Scene\\scene.h"


Matte::~Matte(void)
{
	DELETE(_diffuse_brdf)
}

Matte::Matte(void)
	: Material(true)
	, _kd(0.0f)
	, _diffuse_brdf(new Lambertian)
{
}

Matte::Matte(const float kd, const Color rgb)
	: Material(true)
	, _kd(kd)
	, _diffuse_brdf(new Lambertian(rgb))
{
}

float Matte::Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	return _diffuse_brdf->Pdfw(normal, w, w_pdf);
}

Vector3 Matte::BSDFSampling(const Vector3 &wo, const Vector3 &normal, 
	bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const
{
	*Le = false;
	//*Le = true;
	*go_into_object = false;
	*xi = 1.0f;

	return _diffuse_brdf->BSDFSampling(wo, normal, costheta, pdf_w);
}

Color Matte::Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo)
{
	return _diffuse_brdf->F(normal, wi ,wo) * _kd;
}

Color Matte::DirectLighting(const Intersection &info)
{
	float lights_sampling_pdf;
	Light *light_ptr;
	Point3 p(info._vertex_ptr->_p);
	Vector3 n(info._vertex_ptr->_n);
	Vector3 wi;
	Vector3 wo(-info._ray_ptr->_d);
	Ray shadow_ray;
	Color lo;

	//if (info._scene_ptr->_ambient)
	//	lo += _diffuse_brdf->F(n, wi, wo) * _kd * info._scene_ptr->_ambient->L(info);

	if (info._scene_ptr->VisibilityTest(p, n, &light_ptr, &lights_sampling_pdf, &wi, &shadow_ray) == false)
		return lo;

	Color f = _diffuse_brdf->F(n, wi, wo) * _kd;
	Color Li = light_ptr->L(info);
	float costheta = __max(Dot(wi, n), 0.0f);
	float g = light_ptr->G(p);
	lo += f * Li * costheta * g / lights_sampling_pdf;

	HomogeneousVolume *volume_ptr = info._scene_ptr->_volume_ptr;
	if (volume_ptr)
		lo *= volume_ptr->ShadowRayTr(shadow_ray);

	return lo;
}

Color Matte::GlobalIllumination(const Intersection &info)
{	
	float costheta;
	float bsdf_pdf;
	Point3 p = info._vertex_ptr->_p;
	Vector3 n = info._vertex_ptr->_n;
	Vector3 wo = -info._ray_ptr->_d;
	Vector3 wi = _diffuse_brdf->BSDFSampling(wo, n, &costheta, &bsdf_pdf);
	Ray scattering(p, wi);

	Color f = _diffuse_brdf->F(n, wi, wo) * _kd;
	Color Li = info._scene_ptr->_tracer_ptr->Trace(&scattering, info._trace_depth + 1, false);
	Color indirect_lo = f * Li * costheta / bsdf_pdf;
	Color direct_lo = DirectLighting(info);

	return direct_lo + indirect_lo;
}