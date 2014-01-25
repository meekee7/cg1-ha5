#include "glm\glm.hpp"
using namespace glm;
#pragma once
class Ray
{
public:
	Ray();
	~Ray();
	Ray(const vec3& origin, const vec3& dir);
	Ray(const vec3& origin, const vec3& dir, float eps);

	vec3 att(float t);
	glm::vec3 o;
	glm::vec3 d;
	float hitdistance;

	float tmin;
	
};

