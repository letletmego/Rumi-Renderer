#include "specular.h"
#include "..\\Kernel\\bsdf.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Light\\light.h"
#include "..\\Integrator\\integrator.h"
#include "..\\Integrator\\path.h"
#include "..\\Scene\\scene.h"


Specular::~Specular(void)
{
	DELETE(_specular_brdf)
}

Specular::Specular(void)
	: Material(true)
	, _kr(0.0f)
	, _specular_brdf(new PerfectReflection)
{
}

Specular::Specular(const float kr, const Color rgb)
	: Material(true)
	, _kr(kr)
	, _specular_brdf(new PerfectReflection(rgb))
{
}

bool Specular::IsDirac(const float &xi)
{
	return true;
}

float Specular::Pdfw(const Intersection &info, const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	return _specular_brdf->Pdfw(normal, w, w_pdf);
}

Vector3 Specular::BSDFSampling(const Vector3 &wo, const Vector3 &normal, 
	bool *Le, bool *go_into_object, float *xi, float *pdf_path, float *costheta, float *pdf_w) const
{
	*Le = true;
	*go_into_object = false;
	*xi = 1.0f;

	return _specular_brdf->BSDFSampling(wo, normal, costheta, pdf_w);
}

Color Specular::Fr(const Intersection &info, const Vector3 &normal, const Vector3 &wi, const Vector3 &wo)
{
	return _specular_brdf->F(normal, wi, wo) * _kr;
}

Color Specular::DirectLighting(const Intersection &info)
{
	return Color();
}

Color Specular::GlobalIllumination(const Intersection &info)
{
	float costheta = 1.0f;
	float bsdf_pdf = 1.0f;
	Point3 p = info._vertex_ptr->_p;
	Vector3 n = info._vertex_ptr->_n;
	Vector3 wo = -info._ray_ptr->_d;
	Vector3 wi = _specular_brdf->BSDFSampling(wo, n, &costheta, &bsdf_pdf);
	Ray reflection(p, wi);

	Color f = _specular_brdf->F(n, wi, wo) * _kr;
	Color Li = info._scene_ptr->_tracer_ptr->Trace(&reflection, info._trace_depth + 1, true);
	Color lo = f * Li * costheta / bsdf_pdf;

	return lo;
}