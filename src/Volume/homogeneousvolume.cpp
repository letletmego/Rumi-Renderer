#include "homogeneousvolume.h"
#include "..\\Kernel\\phase.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Light\\light.h"
#include "..\\Acceleration\\bvh.h"
#include "..\\Integrator\\integrator.h"
#include "..\\Integrator\\path.h"
#include "..\\Scene\\scene.h"


HomogeneousVolume::~HomogeneousVolume(void)
{
	DELETE(_phase)
}

HomogeneousVolume::HomogeneousVolume(void)
	: _phase(0x00)
	, _extents()
	, _sigma_a(0.0f)
	, _sigma_s(0.0f)
	, _sigma_t(0.0f)
	, _avg_sigma_t(0.0f)
{
}

HomogeneousVolume::HomogeneousVolume(BBox extents, Color sigma_s, Color sigma_a)
	: _phase(0x00)
	, _extents(extents)
	, _sigma_s(sigma_s)
	, _sigma_a(sigma_a)
	, _sigma_t(0.0f)
	, _avg_sigma_t(0.0f)
{
	_phase = new Isotropic();
	_sigma_t = sigma_a + sigma_s;
	_avg_sigma_t = Average(_sigma_t);
	_absorb = Average(sigma_a / _sigma_t);
}

Color HomogeneousVolume::Sigma_t(void)
{
	return _sigma_t;
}

Color HomogeneousVolume::Tr(const Ray &ray, const float &tnear, const float &tfar)
{
	Color tr;
	tr._b = exp(_sigma_t._b * -(tfar - tnear));
	tr._g = exp(_sigma_t._g * -(tfar - tnear));
	tr._r = exp(_sigma_t._r * -(tfar - tnear));

	return tr;
}

Color HomogeneousVolume::ShadowRayTr(const Ray &ray)
{
	Color tau;
	Color pdf_s;
	float smax = 0.0f;
	float xi = 1.0f;
	float tnear = 0.0f;
	float tfar = 0.0f;

	if (Intersect(ray, &tnear, &tfar) == false)
		return Color(1.0f);

	smax = MaxDistance(ray._tmax, tnear, tfar);
	xi = RandomFloat();
	// Scattering
	if (xi < 1.0f - exp(_avg_sigma_t * -(smax)))
		return Color(0.0f);

	// direct lighting
	if (tnear > 0.0f)
	{
		if (ray._tmax > tfar)
			tau = Tr(ray, tnear, tfar);
		else
			tau = Tr(ray, tnear, ray._tmax);
	}
	else
	{
		if (ray._tmax > tfar)
			tau = Tr(ray, 0.0f, tfar);
		else
			tau = Tr(ray, 0.0f, ray._tmax);
	}

	pdf_s = tau;

	return tau / pdf_s;
}

float HomogeneousVolume::Pdfw(const Point3 &position, const Vector3 &w, const Vector3 &w_pdf)
{
	return _phase->Pdfw(position, w, w_pdf);
}

void HomogeneousVolume::PdfDistance(const float &distance, Color *tau, Color *pdf)
{
	tau->_b = exp(_sigma_t._b * -(distance - 0.0f));
	tau->_g = exp(_sigma_t._g * -(distance - 0.0f));
	tau->_r = exp(_sigma_t._r * -(distance - 0.0f));

	*pdf = *tau * _sigma_t;
	if (*pdf == 0.0f)
		*pdf = 1e-4f;

	return;
}

bool HomogeneousVolume::Intersect(const Ray &ray, float *tnear, float *tfar) const
{
	*tnear = 0.0f;
	*tfar = 0.0f;

	if (_extents.Intersect(ray, tnear, tfar) == false)
		return false;

	// NaN
	if (*tnear != *tnear || *tfar != *tfar)
		return false;

	// The object block the volume
	if (0.0f < ray._tmin && ray._tmin < *tnear)
		return false;

	if (*tfar < 0.0f)
		return false;

	return true;
}

float HomogeneousVolume::MaxDistance(const float &thit, const float &tnear, const float &tfar)
{
	float max_distance = 0.0f;

	// Ray is start In the volume
	if (tnear < 0.0)
	{
		if (thit == 0.0f)
			max_distance = tfar;
		else
		{
			if (thit < tfar)
				max_distance = thit;
			else
				max_distance = tfar;
		}
	}
	else
	{
		if (thit == 0.0f)
			max_distance = tfar - tnear;
		else
		{
			if (thit < tfar)
				max_distance = thit - tnear;
			else
				max_distance = tfar - tnear;
		}
	}

	return max_distance;
}

bool HomogeneousVolume::DistanceSampling(const float &max_distance, float *s, bool *absorb, Color *tau, Color *pdf)
{
	float xi = RandomFloat();

	// Over the sampling distance
	if (xi >= 1.0f - exp(_avg_sigma_t * -(max_distance)))
	{
		tau->_b = exp(_sigma_t._b * -(max_distance - 0.0f));
		tau->_g = exp(_sigma_t._g * -(max_distance - 0.0f));
		tau->_r = exp(_sigma_t._r * -(max_distance - 0.0f));
		if (*tau == 0.0f)
			*tau = 1e-4f;
		*pdf = *tau;

		return false;
	}

	// Distance sampling
	float ds = -log(1.0f - xi) / _avg_sigma_t;
	if (max_distance - ds < 1e-4f)
	{
		tau->_b = exp(_sigma_t._b * -(max_distance - 0.0f));
		tau->_g = exp(_sigma_t._g * -(max_distance - 0.0f));
		tau->_r = exp(_sigma_t._r * -(max_distance - 0.0f));
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

Vector3 HomogeneousVolume::PhaseSampling(Intersection *info, bool *Le, bool *in_object, float *pdf_w)
{
	*Le = false;
	//*Le = true;
	*in_object = false;

	return _phase->PhaseSampling(info->_ray_ptr->_d, pdf_w);
}

Color HomogeneousVolume::Fp(const Point3 &position, const Vector3 &wi, const Vector3 &wo)
{
	return  _phase->Fp(position, wi, wo) * _sigma_s; // radiance * sigma_s * phase / pdf_phase
}

Color HomogeneousVolume::DirectLighting(const Intersection &info)
{
	float lights_sampling_pdf;
	Light *light_ptr;
	Point3 p(info._vertex_ptr->_p);
	Vector3 n(info._vertex_ptr->_n);
	Vector3 wi;
	Vector3 wo(-info._ray_ptr->_d);
	Ray shadow_ray;
	
	if (info._scene_ptr->VisibilityTest(p, n, &light_ptr, &lights_sampling_pdf, &wi, &shadow_ray) == false)
		return Color(0.0f);

	Color Li = light_ptr->L(info);
	Color tau = ShadowRayTr(shadow_ray);
	float fp = 1.0f / (4.0f * PI);
	float g = light_ptr->G(p);

	return Li * tau * _sigma_s * fp * g / lights_sampling_pdf;
}

Color HomogeneousVolume::GlobalIllumination(const Intersection &info, const float &s,  const float &smax)
{
	Color direct_lo = DirectLighting(info);

	float pdf_w = 1.0f;
	Vector3 wi(_phase->PhaseSampling(-info._ray_ptr->_d, &pdf_w));
	Point3 p(info._vertex_ptr->_p);
	Ray in_scattering(p, wi);
	// radiance * phase / pdf_phase
	Color indirect_lo = info._scene_ptr->_tracer_ptr->Trace(&in_scattering, info._trace_depth + 1, false) * _sigma_s;

	return (direct_lo + indirect_lo);
}

