#ifndef _IMAGETEXTURE_H
#define _IMAGETEXTURE_H

#include "texture.h"

class ImageTexture : public Texture
{
private:
	unsigned char *_buffer;
	unsigned int _width;
	unsigned int _height;
	unsigned int _byte_per_pixel;
	float _gamma;

public:
	~ImageTexture(void);
	ImageTexture(void);
	ImageTexture(char *file_name, float gamma = 1.0f);

private:
	Color Pixel(const float &u, const float &v) const;
};

#endif