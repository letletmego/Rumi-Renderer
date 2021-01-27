#include "rectsample.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"


RectSample::~RectSample(void)
{
}

RectSample::RectSample(void)
	: Sampler()
	, _depth(0)
	, _x_length(0)
	, _y_length(0)
{
}

RectSample::RectSample(const int sample_count, const float x_length, const float y_length)
	: Sampler()
	, _depth(sample_count > 0 ? (int)sqrt((float)sample_count) : 0)
	, _x_length(x_length)
	, _y_length(y_length)
{
}

void RectSample::GenerateSamples(void)
{
	if (_depth == 0)
		return;

	if (_sample != 0x00)
		DELETE_ARRAY(_sample);

	float ratio = 0.0f;
	if (_x_length >= _y_length)
		ratio = _y_length / _x_length;
	else
		ratio = _x_length / _y_length;

	bool sampling = false;
	float dx = 1.0f / (float)_depth;
	float dy = 1.0f / (float)_depth;
	int sample_idx = 0;
	_sample_count = 0;
	Point2 *temp_sample = new Point2 [_depth * _depth];

	for (int idx_y = 0; idx_y < _depth; ++idx_y)
	{
		for (int idx_x = 0; idx_x < _depth; ++idx_x)
		{
			sample_idx = idx_y * _depth + idx_x;

			(temp_sample + sample_idx)->_x = ((float)idx_x + RandomFloat()) * dx;
			(temp_sample + sample_idx)->_y = ((float)idx_y + RandomFloat()) * dy;

			if ((temp_sample + sample_idx)->_y < ratio)
			{
				sampling = true;
				++_sample_count;
			}
		}
			
		if (sampling == false)
			break;

		sampling = false;
	}

	if (_sample_count == 0)
		return;

	_sample = new Point2 [_sample_count];

	float inv_ratio = 1.0f / ratio;
	int idx = 0;
	int temp_idx = 0;
	for (; idx < _sample_count; ++temp_idx)
	{
		if (_x_length >= _y_length)
		{
			if ((temp_sample + temp_idx)->_y < ratio)
			{
				(_sample + idx)->_x = (temp_sample + temp_idx)->_x;
				(_sample + idx)->_y = (temp_sample + temp_idx)->_y * inv_ratio;
				++idx;
			}
		}
		else
		{
			if ((temp_sample + temp_idx)->_y < ratio)
			{
				(_sample + idx)->_x = (temp_sample + temp_idx)->_y * inv_ratio;
				(_sample + idx)->_y = (temp_sample + temp_idx)->_x;
				++idx;
			}
		}
	}

	DELETE_ARRAY(temp_sample);

	return;
}
