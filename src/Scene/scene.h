#ifndef _SCENE_H
#define _SCENE_H

class Point3;
class Vector3;
class Ray;
class Film;
class Pinhole;
class Shape;
class Light;
class HomogeneousVolume;
class BVHAccel;
class Integrator;
class Intersection;

class Scene
{
private:
	int _n_objects;
	int _n_lights;

public:
	// Output image
	Film *_image;
	// Camera
	Pinhole *_camera;
	// Geometry object
	Shape **_objects;
	// Ambient light
	Light *_ambient;
	// light source
	Light **_lights;
	// Volume
	HomogeneousVolume *_volume_ptr;
	// Acceleration
	BVHAccel *_bvh_ptr;
	// Integrator
	Integrator *_tracer_ptr;

public:
	~Scene(void);
	Scene(void);

public:
	void LightsUniformSampling(Light **light_ptr, float *lights_sampling_pdf) const;
	bool VisibilityTest(const Point3 &position, const Vector3 &normal, Light **light_ptr, float *lights_sampling_pdf, Vector3 *wi, Ray *shadow_ray) const;

	int ObjectCount(void) const;
	int LightCount(void) const;

	void Build(void);
	void Rendering(float *time_lapse);
};

void FirefliesFiltering(char *_file_name);

#endif