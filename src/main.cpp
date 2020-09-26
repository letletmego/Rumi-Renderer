#include "Kernel\\clibrary.h"
#include "Scene\\scene.h"


int main(int argc, char **argv)
{
	Scene the_scene;
	the_scene.Build();

	printf("Rendering Start!\n");

	float time_lapse = 0.0f;
	the_scene.Rendering(&time_lapse);

	printf("Rendering Finish!\n\n");
	printf("Time Lapse: %.3fs\n\n", time_lapse);
	system("pause");

	return 0;
}