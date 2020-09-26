#ifndef _BDPT_H
#define _BDPT_H

#include "integrator.h"
#include "path.h"


class BidirectionalPathTracer : public Integrator
{
private:
	// Number of s_path
	int _nL;
	// Number of t_path
	int _nE;
	// Path of light
	Path *_s_path;
	// Path of view
	Path *_t_path;
	// Compute contribution of light path
	Color *_alphaL;
	// Compute contribution of view apth
	Color *_alphaE;
	// Compute light forward pdf
	Color *_s_pdfforward;
	// Compute light backward pdf
	Color *_s_pdfbackward;
	// Compute view forward pdf
	Color *_t_pdfforward;
	// Compute view backward pdf
	Color *_t_pdfbackward;

public:
	~BidirectionalPathTracer(void);
	BidirectionalPathTracer(void);
	BidirectionalPathTracer(Scene *scene_ptr, int nL, int nE);

public:
	Color Trace(const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Film *image_ptr);

private:
	void CleanPath(const int trace_depth, Path *path, Color *alpha);
	int ConstructViewPath(const int &max_depth, const unsigned int &width_idx, const unsigned int &height_idx, Pinhole *camera_ptr, Light *light_ptr);
	int ConstructLightPath(const int &max_depth, Pinhole *camera_ptr, Light *light_ptr);
	bool VisibilityTest(const int ns, const int nt, const Point3 &s_vertex, const Point3 &t_vertex);
	bool VolumetricTest(const int ns, const int nt, const Point3 &s_vertex, const Point3 &t_vertex, float *s, Color *tau);
	float MISWeight(Pinhole *camera_ptr, Light *light_ptr, const int ns, const int nt, const Vector3 &connect_wi, const Vector3 &connect_wo);
};

#endif