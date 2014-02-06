#include "GLSLShader.h"
#include "glm/glm.hpp"
#include "Ray.h"
#include "Hitresult.h"
using namespace glm;
#pragma once
class Mesh
{
public:
	Mesh();
	~Mesh();
	bool loadOff(std::string file, mat4 modelview);
	void printmesh();
	enum RenderMode{
		FLAT_RENDERER,
		GOURAUD_RENDERER,
		TEXTURE_RENDERER
	};
	void setRenderMode(RenderMode mode);
	void render();
	void invertnormals();
	Hitresult* intersectModel(Ray* ray);

//private:
	typedef struct {
		int size;
		vec3 normal, hnormal;
		int* nodes;
	} poly;
	typedef struct {
		vec3 hnormal;
		vec3 normal;
		vec3 node;
		vec2 tex;
	} nodestruct;
	struct {
		vec3 max;
		vec3 min;
	} boundaries;
	int nodes;
	int polygons;
	int edges;
	nodestruct* node;
	poly* polygon;
	RenderMode rendermode;
	void renderFlat();
	void renderSmooth();
	void renderTextured();

	Hitresult* intersectpolygon(poly poly, Ray* ray);
	bool intersectboundarybox(Ray* ray);
	void swap(float* a, float*b);
};