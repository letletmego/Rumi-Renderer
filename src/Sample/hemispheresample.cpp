#include "hemispheresample.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"


HemisphereSample::~HemisphereSample(void)
{
}

HemisphereSample::HemisphereSample(void)
	: Sampler()
	, _e(0)
{
}

HemisphereSample::HemisphereSample(const int sample_count, const int e)
	: Sampler()
	, _e(e)
{
	if (sample_count > 0)
	{
		_sample_count = sample_count;
		_sample = new Point2 [sample_count];
	}
}

void HemisphereSample::GenerateSamples(void)
{
	for (int idx = 0; idx < _sample_count; ++idx)
	{
		// Phi
		_sample[idx]._x = RandomFloat() * 2.0f * PI;
		// Theta
		_sample[idx]._y = acos(pow(RandomFloat(), 1.0f / (1.0f + (float)_e)));
	}

	return;
}