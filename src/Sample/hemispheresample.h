#ifndef _HEMISPHERESAMPLE_H
#define _HEMISPHERESAMPLE_H

#include "sampler.h"

class HemisphereSample : public Sampler
{
private:
	// Cosine power distribution
	int _e;

public:
	~HemisphereSample(void);
	HemisphereSample(void);
	HemisphereSample(const int sample_count, const int e);

private:
	virtual void GenerateSamples(void);
};

#endif