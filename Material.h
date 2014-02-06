#include "glm/glm.hpp"
//#include "Scene.h"
using namespace glm;
#pragma once
class Material
{
public:
	Material();
	Material(vec3 colour);
	~Material();
	vec3 colour;
	bool reflecting;
};

