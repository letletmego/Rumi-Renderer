#ifndef _COLOR_H
#define _COLOR_H

class Color
{
public:
	float _b;
	float _g;
	float _r;

public:
	~Color(void);
	Color(void);
	Color(const float &value);
	Color(const float &red, const float &green, const float &blue);

public:
	Color operator + (const Color &bgr) const;
	Color operator - (const Color &bgr) const;
	Color operator * (const Color &bgr) const;
	Color operator * (const float &scalar) const;
	Color operator / (const Color &bgr) const;
	Color operator / (const float &scalar) const;
	void operator += (const Color &bgr);
	void operator *= (const Color &bgr);
	void operator *= (const float &scalar);
	bool operator == (const Color &bgr) const;
	bool operator != (const Color &bgr) const;
	bool IsNaN(void) const;
	bool IsInf(void) const;
};

inline Color operator * (const float &scalar, const Color &bgr)
{
	return Color(scalar * bgr._b, scalar * bgr._g, scalar * bgr._r);
}

inline float Average(const Color &bgr)
{
	return (bgr._b + bgr._g + bgr._r) * 0.33333333333f;
}

#endif