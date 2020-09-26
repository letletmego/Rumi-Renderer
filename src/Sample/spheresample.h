#ifndef _SPHERESAMPLE_H
#define _SPHERESAMPLE_H

#include "sampler.h"

class SphereSample : public Sampler
{
public:
	~SphereSample(void);
	SphereSample(void);
	SphereSample(const int sample_count);

private:
	virtual void GenerateSamples(void);
};

#endif