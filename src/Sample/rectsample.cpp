#include "rectsample.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"


RectSample::~RectSample(void)
{
}

RectSample::RectSample(void)
	: Sampler()
	, _xdepth(0)
	, _ydepth(0)
{
}

RectSample::RectSample(const int sample_count, const float x_length, const float y_length)
	: Sampler()
	, _xdepth(0)
	, _ydepth(0)
{
	int depth = sample_count > 0 ? (int)sqrt((float)sample_count) : 0;

	float sampling_unit = 0.0f;
	float difference = 0.0f;

	if (x_length > y_length)
	{
		sampling_unit = y_length / (float)depth;
		difference = x_length - y_length;
	}
	else
	{
		sampling_unit = x_length / (float)depth;
		difference = y_length - x_length;
	}

	if (difference > sampling_unit)
	{
		float add_depth = difference / sampling_unit;

		if (x_length > y_length)
		{
			_xdepth = depth + (int)add_depth;
			_ydepth = depth;
		}
		else
		{
			_xdepth = depth;
			_ydepth = depth + (int)add_depth;
		}

		_sample_count = _xdepth * _ydepth;
	}
	else
	{
		_xdepth = depth;
		_ydepth = depth;
		_sample_count = sample_count;
	}

	if (_sample_count > 0)
		_sample = new Point2 [_sample_count];
}

void RectSample::GenerateSamples(void)
{
	if (_sample_count == 1)
	{
		_sample->_x = 0.5f;
		_sample->_y = 0.5f;
		return;
	}

	int sample_idx = 0;
	// Map into 0 ~ 1
	float dx = 1.0f / _xdepth;
	float dy = 1.0f / _ydepth;

	for (int idx_y = 0; idx_y < _ydepth; ++idx_y)
	{
		for (int idx_x = 0; idx_x < _xdepth; ++idx_x)
		{
			sample_idx = idx_y * _xdepth + idx_x;

			(_sample + sample_idx)->_x = ((float)idx_x + RandomFloat()) * dx;
			(_sample + sample_idx)->_y = ((float)idx_y + RandomFloat()) * dy;
		}
	}

	return;
}
