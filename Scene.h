#pragma once
#include "Hitresult.h"
#include "Ray.h"
#include "Mesh.h"
class Scene
{
public:
	Scene();
	~Scene();
	void loadscenedata();
	Hitresult* intersectscene(Ray* ray);
	void renderscenegl();
private:
	Mesh** sceneobjects;
	int objects;
};

