#include "color.h"
#include "clibrary.h"



Color::~Color(void)
{
}

Color::Color(void)
	: _b(0.0f)
	, _g(0.0f)
	, _r(0.0f)
{
}

Color::Color(const float &value)
	: _b(value)
	, _g(value)
	, _r(value)
{
}

Color::Color(const float &red, const float &green, const float &blue)
	: _b(blue)
	, _g(green)
	, _r(red)
{
}

Color Color::operator + (const Color &bgr) const
{
	return Color(_r + bgr._r, _g + bgr._g, _b + bgr._b);
}

Color Color::operator - (const Color &bgr) const
{
	return Color(_r - bgr._r, _g - bgr._g, _b - bgr._b);
}

Color Color::operator * (const Color &bgr) const
{
	return Color(_r * bgr._r, _g * bgr._g, _b * bgr._b);
}

Color Color::operator * (const float &scalar) const
{
	return Color(_r * scalar, _g * scalar, _b * scalar);
}

Color Color::operator / (const Color &bgr) const
{
	return Color(_r / bgr._r, _g / bgr._g, _b / bgr._b);
}

Color Color::operator / (const float &scalar) const
{
	float inv_scalar = 1.0f / scalar;
	return Color(_r * inv_scalar, _g * inv_scalar, _b * inv_scalar);
}

void Color::operator += (const Color &bgr)
{
	_b += bgr._b;
	_g += bgr._g;
	_r += bgr._r;

	return;
}

void Color::operator *= (const Color &bgr)
{
	_b *= bgr._b;
	_g *= bgr._g;
	_r *= bgr._r;

	return;
}

void Color::operator *= (const float &scalar)
{
	_b *= scalar;
	_g *= scalar;
	_r *= scalar;

	return;
}

bool Color::operator == (const Color &bgr) const
{
	return (_b != bgr._b) ? false : (_g != bgr._g) ? false : (_r != bgr._r) ? false : true;
}

bool Color::operator != (const Color &bgr) const
{
	return (_b != bgr._b) ? true : (_g != bgr._g) ? true : (_r != bgr._r) ? true : false;
}

bool Color::IsNaN(void) const
{
	return _b != _b || _g != _g || _r != _r;
}

bool Color::IsInf(void) const
{
	return !_finite(_b) || !_finite(_g) || !_finite(_r);
}