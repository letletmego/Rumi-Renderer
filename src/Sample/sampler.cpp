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

	int idx_1 = (rand() % (_sample_count - _sampling_idx)) + _sampling_idx;
	int idx_2 = (rand() % (_sample_count - _sampling_idx)) + _sampling_idx;
	float sampling_x = (_sample + idx_1)->_x;
	float sampling_y = (_sample + idx_2)->_y;

	(_sample + idx_1)->_x = (_sample + _sampling_idx)->_x;
	(_sample + idx_2)->_y = (_sample + _sampling_idx)->_y;
	(_sample + _sampling_idx)->_x = sampling_x;
	(_sample + _sampling_idx)->_y = sampling_y;

	++_sampling_idx;

	return Point2(sampling_x, sampling_y);
}

void Sampler::ShuffleSamples(void)
{
	int swap_idx = 0;
	float temp;

	for (int idx = 0; idx < _sample_count; ++idx)
	{
		swap_idx = rand() % _sample_count;

		temp = (_sample + idx)->_x;
		(_sample + idx)->_x = (_sample + swap_idx)->_x;
		(_sample + swap_idx)->_x = temp;
	}

	for (int idx = 0; idx < _sample_count; ++idx)
	{
		swap_idx = rand() % _sample_count;

		temp = (_sample + idx)->_y;
		(_sample + idx)->_y = (_sample + swap_idx)->_y;
		(_sample + swap_idx)->_y = temp;
	}

	return;
}

unsigned int Sampler::SampleCount(void) const
{
	return _sample_count;
}