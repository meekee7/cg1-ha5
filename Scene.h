#pragma once
#include "Hitresult.h"
#include "Ray.h"
#include "Mesh.h"
#include "Light.h"
#define IDENTITY4 mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
#define BACKGROUND vec3(0.6f, 0.6f, 0.6f)
class Scene
{
public:
	Scene();
	~Scene();
	void loadscenedata();
	Hitresult* intersectscene(Ray* ray);
	Hitresult* hitany(Ray* ray, float distance);
	void renderscenegl();
	Light** lights;
	int numlights;
	int intercounter;
	bool showshadow;
private:
	Mesh** sceneobjects;
	int objects;
	void makelights();
};

