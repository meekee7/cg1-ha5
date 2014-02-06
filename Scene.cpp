#include "Scene.h"
using namespace std;

Scene::Scene()
{
	objects = 0;
	numlights = 0;
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
	numlights = 3;
	lights = new Light*[numlights];
	Light* l1 = new Light();
	l1->position = vec3(0, 0, -2);
	lights[0] = l1;
	Light* l2 = new Light();
	l2->position = vec3(0, 0, 0);
	lights[1] = l2;
	Light* l3 = new Light();
	l3->position = vec3(0, 0, -7);
	lights[2] = l3;
}

void Scene::loadscenedata(){
	this->makelights();
	mat4 mat;
	objects = 7;
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
	Mesh* planeback = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(0, 0, -8, 1));
	planeback->loadOff("scenedata/planeback.off", mat, new Material(), this);
	planeback->setRenderMode(Mesh::FLAT_RENDERER);
	sceneobjects[2] = planeback;
	Mesh* planeleft = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(-8, 0, 0, 1));
	planeleft->loadOff("scenedata/planeverti.off", mat, new Material(vec3(0.8, 0.8, 0.0)), this);
	planeleft->setRenderMode(Mesh::FLAT_RENDERER);
	planeleft->invertnormals();
	sceneobjects[3] = planeleft;
	Mesh* planeright = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(8, 0, 0, 1));
	planeright->loadOff("scenedata/planeverti.off", mat, new Material(vec3(0.0, 0.8, 0.8)), this);
	planeright->setRenderMode(Mesh::FLAT_RENDERER);
	sceneobjects[4] = planeright;
	Mesh* planeup = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(0, 8, 0, 1));
	planeup->loadOff("scenedata/planehori.off", mat, new Material(vec3(0.8, 0.8, 0.8)), this);
	planeup->setRenderMode(Mesh::FLAT_RENDERER);
	planeup->invertnormals();
	sceneobjects[5] = planeup;
	Mesh* planedown = new Mesh();
	mat = mat4(vec4(10, 0, 0, 0), vec4(0, 10, 0, 0), vec4(0, 0, 10, 0), vec4(0, -8, 0, 1));
	planedown->loadOff("scenedata/planehori.off", mat, new Material(vec3(0.8, 0.0, 0.8)), this);
	planedown->setRenderMode(Mesh::FLAT_RENDERER);
	sceneobjects[6] = planedown;
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