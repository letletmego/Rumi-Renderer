#include "dielectric.h"
#include "..\\Kernel\\bsdf.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Light\\light.h"
#include "..\\Volume\\homogeneousvolume.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Integrator\\integrator.h"
#include "..\\Integrator\\path.h"
#include "..\\Scene\\scene.h"


Dielectric::~Dielectric(void)
{
	DELETE(_fresnel_brdf)
	DELETE(_fresnel_btdf)
}

Dielectric::Dielectric(void)
	: Material(false)
	, _fresnel_brdf(new FresnelReflection)
	, _fresnel_btdf(new FresnelTransparency)
{
}

Dielectric::Dielectric(float eta_in, float eta_out, Color absorption)
	: Material(false)
	, _fresnel_brdf(new FresnelReflection(Color(1.0f, 1.0f, 1.0f)))
	, _fresnel_btdf(new FresnelTransparency(eta_in, eta_out, absorption))
{
}

bool Dielectric::IsDirac(const float &xi)
{
	return true;
}

float Dielectric::Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	return 1.0f;
}

Vector3 Dielectric::BSDFSampling(const Vector3 &wo, const Vector3 &normal, 
	bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const
{
	Vector3 w;

	float kr = _fresnel_brdf->Fresnel(-wo, normal, _fresnel_btdf->_eta_in, _fresnel_btdf->_eta_out);
	float kt = 1.0f - kr;

	if (kr && RandomFloat() <= kr)
	{
		w = _fresnel_brdf->BSDFSampling(wo, normal, costheta, pdf_w);
		*go_into_object = *costheta < 0.0f; // next go in object
		*xi = 1.0f;
		*pdf_path = kr;
	}
	else
	{
		w = _fresnel_btdf->BSDFSampling(wo, normal, costheta, pdf_w);
		*go_into_object = *costheta > 0.0f; // next go in object
		*xi = 0.0f;
		*pdf_path = kt;
	}

	*Le = true;
	*costheta = 1.0f;
	*pdf_w = 1.0f;

	return w;
}

Color Dielectric::Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo)
{
	Color fr;

	bool in_object = info._in_object;
	float k = info._vertex_ptr->_pdf_path;

	if (info._rand == 1.0f)
	{
		fr = _fresnel_brdf->F(normal, wi, wo) * k;
	}
	else
	{
		if (in_object == true)
			_fresnel_btdf->_eta = _fresnel_btdf->_eta_in / _fresnel_btdf->_eta_out;
		else
			_fresnel_btdf->_eta = _fresnel_btdf->_eta_out / _fresnel_btdf->_eta_in;

		fr = _fresnel_btdf->F(normal, wi, wo) * k;
	}

	if (in_object == true)
		fr *= BeerLambertLaw(_fresnel_btdf->_absorption, info._ray_ptr->_tmin);

	return fr;
}

Color Dielectric::DirectLighting(const Intersection &info)
{
	return Color();
}

Color Dielectric::GlobalIllumination(const Intersection &info)
{
	Ray tracing;
	Color f;
	Color Li;
	Color lo;
	Point3 p = info._vertex_ptr->_p;
	Vector3 n = info._vertex_ptr->_n;
	Vector3 wo = -info._ray_ptr->_d;

	bool entering = false;
	float kr = _fresnel_brdf->Fresnel(info._ray_ptr->_d, n, _fresnel_btdf->_eta_in, _fresnel_btdf->_eta_out);
	float kt = 1.0f - kr;
	float costheta = 1.0f;
	float pdf_w = 1.0f;
	float inv_path_pdf = 1.0f;

	// Path sampling
	if (kr && RandomFloat() <= kr)
	{
		// Reflection
		Vector3 wi = _fresnel_brdf->BSDFSampling(wo, n, &costheta, &pdf_w);
		entering = costheta < 0.0f;
		tracing._o = p;
		tracing._d = wi;

		Li = info._scene_ptr->_tracer_ptr->Trace(&tracing, info._trace_depth + 1, true, entering);
		f = _fresnel_brdf->F(n, wi, wo) * kr;
		lo = f * Li / pdf_w;
		inv_path_pdf = 1.0f / kr;
	}
	else
	{
		// Refraction
		Vector3 wt = _fresnel_btdf->BSDFSampling(wo, n, &costheta, &pdf_w);
		entering = costheta > 0.0f;
		tracing._o = p;
		tracing._d = wt;

		Li = info._scene_ptr->_tracer_ptr->Trace(&tracing, info._trace_depth + 1, true, entering);

		if (info._in_object == true)
			_fresnel_btdf->_eta = _fresnel_btdf->_eta_in / _fresnel_btdf->_eta_out;
		else
			_fresnel_btdf->_eta = _fresnel_btdf->_eta_out / _fresnel_btdf->_eta_in;
		
		f = _fresnel_btdf->F(n, wt, wo) * kt;
		lo = f * Li / pdf_w;
		inv_path_pdf = 1.0f / kt;
	}

	if (entering == true)
		lo *= BeerLambertLaw(_fresnel_btdf->_absorption, tracing._tmin);

	return lo * inv_path_pdf;
}