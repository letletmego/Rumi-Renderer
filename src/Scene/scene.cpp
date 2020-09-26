#include "scene.h"

#include "..\\Kernel\\bmp.h"
#include "..\\Kernel\\ray.h"
#include "..\\Kernel\\intersection.h"

#include "..\\Camera\\film.h"
#include "..\\Camera\\pinhole.h"

#include "..\\Texture\\checker.h"
#include "..\\Texture\\imagetexture.h"
#include "..\\Texture\\hdrtexture.h"

#include "..\\Shape\\sphere.h"
#include "..\\Shape\\rect.h"
#include "..\\Shape\\triangle.h"
#include "..\\Shape\\model.h"

#include "..\\Material\\matte.h"
#include "..\\Material\\glossy.h"
#include "..\\Material\\specular.h"
#include "..\\Material\\emissive.h"
#include "..\\Material\\dielectric.h"
#include "..\\Material\\subsurface.h"

#include "..\\Light\\ambient.h"
#include "..\\Light\\pointLight.h"
#include "..\\Light\\arealight.h"
#include "..\\Light\\imagebasedlight.h"

#include "..\\Sample\\sampler.h"

#include "..\\Acceleration\\bvh.h"

#include "..\\Integrator\\raytracer.h"
#include "..\\Integrator\\pathtracer.h"
#include "..\\Integrator\\bidirectionalpathtracer.h"

#include "..\\Volume\\homogeneousvolume.h"


Scene::~Scene(void)
{
	// Release integrator
	DELETE(_tracer_ptr);

	// Release BVH
	DELETE(_bvh_ptr);

	// Release volume
	DELETE(_volume_ptr);

	// Release light
	for (int idx = 0; idx < _n_lights; ++idx)
	{
		DELETE(*(_lights + idx))
	}

	DELETE_ARRAY(_lights)

	// Release geometry object
	for (int idx = 0; idx < _n_objects; ++idx)
	{
		DELETE(*(_objects + idx))
	}

	DELETE_ARRAY(_objects)

	// Release camera
	DELETE(_camera);

	// Release image
	DELETE(_image);
}

Scene::Scene(void)
	: _n_objects(0)
	, _n_lights(0)
	, _n_samples(0)
	, _image(0x00)
	, _camera(0x00)
	, _objects(0x00)
	, _ambient(0x00)
	, _lights(0x00)
	, _volume_ptr(0x00)
	, _bvh_ptr(0x00)
	, _tracer_ptr(0x00)
{
	srand((unsigned int)time(0x00));
}

void Scene::LightsGenerateSamples(void)
{
	for (int idx = 0; idx < _n_lights; ++idx)
	{
		(*_lights + idx)->GenerateSamples();
	}

	return;
}

void Scene::LightsUniformSampling(Light **light_ptr, float *lights_sampling_pdf) const
{
	*light_ptr = *(_lights + (rand() % _n_lights));
	*lights_sampling_pdf = (1.0f / _n_lights) * (*light_ptr)->Pdf();

	return;
}

bool Scene::VisibilityTest(const Point3 &position, const Vector3 &normal,
	Light **light_ptr, float *lights_sampling_pdf, Vector3 *wi, Ray *shadow_ray) const
{
	*light_ptr = *(_lights + (rand() % _n_lights));
	(*light_ptr)->Sampling(position, normal);
	*lights_sampling_pdf = 1.0f / _n_lights * (*light_ptr)->Pdf();

	*wi = (*light_ptr)->Wi(position);

	shadow_ray->_o = position;
	shadow_ray->_d = *wi;
	shadow_ray->_tmax = Length((*light_ptr)->P() - position);

	// Illumination
	if (Dot(shadow_ray->_d, (*light_ptr)->N(position)) >= 0.0f)
		return false;

	_bvh_ptr->Travel(shadow_ray, 0x00);

	// In shadow
	if (shadow_ray->_tmin > 0.0f)
		return false;

	return true;
}

int Scene::ObjectCount(void) const
{
	return _n_objects;
}

int Scene::LightCount(void) const
{
	return _n_lights;
}

void Scene::Build(void)
{
	_n_objects = 3;
	_objects = new Shape *[_n_objects];

	_n_lights = 1;
	_lights = new Light *[_n_lights];

	_n_samples = 1;
	_image = new Film(1200, 1000, 4, _n_samples, 1.0f / 2.2f);

	Mat4x4 camera_transform = RotationZ(0.0f) * RotationX(0.0f) * RotationY(PI);
	_camera = new Pinhole(Point3(0.0f, 0.0f, 11.2f), camera_transform, 1.0f / _image->AspectRatio());
	_camera->DepthOfField(11.6f, 0.02f);

	Mat4x4 t_transform = RotationY(0.0) * Translation(0.3f, -1.47f, -0.4f) * Scaling(1.5f, 1.5f, 1.5f);
	_objects[0] = new Model("Scene\\Model\\bunny.obj", t_transform);
	_objects[0]->AddingMaterial(new Matte(0.55f, Color(1.0f, 1.0f, 1.0f)));

	Mat4x4 cube_transform = Translation(0.0f, -1.54f, 0.0f) * Scaling(2.0f, 0.1f, 2.0f);
	_objects[1] = new Model("Scene\\Model\\cube.obj", cube_transform);
	_objects[1]->AddingMaterial(new Matte(0.2f, Color(1.0f, 1.0f, 1.0f)));

	_objects[2] = new Sphere(Point3(0.0f, 5.0f, 0.0f), 15.0f);
	_objects[2]->_transform = RotationY(-PI);
	_objects[2]->AddingMaterial(new Emissive(1.0f, Color(1.0f, 1.0f, 1.0f)));
	_objects[2]->AddingTexture(new HDRTexture("Scene\\No HDRI.hdr"));

	_lights[0] = new ImageBasedLight(_objects[2], _objects[2]->MaterialPtr(), _objects[2]->TexturePtr());

	_bvh_ptr = new BVHAccel(_objects, _n_objects);

	//_tracer_ptr = new RayTracer(this, 10);
	_tracer_ptr = new PathTracer(this, 15);

	return;
}

void Scene::Rendering(float *time_lapse)
{
	// Rendering equation Lo(p, wo)
	Color Lo;
	Color lo;

	int height = _image->Height();
	int width  = _image->Width();
	int n_samples = _image->PixelSampler()->SampleCount();
	float inv_n_sample = 1.0f / (float)n_samples;
	float aspect_ratio = _image->AspectRatio();

	//  Generate pixel sample
	_image->PixelSampler()->GenerateSamples();

	clock_t t = clock();
	float path_sampling = 0.0f;
	//  Height of image
	for (int height_idx = 0; height_idx < height; ++height_idx)
	{
		//  Width of image
		for (int width_idx = 0; width_idx < width; ++width_idx)
		{
			LightsGenerateSamples();

			//  Anti-Aliasing
			for (int sampling_idx = 0; sampling_idx < n_samples; ++sampling_idx)
			{
				lo = _tracer_ptr->Trace(width_idx, height_idx, _camera, _image);

				if (lo.IsNaN() == true)
				{
					printf("NAN!\n");
					printf("Pixel Width Index  : %d\n", width_idx);
					printf("Pixel Height Index : %d\n\n", height_idx);
				}
				else
				{
					Lo += lo;
				}
			}

			Lo *= inv_n_sample;
			_image->Pixel(width_idx, height_idx, Lo);

			Lo._r = 0.0f;
			Lo._g = 0.0f;
			Lo._b = 0.0f;
		}
	}

	_image->CombineBuffer();

	t = clock() - t;
	*time_lapse = (float)t * 1e-3f;

	_image->Save(*time_lapse);

	return;
}


void FirefliesFiltering(char *file_name)
{
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int byte_per_pixel = 0;
	unsigned char *src = 0x00;
	src = LoadBmp(file_name, &width, &height, &byte_per_pixel);

	// Filtering
	unsigned char *dst = (unsigned char *)malloc(width * height * byte_per_pixel);
	Color target;
	Color new_lo;
	Color filter[8];
	bool fireflies = false;
	int pixel_hidx;
	int pixel_widx;
	int pixel_idx;
	int n_copy;
	int copy_idx;

	for (int h_idx = 0; h_idx < height; ++h_idx)
	{
		for (int w_idx = 0; w_idx < width; ++w_idx)
		{
			n_copy = 8;
			copy_idx = 0;
			
			if (w_idx == 0 || h_idx == 0 || w_idx == width - 1 || h_idx == height - 1)
			{
				n_copy -= 3;
				if (w_idx == 0 && h_idx == 0)
					n_copy -= 2;
				if (w_idx == 0 && h_idx == height - 1)
					n_copy -= 2;
				if (w_idx == width - 1 && h_idx == 0)
					n_copy -= 2;
				if (w_idx == width - 1 && h_idx == height - 1)
					n_copy -= 2;
			}

			pixel_hidx = (height - (h_idx + 1)) * width * byte_per_pixel;
			pixel_widx = w_idx * byte_per_pixel;
			pixel_idx = pixel_hidx + pixel_widx;
			target._b = (float)*(src + pixel_idx);
			target._g = (float)*(src + ++pixel_idx);
			target._r = (float)*(src + ++pixel_idx);
			
			if (target._b < 255.0f && target._g < 255.0f && target._r < 255.0f)
			{
				pixel_idx = pixel_hidx + pixel_widx;
				*(dst + pixel_idx) = (int)target._b;
				*(dst + ++pixel_idx) = (int)target._g;
				*(dst + ++pixel_idx) = (int)target._r;
				continue;
			}

			if (w_idx != 0)
			{
				pixel_idx = pixel_hidx + pixel_widx - byte_per_pixel;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			if (w_idx != width - 1)
			{
				pixel_idx = pixel_hidx + pixel_widx + byte_per_pixel;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			if (h_idx != 0 || w_idx != 0)
			{
				pixel_idx = (pixel_hidx - width * byte_per_pixel) + pixel_widx - byte_per_pixel;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			if (h_idx != 0)
			{
				pixel_idx = (pixel_hidx - width * byte_per_pixel) + pixel_widx;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			if (h_idx != 0 || w_idx != width - 1)
			{
				pixel_idx = (pixel_hidx - width * byte_per_pixel) + pixel_widx + byte_per_pixel;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			if (h_idx != height - 1 || w_idx != 0)
			{
				pixel_idx = (pixel_hidx + width * byte_per_pixel) + pixel_widx - byte_per_pixel;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			if (h_idx != height - 1)
			{
				pixel_idx = (pixel_hidx + width * byte_per_pixel) + pixel_widx;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			if (h_idx != height - 1 || w_idx != width - 1)
			{
				pixel_idx = (pixel_hidx + width * byte_per_pixel) + pixel_widx + byte_per_pixel;
				filter[copy_idx]._b = (float)*(src + pixel_idx);
				filter[copy_idx]._g = (float)*(src + ++pixel_idx);
				filter[copy_idx]._r = (float)*(src + ++pixel_idx);
				++copy_idx;
			}

			fireflies = true;
			for (copy_idx = 0; copy_idx < n_copy; ++copy_idx)
			{
				if (filter[copy_idx]._b == 255.0f && filter[copy_idx]._g == 255.0f && filter[copy_idx]._r == 255.0f)
					fireflies = false;
				else
					continue;
			}

			if (fireflies == false)
			{
				pixel_idx = pixel_hidx + pixel_widx;
				*(dst + pixel_idx) = (int)target._b;
				*(dst + ++pixel_idx) = (int)target._g;
				*(dst + ++pixel_idx) = (int)target._r;
				continue;
			}

			new_lo = 0.0f;
			for (copy_idx = 0; copy_idx < n_copy; ++copy_idx)
			{
				new_lo += filter[copy_idx];
			}

			new_lo *= (1.0f / (float)n_copy);
			new_lo._r = new_lo._r > 255.0f ? 255.0f : new_lo._r < 0.0f ? 0.0f : new_lo._r;
			new_lo._g = new_lo._g > 255.0f ? 255.0f : new_lo._g < 0.0f ? 0.0f : new_lo._g;
			new_lo._b = new_lo._b > 255.0f ? 255.0f : new_lo._b < 0.0f ? 0.0f : new_lo._b;

			pixel_idx = pixel_hidx + pixel_widx;
			*(dst + pixel_idx) = (int)new_lo._b;
			*(dst + ++pixel_idx) = (int)new_lo._g;
			*(dst + ++pixel_idx) = (int)new_lo._r;
		}
	}

	printf("Filtering Complete.\n");

	SaveBmp(dst, "FirefliesFiltering.bmp", width, height, byte_per_pixel);
	FREE(dst);
	FREE(src);

	return;
}