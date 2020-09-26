#include "imagetexture.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\bmp.h"
#include "..\\Kernel\\Color.h"


ImageTexture::~ImageTexture(void)
{
	FREE(_buffer);
}

ImageTexture::ImageTexture(void)
	: _buffer(0x00)
	, _width(0)
	, _height(0)
	, _byte_per_pixel(0)
	, _gamma(1.0f)
{
}

ImageTexture::ImageTexture(char *file_name, float gamma)
	: _buffer(0x00)
	, _width(0)
	, _height(0)
	, _byte_per_pixel(0)
	, _gamma(gamma)

{
	_buffer = LoadBmp(file_name, &_width, &_height, &_byte_per_pixel);
}

Color ImageTexture::Pixel(const float &u, const float &v) const
{
	if (_buffer == 0x00)
		return Color(1.0f);

	int row_idx = (1.0f - v) * (_height - 1);
	int col_idx = u * (_width - 1);

	int pixel_row = (_height - 1 - row_idx) * _width * _byte_per_pixel;
	int pixel_col = col_idx * _byte_per_pixel;
	int pixel_idx = pixel_row + pixel_col;

	float inv_rgb = 1.0f / 255.0f;
	Color pixel;

	if (_gamma == 1.0f)
	{
		pixel._b = (float)*(_buffer + pixel_idx)   * inv_rgb;
		pixel._g = (float)*(_buffer + ++pixel_idx) * inv_rgb;
		pixel._r = (float)*(_buffer + ++pixel_idx) * inv_rgb;
	}
	else
	{
		pixel._b = powf((float)*(_buffer + pixel_idx) * inv_rgb, _gamma);
		pixel._g = powf((float)*(_buffer + ++pixel_idx) * inv_rgb, _gamma);
		pixel._r = powf((float)*(_buffer + ++pixel_idx) * inv_rgb, _gamma);
	}

	return pixel;
}
