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
	objects = 2;
	sceneobjects = new Mesh*[objects];
	Mesh* triangle = new Mesh();
	triangle->loadOff("scenedata/drei.off");
	triangle->setRenderMode(Mesh::GOURAUD_RENDERER);
	sceneobjects[0] = triangle;
	Mesh* cup = new Mesh();
	cup->loadOff("scenedata/tasse.off");
	cup->setRenderMode(Mesh::GOURAUD_RENDERER);
	sceneobjects[1] = cup;
}

void Scene::renderscenegl(){
	glEnable(GL_LIGHT0);
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Light::getlight().ambientlight);
	/*glMaterialfv(GL_FRONT, GL_AMBIENT, Light::getlight().ambientmat);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Light::getlight().specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, Light::getlight().shininess);
	//glLightfv(GL_LIGHT0, GL_POSITION, Light::getlight().position);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, Light::getlight().ambientlight);
	//glLightfv(GL_LIGHT0, GL_SHININESS, Light::getlight().shininess);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, Light::getlight().specular);
	float lightpos[4];
	float specular[4];
	float shini[1];
	float shinimat[1];
	float specularmat[4];
	float ambientmat[4];
	glGetLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glGetLightfv(GL_LIGHT0, GL_SHININESS, shini);
	glGetLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glGetMaterialfv(GL_FRONT, GL_AMBIENT, ambientmat);
	glGetMaterialfv(GL_FRONT, GL_SPECULAR, specularmat);
	glGetMaterialfv(GL_FRONT, GL_SHININESS, shinimat);

	cout << "Pos " << lightpos[0] << " " << lightpos[1] << " " << lightpos[2] << " " << lightpos[3] << "\n";
	cout << "Spec " << specular[0] << " " << specular[1] << " " << specular[2] << " " << specular[3] << "\n";
	cout << "Shin " << shini[0] << "\n";
	cout << "AmbMat " << ambientmat[0] << " " << ambientmat[1] << " " << ambientmat[2] << " " << ambientmat[3] << "\n";
	cout << "SpecMat " << specularmat[0] << " " << specularmat[1] << " " << specularmat[2] << " " << specularmat[3] << "\n";
	cout << "ShinMat " << shinimat[0] << "\n";*/
	for (int i = 0; i < objects; i++)
		sceneobjects[i]->render();
	glDisable(GL_LIGHT0);
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