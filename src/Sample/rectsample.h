#ifndef _RECTSAMPLE_H
#define _RECTSAMPLE_H

#include "sampler.h"

class RectSample : public Sampler
{
private:
	int _xdepth;
	int _ydepth;

public:
	~RectSample(void);
	RectSample(void);
	RectSample(const int sample_count, const float x_length, const float y_length);

private:
	virtual void GenerateSamples(void);
};

#endif