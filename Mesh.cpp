#include "Mesh.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
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
}

bool Mesh::loadOff(std::string filename){
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
				polygon[i].normal = this->normalizevector(this->crossproduct(a, b));
				{
					vec3 x = node[polygon[i].nodes[1]].node - node[polygon[i].nodes[0]].node;
					vec3 y = node[polygon[i].nodes[2]].node - node[polygon[i].nodes[0]].node;
					float xdotx = dot(x, x);
					float xdoty = dot(x, y);
					float ydoty = dot(y, y);
					float D = 1.0f / (ydoty*xdotx - xdoty*xdoty);
					polygon[i].h.beta1 = (ydoty * D * x) - (y * xdoty * D);
					polygon[i].h.beta2 = dot(-node[polygon[i].nodes[0]].node, polygon[i].h.beta1);
					polygon[i].h.gamma1 = (y *xdotx * D) - (x*xdoty*D);
					polygon[i].h.gamma2 = dot(-node[polygon[i].nodes[0]].node, polygon[i].h.gamma1);
					polygon[i].h.normal = crossproduct(node[polygon[i].nodes[2]].node - node[polygon[i].nodes[0]].node, node[polygon[i].nodes[1]].node - node[polygon[i].nodes[0]].node);
				}
			}
			{ //Apply surface normal vector to vertex normal vectors
				for (int j = 0; j < 3; j++)
					node[polygon[i].nodes[j]].normal += polygon[i].normal;
			}
		}
		else {
			modelfile.close();
			return false;
		} //error handling because line is missing
		modelfile.close();
		for (int i = 0; i < nodes; i++) {//Normalize vertex normal vectors
			node[i].normal = this->normalizevector(node[i].normal);
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

Ray* Mesh::intersectModel(Ray* ray){ //TODO testen und beenden
	int closestpoly = -1;
	Ray* hit = nullptr;
	for (int i = 0; i < this->nodes; i++){
		Ray* intersect = this->intersectpolygon(polygon[i], ray);
		if (intersect != nullptr)
		if (hit == nullptr)
			hit = intersect;
		else if (intersect->hitdistance < hit->hitdistance){
			delete hit;
			hit = intersect;
		}
	}
	return hit;
}
Ray* Mesh::intersectpolygon(poly poly, Ray* ray){
	float d = dot(poly.h.normal, node[poly.nodes[0]].node);
	float direction = dot(poly.h.normal, ray->d);
	if (direction == 0.0f) // If we do not catch division by zero, then we get invalid results
		return nullptr;
	float distance = (d - dot(ray->o, poly.h.normal)) / direction;
	vec3 hitpoint = ray->att(distance);
	float beta = dot(poly.h.beta1, hitpoint) + poly.h.beta2; //dep
	float gamma = dot(poly.h.gamma1, hitpoint) + poly.h.gamma2; //dep
	float alpha = 1 - beta - gamma;
	if (direction <= 0.0f || beta < 0.0f || gamma < 0.0f || alpha < 0.0f)
		return nullptr;
	else {
		Ray* hit = new Ray();
		hit->o = hitpoint;
		hit->hitdistance = distance;
		//TODO hit->d soll die Richtung des reflektierten Strahls werden
		return hit;
	}
}

void Mesh::setRenderMode(RenderMode mode){
	this->rendermode = mode;
}

void Mesh::render(){
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

vec3 Mesh::normalizevector(vec3 vector){
	vec3 v = vec3(vector.x, vector.y, vector.z);
	GLfloat scale = v.x * v.x + v.y * v.y + v.z * v.z;
	int integer = 0x5f3759df - (*(long*)&scale >> 1); //Using the fast inverse square root algorithm
	GLfloat isr = *(GLfloat*)&integer;
	isr *= 1.5f - (scale * 0.5f * isr * isr); //One iteration should be precise enough
	//isr *= 1.5f - (scale * 0.5f * isr * isr);
	//isr *= 1.5f - (scale * 0.5f * isr * isr); //Three iterations are enough because GLfloat is less precise
	v *= isr;
	return v;
}

vec3 Mesh::crossproduct(vec3 v1, vec3 v2){
	return vec3(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

float Mesh::dot(vec3 v1, vec3 v2){
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
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

