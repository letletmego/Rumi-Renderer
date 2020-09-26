#include "texture.h"
#include "..\\Kernel\\intersection.h"
#include "..\\Shape\\shape.h"

Texture::~Texture(void)
{
}

Texture::Texture(void)
{
}

Color Texture::Pattern(Shape *object, const Point3 &position) const
{
	if (object == 0x00)
		return Color(1.0f);

	float u = 0;
	float v = 0;
	
	object->TextureMapping(position, &u, &v);

	return Pixel(u, v);
}