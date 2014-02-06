#include "glm/glm.hpp"

using namespace glm;

#pragma once
class Light
{
public:
	Light();
	~Light();
	vec3 position;
	unsigned int lightnum;
};
