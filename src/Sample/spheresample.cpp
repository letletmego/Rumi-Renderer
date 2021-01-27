#include "spheresample.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"


SphereSample::~SphereSample(void)
{
}

SphereSample::SphereSample(void)
	: Sampler()
{
}

SphereSample::SphereSample(const int sample_count)
	: Sampler()
{
	if (sample_count > 0)
	{
		_sample_count = sample_count;
		_sample = new Point2 [_sample_count];
	}
}

void SphereSample::GenerateSamples(void)
{
	if (_sample_count == 1)
	{
		// Phi
		(_sample + 0)->_x = RandomFloat() * 2.0f * PI;
		// Theta
		(_sample + 0)->_y = acos(1.0f - RandomFloat() * 2.0f);

		return;
	}

	float golden_ratio = (1.0f + sqrt(5.0f)) * 0.5f;

	for (int idx = 0; idx < _sample_count; ++idx)
	{
		// Phi
		(_sample + idx)->_x = PI * golden_ratio * (idx + 0.5f);
		// Theta
		(_sample + idx)->_y = acos(1.0f - 2.0f * (idx + 0.5f) / _sample_count);
	}


	return;
}