#include "Scene.h"
using namespace std;

Scene::Scene()
{
}


Scene::~Scene()
{
	if (sceneobjects != nullptr){
		for (int i = 0; i < objects; i++)
			delete sceneobjects[i];
		delete sceneobjects;
	}
}

void Scene::loadscenedata(){
	objects = 1;
	sceneobjects = new Mesh*[objects];
	Mesh* triangle = new Mesh();
	triangle->loadOff("scenedata/drei.off");
	triangle->setRenderMode(Mesh::GOURAUD_RENDERER);
	sceneobjects[0] = triangle;

}

void Scene::renderscenegl(){
	for (int i = 0; i < objects; i++)
		sceneobjects[i]->render();
}

Hitresult* Scene::intersectscene(Ray* ray){
	Hitresult* result = nullptr;
	for (int i = 0; i < objects; i++){
		Hitresult* hit = sceneobjects[i]->intersectModel(ray);
		if (hit != nullptr){
			if (result == nullptr)
				result = hit;
			else if (hit->distance < result->distance){
				delete result;
				result = hit;
			}
		}
	}
	return result;
}