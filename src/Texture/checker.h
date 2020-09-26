#ifndef _CHECKER_H
#define _CHECKER_H

#include "texture.h"
#include "..\\Kernel\\color.h"

class Checker : public Texture
{
private:
	float _w;
	float _h;
	Color _c1;
	Color _c2;

public:
	~Checker(void);
	Checker(void);
	Checker(const float _width, const float _height, const Color color_1, const Color color_2);

private:
	Color Pixel(const float &u, const float &v) const;
};

#endif