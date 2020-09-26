#include "film.h"
#include "..\\Kernel\\bmp.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\point.h"
#include "..\\Sample\\uniformsample.h"


Film::~Film(void)
{
	FREE(_light_buffer)
	FREE(_buffer)
	DELETE(_pixel_sampler)
}

Film::Film(void)
	: _buffer(0x00)
	, _light_buffer(0x00)
	, _width(0)
	, _height(0)
	, _byte_per_pixel(0)
	, _sampling_depth(0)
	, _aspect_ratio(0.0f)
	, _pixel_sampler(0x00)
	, _gamma(1.0f)
{
}

Film::Film(const unsigned int width, const unsigned int height, const unsigned int byte_per_pixel, const unsigned int n_samples, float gamma)
    : _buffer(0x00)
	, _light_buffer(0x00)
	, _width(width)
	, _height(height)
	, _byte_per_pixel(byte_per_pixel)
	, _sampling_depth(0)
	, _aspect_ratio((float)width / (float)height)
	, _pixel_sampler(0x00)
	, _gamma(gamma)
{
	if (n_samples > 0)
		_sampling_depth = (int)sqrt((float)n_samples);

	if (n_samples > 0)
		_pixel_sampler = new UniformSample(n_samples, n_samples > 1);

	if (_width > 0 && _height > 0 && _byte_per_pixel > 0)
	{
		_buffer = (unsigned char *)malloc(sizeof(unsigned char) * _width * _height * _byte_per_pixel);
		memset(_buffer, 0, _byte_per_pixel * _width * _height);
	}
}

void Film::AddLightBuffer(void)
{
	if (_light_buffer)
		return;

	_light_buffer = (float *)malloc(sizeof(float) * _width * _height * 3);
	memset(_light_buffer, 0, sizeof(float) * 3 * _width * _height);

	return;
}

Sampler *Film::PixelSampler(void) const
{
	return _pixel_sampler;
}

float Film::AspectRatio(void) const
{
	return _aspect_ratio;
}

int Film::Width(void) const
{
	return _width;
}

int Film::Height(void) const
{
	return _height;
}

void Film::Pixel(const int &width_idx, const int &height_idx, const Color &rgb)
{
	// Index of pixel
	int pixel_h = (_height - (height_idx + 1)) * _width * _byte_per_pixel;
	int pixel_w = width_idx * _byte_per_pixel;
	int pixel_idx = pixel_h + pixel_w;

	// Average of anti-aliasing
	Color pixel(rgb);
	pixel._r = powf(pixel._r, _gamma) * 255.0f;
	pixel._g = powf(pixel._g, _gamma) * 255.0f;
	pixel._b = powf(pixel._b, _gamma) * 255.0f;

	// Color 0~255 clamp
	pixel._r = pixel._r > 255.0f ? 255.0f : pixel._r < 0.0f ? 0.0f : pixel._r;
	pixel._g = pixel._g > 255.0f ? 255.0f : pixel._g < 0.0f ? 0.0f : pixel._g;
	pixel._b = pixel._b > 255.0f ? 255.0f : pixel._b < 0.0f ? 0.0f : pixel._b;

	// Result
	unsigned char *pixel_ptr = (_buffer + pixel_idx);
	float value = (float)*pixel_ptr + pixel._b;
	if (value > 255.0f)
		*pixel_ptr = 255;
	else
		*pixel_ptr = (int)value;

	pixel_ptr = (_buffer + ++pixel_idx);
	value = (float)*pixel_ptr + pixel._g;
	if (value > 255.0f)
		*pixel_ptr = 255;
	else
		*pixel_ptr = (int)value;

	pixel_ptr = (_buffer + ++pixel_idx);
	value = (float)*pixel_ptr + pixel._r;
	if (value > 255.0f)
		*pixel_ptr = 255;
	else
		*pixel_ptr = (int)value;

	return;
}

void Film::LightPixel(const int &width_idx, const int &height_idx, const Color& rgb)
{
	if (_light_buffer == 0x00)
		return;

	int pixel_hidx = (_height - (height_idx + 1)) * _width * 3;
	int pixel_widx = width_idx * 3;
	int pixel_idx = pixel_hidx + pixel_widx;

	float *light_buffer_ptr = (_light_buffer + pixel_idx);
	float value = *light_buffer_ptr + rgb._b;
	*light_buffer_ptr = value;

	light_buffer_ptr = (_light_buffer + ++pixel_idx);
	value = *light_buffer_ptr + rgb._g;
	*light_buffer_ptr = value;

	light_buffer_ptr = (_light_buffer + ++pixel_idx);
	value = *light_buffer_ptr + rgb._r;
	*light_buffer_ptr = value;

	return;
}

void Film::CombineBuffer(void)
{
	if (_light_buffer == 0x00)
		return;

	Color light;
	float inv_n_sampling = 1.0f / (float)(_sampling_depth * _sampling_depth);
	float inv_count = 1.0f;
	int light_hidx;
	int light_widx;
	int light_idx;

	for (int h_idx = 0; h_idx < _height; ++h_idx)
	{
		for (int w_idx = 0; w_idx < _width; ++w_idx)
		{
			light_hidx = (_height - (h_idx + 1)) * _width * 3;
			light_widx = w_idx * 3;
			light_idx = light_widx + light_hidx;
			
			light._b = *(_light_buffer + light_idx);
			light._g = *(_light_buffer + ++light_idx);
			light._r = *(_light_buffer + ++light_idx);
			light *= inv_n_sampling;
			
			Pixel(w_idx, h_idx, light);
		}
	}

	return;
}

void Film::Save(const float time_lapse)
{
	char file_name[64];

	sprintf(file_name, "%.3fs.bmp", time_lapse);

	SaveBmp(_buffer, file_name, _width, _height, _byte_per_pixel);

	return;
}