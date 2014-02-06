#include "glm/glm.hpp"
//#include "Scene.h"
#define MIRRORMAT new Material()
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

