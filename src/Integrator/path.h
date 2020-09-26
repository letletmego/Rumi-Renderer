#ifndef _PATH_H
#define _PATH_H

#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"


class Vertex
{
public:
	Point3 _p;
	Vector3 _n;
	float _costheta_i;
	float _costheta_o;
	float _pdf_path;
	Color _pdf_wi;
	Color _pdf_wo;
	Color _pdf_pbackward;
	Color _pdf_pforward;
	bool _isdirac;
	bool _onsurface;

public:
	~Vertex(void);
	Vertex(void);
	void Clean(void);
};


class Path
{
public:
	Vertex _start;
	Vertex _end;
	Ray _tracing;
	Intersection _info;

public:
	~Path(void);
	Path(void);
	void Clean(void);
};

#endif