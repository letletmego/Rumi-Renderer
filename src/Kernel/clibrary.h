#ifndef _CLIBRARY_H
#define _CLIBRARY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <typeinfo>


#ifndef FREE
#define FREE(POINTER) \
	if (POINTER) \
	{ \
		free(POINTER); \
		POINTER = 0x00; \
	}
#endif

#ifndef DELETE
#define DELETE(POINTER) \
	if (POINTER) \
	{ \
		delete POINTER; \
		POINTER = 0x00; \
	}
#endif

#ifndef DELETE_ARRAY
#define DELETE_ARRAY(POINTER) \
	if (POINTER) \
	{ \
		delete [] POINTER; \
		POINTER = 0x00; \
	}
#endif

#ifndef PI
#define PI 3.14159265358979323846
#endif


#ifndef INV_PI
#define INV_PI 0.3183098865475128
#endif

inline float RandomFloat(void)
{
	return (float)rand() / (float)RAND_MAX;
}


inline float Round(const float value)
{
	return value - (int)value >= 0.5f ? ceil(value) : floor(value);
}


inline bool SolveQuadratic(const float &A, const float &B, const float &C, float *t0, float *t1)
{
	// B*B - 4AC
	float discriminant = B * B - 4 * A * C;

	// No intersection
	if (discriminant < 0.0f)
		return false;

	float inv_2A = 1.0f / (2.0f * A);

	// t0 and t1 have the same solution
	if (discriminant == 0.0f || discriminant == -0.0f)
	{
		*t0 = *t1 = -B * inv_2A;
		return true;
		//return false;
	}

	float sqrt_discriminant = sqrtf(discriminant);

	*t0 = (-B - sqrt_discriminant) * inv_2A;
	*t1 = (-B + sqrt_discriminant) * inv_2A;

	return true;
}

#endif