#include "vec3.h"

#include "ray.h"

ray::ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction) {}

point3 ray::origin() const  { return orig; }
vec3 ray::direction() const { return dir; }

point3 ray::at(double t) const {
	return orig + t*dir;
}
