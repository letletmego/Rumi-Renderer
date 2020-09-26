#include "sampler.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"

Sampler::~Sampler(void)
{
	DELETE_ARRAY(_sample)
}

Sampler::Sampler(void)
	: _sample_count(0)
	, _sampling_idx(0)
	, _sample(0x00)
{
}

Point2 Sampler::Sampling(void)
{
	if (_sample == 0x00)
		return Point2();

	if (_sampling_idx >= _sample_count)
		_sampling_idx = 0;

	Point2 sample = *(_sample + _sampling_idx);

	++_sampling_idx;

	return sample;
}

void Sampler::ShuffleSamples(void)
{
	int swap_idx = 0;
	Point2 temp;

	for (int idx = 0; idx < _sample_count; ++idx)
	{
		swap_idx = rand() % _sample_count;

		temp = *(_sample + idx);
		*(_sample + idx) = *(_sample + swap_idx);
		*(_sample + swap_idx) = temp;
	}

	return;
}

unsigned int Sampler::SampleCount(void) const
{
	return _sample_count;
}