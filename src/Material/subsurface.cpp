#include "subsurface.h"
#include "..\\Kernel\\bsdf.h"
#include "..\\Kernel\\phase.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Light\\light.h"
#include "..\\Volume\\homogeneousvolume.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Integrator\\integrator.h"
#include "..\\Integrator\\path.h"
#include "..\\Scene\\scene.h"


Subsurface::~Subsurface(void)
{
	DELETE(_fresnel_brdf)
	DELETE(_fresnel_btdf)
	DELETE(_phase)
}

Subsurface::Subsurface(void)
	: Material(false)
	, _fresnel_brdf(new FresnelReflection)
	, _fresnel_btdf(new FresnelTransparency)
	, _phase(new Isotropic)
	, _sigma_s()
	, _sigma_a()
	, _sigma_t()
	, _avg_sigmat()
	, _absorb()
{
}

Subsurface::Subsurface(float eta_in, float eta_out, Color sigma_s, Color sigma_a, float density)
	: Material(false)
	, _fresnel_brdf(new FresnelReflection(Color(1.0f, 1.0f, 1.0f)))
	, _fresnel_btdf(new FresnelTransparency(eta_in, eta_out, sigma_s + sigma_a))
	, _phase(new Isotropic())
	, _sigma_s(sigma_s)
	, _sigma_a(sigma_a)
	, _sigma_t(sigma_s + sigma_a)
	, _avg_sigmat()
	, _absorb()
{
	_avg_sigmat = (_sigma_t._b + _sigma_t._g + _sigma_t._r) / 3.0f * density;
	_absorb = ((_sigma_a._b / _sigma_t._b) + (_sigma_a._g / _sigma_t._g) + (_sigma_a._r / _sigma_t._r)) / 3.0f;
}

bool Subsurface::DistanceSampling(const float &tnear, const float &tfar, bool *absorb, float *s, float *smax, Color *tau, Color *pdf)
{
	*smax = tfar;

	float xi = RandomFloat();

	// Over the sampling distance
	if (xi >= 1.0f - exp(_avg_sigmat * -(*smax)))
	{
		tau->_b = exp(_sigma_t._b * -(*smax - 0.0f));
		tau->_g = exp(_sigma_t._g * -(*smax - 0.0f));
		tau->_r = exp(_sigma_t._r * -(*smax - 0.0f));
		if (*tau == 0.0f)
			*tau = 1e-4f;
		
		*pdf = *tau;

		return false;
	}

	// Distance sampling
	float ds = -log(1.0f - xi) / _avg_sigmat;
	if (*smax - ds < 1e-4f)
	{
		tau->_b = exp(_sigma_t._b * -(*smax - 0.0f));
		tau->_g = exp(_sigma_t._g * -(*smax - 0.0f));
		tau->_r = exp(_sigma_t._r * -(*smax - 0.0f));
		if (*tau == 0.0f)
			*tau = 1e-4f;
		*pdf = *tau;

		return false;
	}

	if (RandomFloat() < _absorb || ds < 1e-4f)
	{
		*s = -1.0f;
		*absorb = true;
		return false;
	}

	*s = ds;
	tau->_b = exp(_sigma_t._b * -(ds - 0.0f));
	tau->_g = exp(_sigma_t._g * -(ds - 0.0f));
	tau->_r = exp(_sigma_t._r * -(ds - 0.0f));
	*pdf = *tau * _sigma_t;
	if (*pdf == 0.0f)
		*pdf = 1e-4f;

	return true;
}

bool Subsurface::IsSubsurface(void)
{
	return true;
}

Vector3 Subsurface::BSDFSampling(const Vector3 &wo, const Vector3 &normal, 
	bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const
{
	Vector3 w;

	float kr = _fresnel_brdf->Fresnel(-wo, normal, _fresnel_btdf->_eta_in, _fresnel_btdf->_eta_out);
	float kt = 1.0f - kr;

	if (kr && RandomFloat() <= kr)
	{
		w = _fresnel_brdf->BSDFSampling(wo, normal, costheta, pdf_w);
		*go_into_object = *costheta < 0.0f;
		*xi = 1.0f;
		*pdf_path = kr;
	}
	else
	{
		w = _fresnel_btdf->BSDFSampling(wo, normal, costheta, pdf_w);
		*go_into_object = *costheta > 0.0f;
		*xi = 0.0f;
		*pdf_path = kt;
	}

	*Le = true;
	*costheta = 1.0f;

	return w;
}

bool Subsurface::PhaseSampling(Intersection *info, bool *Le, bool *go_into_object, Vector3 *wi, float *pdf_w)
{
	Vector3 wo = -info->_ray_ptr->_d;
	bool absorb = false;

	if (DistanceSampling(0.0f, info->_ray_ptr->_tmin, &absorb, 
		&info->_volumetric_s, &info->_volumetric_smax, &info->_volumetric_tau, &info->_volumetric_pdf) == false)
	{
		if (absorb == true)
			info->_volumetric_s = -1.0f;

		return false; // No sampling or absorb
	}

	*Le = true;
	*go_into_object = true;
	info->_vertex_ptr->_p = info->_ray_ptr->_o + info->_ray_ptr->_d * info->_volumetric_s;
	*wi = _phase->PhaseSampling(-wo, pdf_w);

	return true;
}

Color Subsurface::Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo)
{
	Color fr;

	float k = info._vertex_ptr->_pdf_path;
	bool in_object = info._in_object;

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
		fr *= (info._volumetric_tau / info._volumetric_pdf);

	return fr;
}

Color Subsurface::Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo)
{
	return _phase->Fp(position, wi, wo) * _sigma_s; // radiance * sigma_s * phase / pdf_phase
}

Color Subsurface::DirectLighting(const Intersection &info)
{
	return Color();
}

Color Subsurface::GlobalIllumination(const Intersection &info)
{
	Color f;
	Color Li;
	Color lo;
	Point3 p = info._vertex_ptr->_p;
	Vector3 n = info._vertex_ptr->_n;
	Vector3 wo = -info._ray_ptr->_d;
	float pdf_w = 1.0f;
	bool absorb = false;
	float s = 0.0f;
	float smax = 0.0f;
	Color distance_tau;
	Color distance_pdf;

	if (info._in_object == true &&
		DistanceSampling(0.0f, info._ray_ptr->_tmin, &absorb, 
		&s, &smax, &distance_tau, &distance_pdf) == true)
	{
		Vector3 wi = _phase->PhaseSampling(wo, &pdf_w);
		Ray in_scattering(info._ray_ptr->_o + info._ray_ptr->_d * s, wi);
		lo = info._scene_ptr->_tracer_ptr->Trace(&in_scattering, info._trace_depth + 1, true, true) * _sigma_s;

		return lo * distance_tau / distance_pdf;
	}

	if (absorb == true)
		return lo;

	float kr = _fresnel_brdf->Fresnel(-wo, n, _fresnel_btdf->_eta_in, _fresnel_btdf->_eta_out);
	float kt = 1.0f - kr;
	float costheta = 1.0f;
	pdf_w = 1.0f;
	float inv_path_pdf = 0.0f;
	bool entering = false;
	Ray tracing;

	// Path sampling
	if (kr && RandomFloat() <= kr)
	{
		Vector3 wi = _fresnel_brdf->BSDFSampling(wo, n, &costheta, &pdf_w);
		entering = costheta < 0.0f;
		tracing._o = p;
		tracing._d = wi;

		f = _fresnel_brdf->F(n, wi, wo) * kr;
		Li = info._scene_ptr->_tracer_ptr->Trace(&tracing, info._trace_depth + 1, true, entering);
		lo = f * Li / pdf_w;
		inv_path_pdf = 1.0f / kr;
	}
	else
	{
		Vector3 wt = _fresnel_btdf->BSDFSampling(wo, n, &costheta, &pdf_w);
		entering = costheta > 0.0f;
		tracing._o = p;
		tracing._d = wt;

		if (info._in_object == true)
			_fresnel_btdf->_eta = _fresnel_btdf->_eta_in / _fresnel_btdf->_eta_out;
		else
			_fresnel_btdf->_eta = _fresnel_btdf->_eta_out / _fresnel_btdf->_eta_in;

		f = _fresnel_btdf->F(n, wt, wo) * kt;
		Li = info._scene_ptr->_tracer_ptr->Trace(&tracing, info._trace_depth + 1, true, entering);
		lo = f * Li / pdf_w;
		inv_path_pdf = 1.0f / kt;
	}

	if (entering == true)
		lo *= (distance_tau / distance_pdf);

	return lo * inv_path_pdf;
}