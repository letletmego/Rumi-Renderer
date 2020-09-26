#ifndef _UNIFORMSAMPLE_H
#define _UNIFORMSAMPLE_H

#include "sampler.h"

class UniformSample : public Sampler
{
private:
	bool _jitter;
	int _depth;

public:
	~UniformSample(void);
	UniformSample(void);
	UniformSample(const int sample_count, const bool jitter);

private:
	virtual void GenerateSamples(void);
};

#endif