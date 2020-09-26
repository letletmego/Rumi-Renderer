#include "checker.h"


Checker::~Checker(void)
{
}

Checker::Checker(void)
	: _w(0.0f)
	, _h(0.0f)
	, _c1()
	, _c2()
{
}

Checker::Checker(const float _width, const float _height, const Color color_1, const Color color_2)
	: _w(_width)
	, _h(_height)
	, _c1(color_1)
	, _c2(color_2)
{
}

Color Checker::Pixel(const float &u, const float &v) const
{
	if ((int)(u * _w) % 2 == 0 && (int)(v * _h) % 2 == 0)
		return _c2;
	if ((int)(u * _w) % 2 != 0 && (int)(v * _h) % 2 != 0)
		return _c2;

	return _c1;
}