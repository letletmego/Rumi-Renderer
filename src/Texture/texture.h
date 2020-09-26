#ifndef _TEXTURE_H
#define _TEXTURE_H

class Point3;
class Color;
class Shape;

class Texture
{
public:
	virtual ~Texture(void);
	Texture(void);

public:
	Color Pattern(Shape *object, const Point3 &position) const;
	virtual Color Pixel(const float &u, const float &v) const = 0;
};

#endif