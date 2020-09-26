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
	for (int idx = 0; idx < _sample_count; ++idx)
	{
		// Phi
		(_sample + idx)->_x = RandomFloat() * 2.0f * PI;
		// Theta
		(_sample + idx)->_y = acos(RandomFloat() * 2.0f - 1.0f);
	}

	return;
}