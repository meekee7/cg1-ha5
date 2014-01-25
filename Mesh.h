#include "GLSLShader.h"
#include "glm\glm.hpp"
#include "Ray.h"
using namespace glm;
#pragma once
class Mesh
{
public:
	Mesh();
	~Mesh();
	bool loadOff(std::string file);
	void printmesh();
	enum RenderMode{
		FLAT_RENDERER,
		GOURAUD_RENDERER,
		TEXTURE_RENDERER
	};
	void setRenderMode(RenderMode mode);
	void render();
	Ray* intersectModel(Ray* ray);

//private:
	typedef struct {
		int size;
		vec3 normal;
		int* nodes;
		struct {
			vec3 gamma1, beta1, normal;
			float gamma2, beta2;
		} h;
	} poly;
	typedef struct {
		vec3 normal;
		vec3 node;
		vec2 tex;
	} nodestruct;
	int nodes;
	int polygons;
	int edges;
	nodestruct* node;
	poly* polygon;
	RenderMode rendermode;
	void renderFlat();
	void renderSmooth();
	void renderTextured();
	vec3 normalizevector(vec3 vector);
	vec3 crossproduct(vec3 v1, vec3 v2);
	float dot(vec3 v1, vec3 v2);

	Ray* intersectpolygon(poly poly, Ray* ray);
};