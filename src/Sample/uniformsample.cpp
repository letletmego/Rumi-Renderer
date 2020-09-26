#include "uniformsample.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"


UniformSample::~UniformSample(void)
{
}

UniformSample::UniformSample(void)
	: Sampler()
	, _jitter(false)
	, _depth(0)
{
}

UniformSample::UniformSample(const int sample_count, const bool jitter)
	: Sampler()
	, _jitter(jitter)
	, _depth(0)
{
	if (sample_count > 0)
	{
		_depth = (int)sqrt((float)sample_count);
		_sample_count = _depth * _depth;
	}

	if (_depth > 0)
		_sample = new Point2 [_depth * _depth];
}

void UniformSample::GenerateSamples(void)
{
	int sample_idx = 0;
	// Map into 0 ~ 1
	float normalize = 1.0f / _depth;

	for (int idx_y = 0; idx_y < _depth; ++idx_y)
	{
		for (int idx_x = 0; idx_x < _depth; ++idx_x)
		{
			sample_idx = idx_y * _depth + idx_x;

			if (_jitter == true)
			{
				(_sample + sample_idx)->_x = ((float)idx_x + RandomFloat()) * normalize;
				(_sample + sample_idx)->_y = ((float)idx_y + RandomFloat()) * normalize;
			}
			else
			{
				(_sample + sample_idx)->_x = ((float)idx_x + 0.5f) * normalize;
				(_sample + sample_idx)->_y = ((float)idx_y + 0.5f) * normalize;
			}
		}
	}

	return;
}