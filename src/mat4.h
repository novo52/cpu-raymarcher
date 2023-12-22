#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"

class mat4 {
public:
	double e[16];

	mat4();
	mat4(double e00, double e01, double e02, double e03,
	     double e04, double e05, double e06, double e07,
	     double e08, double e09, double e10, double e11,
	     double e12, double e13, double e14, double e15);

	// Element access like matrix(i, j)
	double operator()(int i, int j) const;
	double& operator()(int i, int j);

	mat4 operator-() const;
	
	// x= operators
	mat4& operator+=(const mat4 &m);
	mat4& operator-=(const mat4 &m);
	mat4& operator*=(double t);
	mat4& operator/=(double t);

};

// Printing
std::ostream& operator<<(std::ostream &out, const mat4 &v);

// Operators
mat4 operator+(const mat4 &u, const mat4 &v);
mat4 operator-(const mat4 &u, const mat4 &v);
mat4 operator*(const mat4 &u, const mat4 &v);
mat4 operator*(double t, const mat4 &v);
mat4 operator*(const mat4 &v, double t);
vec3 operator*(const mat4 &m, const vec3 &v);
mat4 operator/(mat4 v, double t);

// Special operations
mat4 transpose(const mat4 &m);
double determinant(const mat4 &m);
mat4 unit();
mat4 view(point3 eye, point3 center, vec3 up);
#endif
