#ifndef _FILM_H
#define _FILM_H

#include "..\\Kernel\\color.h"

class Sampler;

class Film
{
private:
	// Samples of pixel
	Sampler *_pixel_sampler;
	// Output of rendering
	unsigned char *_buffer;
	// buffer of light tracing
	float *_light_buffer;
	// Width of film
	unsigned int _width;
	// Height of film
	unsigned int _height;
	// Bytes count of a pixel
	unsigned int _byte_per_pixel;
	// Depth of pixel
	unsigned int _sampling_depth;
	// Aspect ratio of film
	float _aspect_ratio;
	// Gamma
	float _gamma;

public:
	~Film(void);
	Film(void);
	Film(const unsigned int width, const unsigned int height, const unsigned int byte_per_pixel, const unsigned int n_samples, float gamma = 1.0f);
	void AddLightBuffer(void);

public:
	Sampler *PixelSampler(void) const;
	float AspectRatio(void) const;
	int Width(void) const;
	int Height(void) const;
	void Pixel(const int &width_idx, const int &height_idx, const Color& rgb);
	void LightPixel(const int &width_idx, const int &height_idx, const Color& rgb);
	void CombineBuffer(void);
	void Save(const float time_lapse);
};

#endif