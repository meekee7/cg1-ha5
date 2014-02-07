#include "Scene.h"
using namespace std;

Scene::Scene()
{
	objects = 0;
	numlights = 0;
	showshadow = false;
}


Scene::~Scene()
{
	if (sceneobjects != nullptr){
		for (int i = 0; i < objects; i++)
			delete sceneobjects[i];
		delete sceneobjects;
	}
	if (lights != nullptr){
		for (int i = 0; i < numlights; i++)
			delete lights[i];
		delete lights;
	}
}

void Scene::makelights(){
	numlights = 2;
	lights = new Light*[numlights];
	Light* l1 = new Light();
	l1->position = vec3(0, 0, -2);
	l1->lightnum = GL_LIGHT0;
	lights[0] = l1;
	/*Light* l2 = new Light();
	l2->position = vec3(0, 0, -7);
	l2->lightnum = GL_LIGHT1;
	lights[1] = l2;*/
	Light* l3 = new Light();
	l3->position = vec3(10, 0, 2);
	l3->lightnum = GL_LIGHT2;
	lights[1] = l3;
}

void Scene::loadscenedata(){
	this->makelights();
	mat4 mat;
	objects = 8;
	sceneobjects = new Mesh*[objects];
	Mesh* cup1 = new Mesh();
	mat = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(1, 1, 2, 1));
	cup1->loadOff("scenedata/tasse.off", mat, new Material(vec3(0, 1, 0)), this);
	cup1->setRenderMode(Mesh::GOURAUD_RENDERER);
	sceneobjects[0] = cup1;
	Mesh* cup2 = new Mesh();
	mat = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(-2, 0, 1, 1));
	cup2->loadOff("scenedata/tasse.off", mat, new Material(vec3(0, 0, 1)), this);
	cup2->setRenderMode(Mesh::GOURAUD_RENDERER);
	sceneobjects[1] = cup2;
	Mesh* cup3 = new Mesh();
	mat = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));
	cup3->loadOff("scenedata/tasse.off", mat, MIRRORMAT, this);
	cup3->setRenderMode(Mesh::GOURAUD_RENDERER);
	sceneobjects[2] = cup3;
	Mesh* planeback = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(0, 0, -8, 1));
	planeback->loadOff("scenedata/planeback.off", mat, MIRRORMAT, this);
	planeback->setRenderMode(Mesh::FLAT_RENDERER);
	sceneobjects[3] = planeback;
	Mesh* planeleft = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(-8, 0, 0, 1));
	planeleft->loadOff("scenedata/planeverti.off", mat, new Material(vec3(0.8, 0.8, 0.0)), this);
	planeleft->setRenderMode(Mesh::FLAT_RENDERER);
	planeleft->invertnormals();
	sceneobjects[4] = planeleft;
	Mesh* planeright = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(8, 0, 0, 1));
	planeright->loadOff("scenedata/planeverti.off", mat, new Material(vec3(0.0, 0.8, 0.8)), this);
	planeright->setRenderMode(Mesh::FLAT_RENDERER);
	sceneobjects[5] = planeright;
	Mesh* planeup = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(0, 8, 0, 1));
	planeup->loadOff("scenedata/planehori.off", mat, MIRRORMAT, this);
	planeup->setRenderMode(Mesh::FLAT_RENDERER);
	planeup->invertnormals();
	sceneobjects[6] = planeup;
	Mesh* planedown = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(0, -8, 0, 1));
	planedown->loadOff("scenedata/planehori.off", mat, new Material(vec3(0.8, 0.0, 0.8)), this);
	planedown->setRenderMode(Mesh::FLAT_RENDERER);
	sceneobjects[7] = planedown;
}

void Scene::renderscenegl(){
	for (int i = 0; i < numlights; i++){
		glEnable(lights[i]->lightnum);
		float lightp[] = { lights[i]->position.x, lights[i]->position.y, lights[i]->position.z, 0 };
		float specv[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		float diffv[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		float ambv[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		float shini[] = { 128.0f };
		glLightfv(lights[i]->lightnum, GL_POSITION, lightp);
		glLightfv(lights[i]->lightnum, GL_AMBIENT, ambv);
		glLightfv(lights[i]->lightnum, GL_DIFFUSE, diffv);
		glLightfv(lights[i]->lightnum, GL_SPECULAR, specv);
		glLightfv(lights[i]->lightnum, GL_SHININESS, shini);
	}
	glEnable(GL_COLOR_MATERIAL);
	for (int i = 0; i < objects; i++)
		sceneobjects[i]->render();
	for (int i = 0; i < numlights; i++)
		glDisable(lights[i]->lightnum);
	glDisable(GL_COLOR_MATERIAL);
	glColor3f(BACKGROUND.x, BACKGROUND.y, BACKGROUND.z);
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