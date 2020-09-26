#include "path.h"


Vertex::~Vertex(void)
{
}

Vertex::Vertex(void)
	: _p()
	, _n()
	, _costheta_i(1.0f)
	, _costheta_o(1.0f)
	, _pdf_path(1.0f)
	, _pdf_wi(1.0f)
	, _pdf_wo(1.0f)
	, _pdf_pbackward(1.0f)
	, _pdf_pforward(1.0f)
	, _isdirac(false)
	, _onsurface(true)
{
}

void Vertex::Clean(void)
{
	_p = 0.0f;
	_n = 0.0f;
	_costheta_i = 1.0f;
	_costheta_o = 1.0f;
	_pdf_path = 1.0f;
	_pdf_wi = 1.0f;
	_pdf_wo = 1.0f;
	_pdf_pbackward = 1.0f;
	_pdf_pforward = 1.0f;
	_isdirac = false;
	_onsurface = true;

	return;
}


Path::~Path(void)
{
	_info._ray_ptr = 0x00;
	_info._vertex_ptr = 0x00;
}

Path::Path(void)
	: _start()
	, _end()
	, _tracing()
	, _info()
{
	_info._vertex_ptr = &_end;
	_info._ray_ptr = &_tracing;
}

void Path::Clean(void)
{
	_start.Clean();
	_end.Clean();
	_tracing.Return();
	_info.Clean();

	return;
}