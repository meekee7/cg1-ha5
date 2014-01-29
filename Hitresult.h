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
	vec4 ambcolour;
	Ray* reflectray;
	int originpoly;
	void* originmodel;
};

