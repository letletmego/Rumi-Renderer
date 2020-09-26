#ifndef _SAMPLER_H
#define _SAMPLER_H

class Point2;

class Sampler
{
protected:
	unsigned int _sample_count;
	unsigned int _sampling_idx;
	Point2 *_sample;

public:
	virtual ~Sampler(void);
	Sampler(void);

public:
	virtual void GenerateSamples(void) = 0;

	Point2 Sampling(void);
	void ShuffleSamples(void);
	unsigned int SampleCount(void) const;
};

#endif