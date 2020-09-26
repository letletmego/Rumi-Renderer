#include "bsdf.h"
#include "vector.h"


BSDF::~BSDF(void)
{
}

BSDF::BSDF(void)
{
}


Lambertian::Lambertian(void)
	: _cd()
{
}

Lambertian::Lambertian(const Color cd)
	: _cd(cd)
{
}

Color Lambertian::Rho(const Vector3 &wi, const Vector3 &wo) const
{
	return _cd;
}

Color Lambertian::F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const
{
	return _cd * INV_PI;
}

float Lambertian::Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	float pdf = INV_PI * Dot(normal, w_pdf);
	if (pdf <= 0.0f)
		pdf = 1e-4f;

	return pdf;
}

Vector3 Lambertian::BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf)
{
	Vector3 v(normal);
	Vector3 u;
	Vector3 w;
	OrthonormalBasis(v, &u, &w);

	float phi = RandomFloat() * 2.0f * PI;
	//float theta = acos(sqrt(1.0f - RandomFloat()));
	float theta = acos(1.0f - RandomFloat() * 2.0f) * 0.5f;
	float sintheta = sin(theta);
	float sampling_u = sintheta * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sintheta * cos(phi);

	Vector3 wi(u * sampling_u + v * sampling_v + w * sampling_w);

	*costheta = Dot(wi, normal);
	if (*costheta <= 0.0f)
		*costheta = 1e-4f;

	*pdf = INV_PI * *costheta;
	if (*pdf <= 0.0f)
		*pdf = 1e-4f;

	return wi;
}


Phong::Phong(void)
	: _e(0.0f)
	, _normalize(0.0f)
	, _cs()
{
}

Phong::Phong(const float e, const Color cs)
	: _e(e)
	, _normalize((e + 1.0f) / (2.0f * PI))
	, _cs(cs)
{
}

Color Phong::Rho(const Vector3 &wi, const Vector3 &wo) const
{
	return Color();
}

Color Phong::F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const
{
	float costheta = Dot(wi, normal);
	if (costheta <= 0.0f)
		costheta = 1e-4f;

	Vector3 r(-wi + normal * (costheta * 2.0f));

	float cosalpha = pow(Dot(wo, r), _e);
	if (cosalpha >= 1e-4f)
		return Color(1.0f, 1.0f, 1.0f) * _normalize * cosalpha;
	else
		return Color(1.0f, 1.0f, 1.0f) * _normalize * 1e-4f;
}

float Phong::Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	float won = Dot(w, normal);
	if (won <= 0.0f)
		won = 1e-4f;

	Vector3 v(-w + normal * (won * 2.0f));

	float pdf = 1.0f;
	float cosalpha = pow(Dot(w_pdf, v), _e);
	if (cosalpha >= 1e-4f)
		pdf = _normalize * cosalpha;
	else
		pdf = _normalize * 1e-4f;

	if (pdf <= 0.0f)
		pdf = 1e-4f;

	return pdf;
}

Vector3 Phong::BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf)
{
	float won = Dot(wo, normal);
	if (won <= 0.0f)
		won = 1e-4f;

	Vector3 v(-wo + normal * won * 2.0f);
	Vector3 u;
	Vector3 w;
	OrthonormalBasis(v, &u, &w);

	float phi = (RandomFloat()) * 2.0f * PI;
	float theta = acos(pow(RandomFloat(), 1.0f / (1.0f + _e)));
	float sampling_u = sin(theta) * sin(phi);
	float sampling_v = cos(theta);
	float sampling_w = sin(theta) * cos(phi);

	Vector3 wi = u * sampling_u + v * sampling_v + w * sampling_w;

	*costheta = Dot(wi, normal);
	if (*costheta < 0.0f)
	{
		*costheta = -*costheta;
		wi = u * -sampling_u + v * sampling_v + w * -sampling_w;
		//*costheta = 0.0f;
		//return Vector3(0.0f);
	}

	if (*costheta == 0.0f)
		*costheta = 1e-4f;

	float cosalpha = pow(Dot(wi, v), _e);
	if (cosalpha >= 1e-4f)
		*pdf = _normalize * cosalpha;
	else
		*pdf = _normalize * 1e-4f;

	if (*pdf <= 0.0f)
		*pdf = 1e-4f;

	*costheta = 1.0f;

	return wi;
}

PerfectReflection::PerfectReflection(void)
	: _cr()
{
}

PerfectReflection::PerfectReflection(const Color cr)
	: _cr(cr)
{
}

Color PerfectReflection::Rho(const Vector3 &wi, const Vector3 &wo) const
{
	return Color();
}

Color PerfectReflection::F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const
{
	return _cr;
}

float PerfectReflection::Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	return 1.0f;
}

Vector3 PerfectReflection::BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf)
{
	*costheta = 1.0f;
	*pdf = 1.0f;

	float won = Dot(wo, normal);
	if (won <= 0.0f)
		won = 1e-4f;

	return -wo + normal * (2.0f * won);
}

FresnelReflection::FresnelReflection(void)
	: _cr()
{
}

FresnelReflection::FresnelReflection(const Color cr)
	: _cr(cr)
{
}

Color FresnelReflection::Rho(const Vector3 &wi, const Vector3 &wo) const
{
	return Color();
}

Color FresnelReflection::F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const
{
	return _cr;
}

float FresnelReflection::Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	return 1.0f;
}

Vector3 FresnelReflection::BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf)
{
	*pdf = 1.0f;
	*costheta = Dot(wo, normal);

	 Vector3 n(normal);
	 float won = *costheta;

	if (won < 1e-4f)
	{
		if (won > -1e-4f)
			won = 1e-4f;
		else
		{
			n = -n;
			won = -won;
		}
	}

	return -wo + n * (2.0f * won);
}

float FresnelReflection::Fresnel(const Vector3 &direction, const Vector3 &normal, const float &eta_in, const float &eta_out)
{
	float eta_i = eta_out;
	float eta_t = eta_in;
	float costheta_i = -Dot(direction, normal);

	if (costheta_i < 0.0f)
	{
		eta_i = eta_in;
		eta_t = eta_out;
		costheta_i = -costheta_i;
	}

	float inv_eta = 1.0f / (eta_t / eta_i);
	float costheta_t = 1.0f - (1.0f - costheta_i * costheta_i) * (inv_eta * inv_eta);

	// Total intenal reflection
	if (costheta_t <= 0.0f)
		return 1.0f;

	costheta_t = sqrt(costheta_t);

	float rs = (eta_i * costheta_i - eta_t * costheta_t) / (eta_i * costheta_i + eta_t * costheta_t);
	float rp = (eta_i * costheta_t - eta_t * costheta_i) / (eta_i * costheta_t + eta_t * costheta_i);
	float kr = (rs * rs + rp * rp) * 0.5f;

	return kr;
}


FresnelTransparency::~FresnelTransparency(void)
{
}

FresnelTransparency::FresnelTransparency(void)
	: _eta_in(1.0f)
	, _eta_out(1.0f)
	, _ct()
	, _absorption()
{
}

FresnelTransparency::FresnelTransparency(float eta_in, float eta_out, Color absorption)
	: _eta_in(eta_in)
	, _eta_out(eta_out)
	, _ct(1.0f, 1.0f, 1.0f)
	, _absorption(absorption)
{
}

Color FresnelTransparency::Rho(const Vector3 &wi, const Vector3 &wo) const
{
	return Color();
}

Color FresnelTransparency::F(const Vector3 &normal, const Vector3 &wi, const Vector3 &wo) const
{
	return _ct / (_eta * _eta);
}

float FresnelTransparency::Pdfw(const Vector3 &normal, const Vector3 &w, const Vector3 &w_pdf) const
{
	return 1.0f;
}

Vector3 FresnelTransparency::BSDFSampling(const Vector3 &wo, const Vector3 &normal, float *costheta, float *pdf)
{
	*pdf = 1.0f;
	_eta = _eta_in / _eta_out;

	Vector3 n = normal;

	float costheta_i = *costheta = Dot(wo, n);
	// in object
	if (costheta_i < 0.0f)
	{
		n = -n;
		_eta = _eta_out / _eta_in;
		costheta_i = -costheta_i;
	}

	float inv_eta = 1.0f / _eta;
	float costheta_t = sqrt(1.0f - (1.0f - costheta_i * costheta_i) * (inv_eta * inv_eta));
	
	//wt
	return -wo * inv_eta + n * (costheta_i * inv_eta - costheta_t);
}