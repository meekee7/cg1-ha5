#pragma once
#include "glm/glm.hpp"
#include "Ray.h"
using namespace glm;
class Hitresult
{
public:
	Hitresult();
	~Hitresult();
	float distance;
	vec3 colour;
	Ray* reflectray;
	int originpoly;
	void* originmodel;
	struct {
		float u;
		float v;
		Ray* ray;
	} shadestuff;
};

