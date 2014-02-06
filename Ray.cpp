#include "Ray.h"




Ray::~Ray()
{
}
Ray::Ray() : tmin(0) {}
Ray::Ray(const vec3& origin, const vec3& dir)
: o(origin), d(dir), tmin(0)
{
	duration = 16;
	shadowtest = false;
}
Ray::Ray(const vec3& origin, const vec3& dir, int duration)
: o(origin), d(dir), tmin(0)
{
	this->duration = duration;
	shadowtest = false;
}
Ray::Ray(const vec3& origin, const vec3& dir, float eps)
: o(origin + eps*dir), d(dir), tmin(0)
{
	duration = 16;
	shadowtest = false;
}

vec3 Ray::att(float t)
{
	return o + t*d;
}