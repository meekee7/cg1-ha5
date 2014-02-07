#include "Mesh.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>

#include "Ray.h"
#include "Scene.h"

#define NONE -1
using namespace std;
Mesh::Mesh()
{
	Mesh::nodes = 0;
	Mesh::polygons = 0;
	Mesh::edges = 0;
}


Mesh::~Mesh()
{
	for (int i = 0; i < polygons; i++)
		delete polygon[i].nodes;
	delete polygon;
	delete node;
	delete material;
}

bool Mesh::loadOff(std::string filename, mat4 modelview, Material* material, void* scene){
	this->scene = scene;
	this->material = material;
	ifstream modelfile(filename);
	if (modelfile.is_open()) {
		string line; //Using short-circuit-or intentionally here
		if (!getline(modelfile, line) || line.compare("OFF")){
			modelfile.close();
			return false;
		} //error handling because file is empty or no OFF file
		if (getline(modelfile, line)){
			std::istringstream istr(line);
			istr >> nodes >> polygons >> edges;
		}
		else {
			modelfile.close();
			return false;
		} //error handling because line is missing
		node = new nodestruct[nodes];
		polygon = new poly[polygons];
		for (int i = 0; i < nodes; i++)
			if (getline(modelfile, line)){
				std::istringstream istr(line);
				node[i].normal = vec3(0, 0, 0);
				node[i].hnormal = vec3(0, 0, 0);
				istr >> node[i].node.x >> node[i].node.y >> node[i].node.z;
			}
			else {
				modelfile.close();
				return false;
			} //error handling because line is missing
			{ //Center the model
				vec3 sum = vec3(0, 0, 0);
				for (int i = 0; i < nodes; i++)
					sum += node[i].node;
				sum /= nodes;
				for (int i = 0; i < nodes; i++)
					node[i].node -= sum;
			}
			{ //Normalize the distance
				GLfloat avg = 0;
				for (int i = 0; i < nodes; i++)
					avg += std::sqrt(node[i].node.x * node[i].node.x + node[i].node.y * node[i].node.y + node[i].node.z * node[i].node.z);
				avg /= nodes;
				for (int i = 0; i < nodes; i++)
					node[i].node /= avg;
			}
			{ //Apply model transformation
				for (int i = 0; i < nodes; i++)
					node[i].node = (vec3)(modelview * vec4(node[i].node, 1));
			}
			{ //Calculate boundary box
				vec3 min = node[0].node;
				vec3 max = node[0].node;
				for (int i = 1; i < nodes; i++)
					for (int j = 0; j<3; j++)
						if (min[j] > node[i].node[j])
							min[j] = node[i].node[j];
						else if (max[j] < node[i].node[j])
							max[j] = node[i].node[j];
				this->boundaries.min = min;
				this->boundaries.max = max;
			}
			for (int i = 0; i < polygons; i++)
				if (getline(modelfile, line)){
					std::istringstream istr(line);
					istr >> polygon[i].size;
					polygon[i].nodes = new int[polygon[i].size];
					for (int j = 0; j < polygon[i].size; j++)
						istr >> polygon[i].nodes[j];
					{ //Calculate surface normal vector
						vec3 a = node[polygon[i].nodes[0]].node - node[polygon[i].nodes[1]].node;
						vec3 b = node[polygon[i].nodes[1]].node - node[polygon[i].nodes[2]].node;
						polygon[i].normal = glm::normalize(glm::cross(a, b));
						polygon[i].hnormal = glm::normalize(glm::cross(node[polygon[i].nodes[2]].node - node[polygon[i].nodes[0]].node, node[polygon[i].nodes[1]].node - node[polygon[i].nodes[0]].node));
					}
					{ //Apply surface normal vector to vertex normal vectors
						for (int j = 0; j < 3; j++){
							node[polygon[i].nodes[j]].normal += polygon[i].normal;
							node[polygon[i].nodes[j]].hnormal += polygon[i].hnormal;
						}
					}
				}
				else {
					modelfile.close();
					return false;
				} //error handling because line is missing
				modelfile.close();
				for (int i = 0; i < nodes; i++) {//Normalize vertex normal vectors
					node[i].normal = glm::normalize(node[i].normal);
					node[i].hnormal = glm::normalize(node[i].hnormal);
					{ //Calculate spherical texture coordinates
						const GLfloat pi = 3.1415926f; //TODO find optimal algorithm
						GLfloat modlength = 2.0f * sqrtf(node[i].normal[0] * node[i].normal[0] + node[i].normal[1] * node[i].normal[1] + (1.0f + node[i].normal[2]) * (1.0f + node[i].normal[2]));
						//This selector is for debugging, so we can easily test another formula
						switch (7){ //TODO remove this selector and the unused algorithms when the decision was made
						case 0:
							node[i].tex[0] = node[i].normal[0] / modlength + 0.5f; //http://www.unc.edu/~zimmons/cs238/maps/environment.html
							node[i].tex[1] = node[i].normal[1] / modlength + 0.5f;
							break;
						case 1:
							node[i].tex[0] = asin(node[i].normal[0]) / pi + 0.5f; //http://www.mvps.org/directx/articles/spheremap.htm
							node[i].tex[1] = asin(node[i].normal[1]) / pi + 0.5f;
							break;
						case 2:
							node[i].tex[0] = -atan2(node[i].normal[0], node[i].normal[1]) / pi * 0.5f;
							node[i].tex[1] = asin(node[i].normal[2]) / pi + 0.5f;
							break;
						case 3: //Vorlesungsfolien
							node[i].tex[0] = (pi + atan2f(node[i].normal[0], node[i].normal[1])) / (2 * pi);
							node[i].tex[1] = -atan2f(sqrtf(node[i].normal[0] * node[i].normal[0] + node[i].normal[1] * node[i].normal[1]), node[i].normal[2]) / pi;
							break;
						case 4: //http://hub.jmonkeyengine.org/forum/topic/coordinate-conversion-and-sphere-mapping/
							node[i].tex[1] = -asinf(node[i].normal[2] / sqrtf(node[i].normal[0] * node[i].normal[0] + node[i].normal[1] * node[i].normal[1] + node[i].normal[2] * node[i].normal[2])) / pi;
							node[i].tex[0] = atan2f(node[i].normal[1], node[i].normal[0]) / pi;
							break;
						case 5:
							node[i].tex[0] = (pi + atan2f(node[i].node[0], node[i].node[1])) / (2 * pi);
							node[i].tex[1] = atan2f(sqrtf(node[i].node[0] * node[i].node[0] + node[i].node[1] * node[i].node[1]), node[i].node[2]) / pi;
							break;
						case 6: //http://www.blendpolis.de/viewtopic.php?f=14&t=37042
							node[i].tex[0] = atan2f(node[i].normal[1] / node[i].normal[0], 0.5f) / pi;
							node[i].tex[1] = node[i].normal[2] * 0.5f;
							break;
						case 7: //http://stackoverflow.com/questions/19723698/sphere-texture-mapping-error
							node[i].tex.x = 0.5f - atan2f(node[i].normal.z, node[i].normal.x) / (2.0f * pi);
							node[i].tex.y = 0.5f + asinf(node[i].normal.y) / pi;
							break;
						}
					}
				}
	}
	else
		return false; //error because file was not opened
	return true;
}

Hitresult* Mesh::intersectModel(Ray* ray){
	int closestpoly = NONE;
	Hitresult* hit = nullptr;
	if (this->intersectboundarybox(ray))
		for (int i = 0; i < this->polygons; i++){
			Hitresult* intersect = this->intersectpolygon(polygon[i], ray);
			if (intersect != nullptr)
				if (hit == nullptr){
					hit = intersect;
					closestpoly = i;
				}
				else if (intersect->distance < hit->distance){
					delete hit;
					hit = intersect;
					closestpoly = i;
				}
		}
	if (hit != nullptr){
		hit->originmodel = this;
		hit->originpoly = closestpoly;
	}
	return hit;
}
Hitresult* Mesh::intersectpolygon(poly poly, Ray* ray){
	Scene* scene = (Scene*) this->scene;
	scene->intercounter++;
	if (glm::dot(ray->d, poly.hnormal) <= 0.0f) //Wrong direction, no chance for a hit
		return nullptr;

	vec3 v2mv1 = node[poly.nodes[1]].node - node[poly.nodes[0]].node;
	vec3 v3mv1 = node[poly.nodes[2]].node - node[poly.nodes[0]].node;
	float det = glm::dot(glm::cross(ray->d, v3mv1), v2mv1);
	float invdet = 1.0f / det;
	float t = invdet * glm::dot(glm::cross(ray->o - node[poly.nodes[0]].node, v2mv1), v3mv1);
	float u = invdet * glm::dot(glm::cross(ray->d, v3mv1), ray->o - node[poly.nodes[0]].node);
	float v = invdet * glm::dot(glm::cross(ray->o - node[poly.nodes[0]].node, v2mv1), ray->d);
	if (0.0f < t && 0.0f < u && 0.0f < v && ((u + v) < 1)){
		Hitresult* hit = new Hitresult();
		hit->distance = t;
		vec3 origin = ray->att(t);;
		vec3 hitnormal = glm::normalize(u*node[poly.nodes[1]].hnormal + v*node[poly.nodes[2]].hnormal + (1 - u - v)*node[poly.nodes[0]].hnormal);
		vec3 direction = glm::normalize(-2.0f * dot(ray->d, hitnormal)*hitnormal - ray->d);
		hit->reflectray = new Ray(origin, direction, ray->duration - 1);

		if (ray->shadowtest) //We do not need the rest and avoid endless recursion 
			return hit;

		vec3 colour;
		if (!this->material->reflecting)
			colour = this->material->colour;
		else{
			if (ray->duration <= 1)
				colour = BACKGROUND;
			else {
				Hitresult* reflection = scene->intersectscene(hit->reflectray);
				if (reflection != nullptr){
					colour = reflection->colour;
					delete reflection;
				}
				else
					colour = BACKGROUND;
			}
		}

		const vec4 specv = vec4(0.7f, 0.7f, 0.7f, 1.0f);
		const vec4 diffv = vec4(0.6f, 0.6f, 0.6f, 1.0f);
		const vec4 ambv = vec4(0.3f, 0.3f, 0.3f, 1.0f);
		float shininess = 128.0f;
		vec3 eye = vec3(0, 5, 20); //see main.cpp
		vec3 normalv = this->rendermode == Mesh::GOURAUD_RENDERER ? hitnormal : poly.hnormal; //Second case is flat-shading

		hit->colour = vec3(0, 0, 0);
		for (int i = 0; i < scene->numlights; i++){
			vec3 lightpos = scene->lights[0]->position;
			vec3 lightdir = glm::normalize(lightpos - hit->reflectray->o);
			if (scene->showshadow){ //Shadow
				Ray* shadowray = new Ray(hit->reflectray->o, lightdir, 1);
				shadowray->shadowtest = true;
				Hitresult* shadowhit = scene->intersectscene(shadowray);
				bool isshadow = shadowhit != nullptr && shadowhit->distance < glm::length(lightpos - hit->reflectray->o);
				delete shadowhit;
				delete shadowray;
				if (isshadow){
					hit->colour = vec3(0, 0, 0);
					return hit;
				}
			}
			vec3 half = glm::normalize(lightdir + eye);
			float diffuse = glm::dot(normalv, lightdir);
			diffuse = diffuse < 0.0f ? 0.0f : (diffuse > 1.0f ? 1.0f : diffuse);
			float halfdnormal = glm::dot(half, normalv);
			vec4 specular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			if (halfdnormal > 0.0f)
				specular = diffuse * pow(halfdnormal, shininess) * diffv * specv;
			vec4 lightedcolour = vec4(colour, 1.0f) * ambv + diffuse*vec4(colour, 1.0f)*specv + specular;

			hit->colour += (vec3)lightedcolour;
		}
		return hit;
	}
	else
		return nullptr;
}

void Mesh::swap(float* a, float* b){
	float t = *a;
	*a = *b;
	*b = t;
}

bool Mesh::intersectboundarybox(Ray* ray){
	vec3 tmin, tmax;
	tmin.x = (this->boundaries.min.x - ray->o.x) / ray->d.x;
	tmax.x = (this->boundaries.max.x - ray->o.x) / ray->d.x;
	if (ray->d.x < 0)
		swap(&tmin.x, &tmax.x);
	tmin.y = (this->boundaries.min.y - ray->o.y) / ray->d.y;
	tmax.y = (this->boundaries.max.y - ray->o.y) / ray->d.y;
	if (ray->d.y < 0)
		swap(&tmin.y, &tmax.y);
	if (tmin.x > tmax.y || tmin.y > tmax.y)
		return false;
	if (tmin.y > tmin.x)
		tmin.x = tmin.y;
	if (tmax.y < tmax.x)
		tmax.x = tmax.y;
	tmin.z = (this->boundaries.min.z - ray->o.z) / ray->d.z;
	tmax.z = (this->boundaries.max.z - ray->o.z) / ray->d.z;
	if (ray->d.z < 0)
		swap(&tmin.z, &tmax.z);
	if (tmin.x > tmax.z || tmin.z > tmax.x)
		return false;
	if (tmin.z > tmin.x)
		tmin.x = tmin.z;
	if (tmax.z < tmax.x)
		tmax.x = tmax.z;
	return (tmin.x > 0 && tmax.x > 0);
}

void Mesh::setRenderMode(RenderMode mode){
	this->rendermode = mode;
}

void Mesh::render(){
	vec3 colour = this->material->reflecting ? BACKGROUND : this->material->colour;
	glColor3f(colour.x, colour.y, colour.z);
	switch (rendermode){
	case FLAT_RENDERER:
		glShadeModel(GL_FLAT);
		this->renderFlat();
		break;
	case GOURAUD_RENDERER:
		glShadeModel(GL_SMOOTH);
		this->renderSmooth();
		break;
	case TEXTURE_RENDERER:
		glShadeModel(GL_SMOOTH);
		this->renderTextured();
		break;
	}
}

void Mesh::invertnormals(){
	for (int i = 0; i < this->nodes; i++){
		node[i].normal *= -1;
		node[i].hnormal *= -1;
	}
	for (int i = 0; i < this->polygons; i++){
		polygon[i].normal *= -1;
		polygon[i].hnormal *= -1;
	}
}

void Mesh::renderFlat(){
	for (int i = 0; i < polygons; i++){
		glNormal3f(polygon[i].normal[0], polygon[i].normal[1], polygon[i].normal[2]);
		glBegin(GL_TRIANGLES); {
			for (int j = 0; j < polygon[i].size; j++)
				glVertex3f(node[polygon[i].nodes[j]].node[0], node[polygon[i].nodes[j]].node[1], node[polygon[i].nodes[j]].node[2]);
		} glEnd();
	}
}

void Mesh::renderSmooth(){
	for (int i = 0; i < polygons; i++){
		glBegin(GL_TRIANGLES); {
			for (int j = 0; j < polygon[i].size; j++){
				glNormal3f(node[polygon[i].nodes[j]].normal[0], node[polygon[i].nodes[j]].normal[1], node[polygon[i].nodes[j]].normal[2]);
				glVertex3f(node[polygon[i].nodes[j]].node[0], node[polygon[i].nodes[j]].node[1], node[polygon[i].nodes[j]].node[2]);
			}
		} glEnd();
	}
}

void Mesh::renderTextured(){
	for (int i = 0; i < polygons; i++){
		glBegin(GL_TRIANGLES); {
			for (int j = 0; j < polygon[i].size; j++){ //Same as smooth, but assign texture
				//glTexCoord2fv(&node[polygon[i].nodes[j]].tex[0]);
				//glNormal3fv(&node[polygon[i].nodes[j]].normal[0]);
				//glVertex3fv(&node[polygon[i].nodes[j]].node[0]);
				glTexCoord2f(node[polygon[i].nodes[j]].tex.x, node[polygon[i].nodes[j]].tex.y);
				glNormal3f(node[polygon[i].nodes[j]].normal.x, node[polygon[i].nodes[j]].normal.y, node[polygon[i].nodes[j]].normal.z);
				glVertex3f(node[polygon[i].nodes[j]].node.x, node[polygon[i].nodes[j]].node.y, node[polygon[i].nodes[j]].node.z);
			}
		} glEnd();
	}
}

void Mesh::printmesh(){
	cout << "Nodes: " << nodes << " Polygons: " << polygons << " Edges: " << edges << "\n";
	cout << "Nodes: \n";
	for (int i = 0; i < nodes; i++){
		cout << i << ": " << node[i].node[0] << " " << node[i].node[1] << " " << node[i].node[2] << "\n";
		cout << "Normal " << node[i].normal[0] << " " << node[i].normal[1] << " " << node[i].normal[2] << "\n";
	}
	cout << "Polygons: \n";
	for (int i = 0; i < polygons; i++)
	{
		cout << i << ": with " << polygon[i].size << " Nodes: ";
		for (int j = 0; j < polygon[i].size; j++)
			cout << polygon[i].nodes[j] << " ";
		cout << "Normal " << polygon[i].normal[0] << " " << polygon[i].normal[1] << " " << polygon[i].normal[2] << "\n";
	}
}

