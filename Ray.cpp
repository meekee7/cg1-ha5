#include "Ray.h"




Ray::~Ray()
{
}
Ray::Ray() : tmin(0) {}
Ray::Ray(const vec3& origin, const vec3& dir)
: o(origin), d(dir), tmin(0)
{
}
Ray::Ray(const vec3& origin, const vec3& dir, float eps)
: o(origin + eps*dir), d(dir), tmin(0)
{
}

vec3 Ray::att(float t)
{
	return o + t*d;
}