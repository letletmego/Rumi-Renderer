#include "hdrtexture.h"
#include "..\\Kernel\\clibrary.h"
#include "..\\Kernel\\hdr.h"
#include "..\\Kernel\\bmp.h"
#include "..\\Kernel\\Color.h"


HDRTexture::~HDRTexture(void)
{
	FREE(_buffer)
}

HDRTexture::HDRTexture(void)
	: _buffer(0x00)
	, _width(0)
	, _height(0)
	, _gamma(1.0f / 2.2f)
{
}

HDRTexture::HDRTexture(char *file_name)
	: _buffer(0x00)
	, _width(0)
	, _height(0)
	, _gamma(1.0f / 2.2f)
{
	LoadHDR(file_name, &_buffer, &_width, &_height);
}

Color HDRTexture::Pixel(const float &u, const float &v) const
{
	int row_idx = (1.0f - v) * (_height - 1);
	int col_idx = (1.0f - u) * (_width - 1);
	int hdr_idx = row_idx * _width * 3 + col_idx * 3;

	Color pixel;
	pixel._b = *(_buffer + hdr_idx + 2);
	pixel._g = *(_buffer + hdr_idx + 1);
	pixel._r = *(_buffer + hdr_idx + 0);

	return pixel;
}

void HDRTexture::OutputBmp(void) const
{
	unsigned char *buffer = (unsigned char *)malloc(_width * _height * 3);
	
	Color bgr;
	int hdr_idx;
	int bmp_idx;

	for (int h = 0; h < _height; ++h)
	{
		hdr_idx = _width * 3 * h;
		bmp_idx = ((_height - 1 - h) * _width * 3);

		for (int w = 0; w < _width; ++w)
		{
			bgr._b = powf(*(_buffer + hdr_idx + 2), _gamma) * 255.0f;
			bgr._g = powf(*(_buffer + hdr_idx + 1), _gamma) * 255.0f;
			bgr._r = powf(*(_buffer + hdr_idx + 0), _gamma) * 255.0f;

			*(buffer + bmp_idx + 0) = __min((int)bgr._b, 255);
			*(buffer + bmp_idx + 1) = __min((int)bgr._g, 255);
			*(buffer + bmp_idx + 2) = __min((int)bgr._r, 255);

			++++++hdr_idx;
			++++++bmp_idx;
		}
	}

	SaveBmp(buffer, "HDR.bmp", _width, _height, 3);

	free(buffer);

	return;
}