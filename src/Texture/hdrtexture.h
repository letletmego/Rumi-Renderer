#ifndef _HDRTEXTURE_H
#define _HDRTEXTURE_H

#include "texture.h"

class HDRTexture : public Texture
{
private:
	float *_buffer;
	int _width;
	int _height;
	float _gamma;

public:
	~HDRTexture(void);
	HDRTexture(void);
	HDRTexture(char *file_name);

private:
	Color Pixel(const float &u, const float &v) const;

public:
	void OutputBmp(void) const;
};

#endif