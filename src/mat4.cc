
#include "mat4.h"

mat4::mat4() : e{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} {}
mat4::mat4(double e00, double e01, double e02, double e03,
	   double e04, double e05, double e06, double e07,
	   double e08, double e09, double e10, double e11,
	   double e12, double e13, double e14, double e15) : 
	e{e00, e01, e02, e03,
          e04, e05, e06, e07,
          e08, e09, e10, e11,
          e12, e13, e14, e15} {}

double mat4::operator()(int i, int j) const { return e[i*4+j]; }
double& mat4::operator()(int i, int j) { return e[i*4+j]; }

mat4& mat4::operator+=(const mat4 &m) {
	for (int i = 0; i < 16; i++) e[i] += m.e[i];
	return *this;
}

mat4& mat4::operator-=(const mat4 &m) {
	for (int i = 0; i < 16; i++) e[i] -= m.e[i];
	return *this;
}

mat4& mat4::operator*=(double t) {
	for (int i = 0; i < 16; i++) e[i] *= t;
	return *this;
}

mat4& mat4::operator/=(double t) {
	for (int i = 0; i < 16; i++) e[i] /= t;
	return *this;
}

std::ostream& operator<<(std::ostream &out, const mat4 &v) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++ ) {
			out << v(i, j);
			if (j != 3) out << ' ';
		}
		if (i != 3) out << "\n";
	}
	return out;
}

mat4 operator+(const mat4 &u, const mat4 &v) {
	mat4 output = mat4();
	for (int i = 0; i < 16; i++) output.e[i] = u.e[i] + v.e[i];
	return output;
}

mat4 operator-(const mat4 &u, const mat4 &v) {
	mat4 output = mat4();
	for (int i = 0; i < 16; i++) output.e[i] = u.e[i] - v.e[i];
	return output;
}

// Matrix-matrix multiplication
mat4 operator*(const mat4 &u, const mat4 &v) {
	mat4 output = mat4();
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) for (int k = 0; k < 4; k++) {
		output(i, j) += u(i, k)*v(k, j);
	}
	return output;
}

mat4 operator*(double t, const mat4 &v) {
	mat4 output = mat4();
	for (int i = 0; i < 16; i++) output.e[i] = v.e[i] * t;
	return output;
}

mat4 operator*(const mat4 &v, double t) {
	return t * v;
}

// Assuming augmented 0 in v[3]
// Assuming vertical vector
vec3 operator*(const mat4 &m, const vec3 &v) {
	vec3 output = vec3();
	for (int i = 0; i < 3; i++) {
		for (int k = 0; k < 3; k++) output[i] += m(i, k) * v[k];
		//output[i] += m(i, 3) * 1.0; // augmented 1
	}
	return output;
}


mat4 operator/(mat4 v, double t) {
	mat4 output = mat4();
	for (int i = 0; i < 16; i++) output.e[i] = v.e[i] / t;
	return output;
}

mat4 transpose(const mat4 &m) {
	mat4 output = mat4();
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) output(j, i) = m(i, j);
	return output;
}

// determinant not implemented... I hope I dont need it :)

mat4 ones() {
	return mat4(1, 1, 1, 1,
		    1, 1, 1, 1,
		    1, 1, 1, 1,
		    1, 1, 1, 1);
}

mat4 unit() {
	return mat4(1, 0, 0, 0,
		    0, 1, 0, 0,
		    0, 0, 1, 0,
		    0, 0, 0, 1);
}

mat4 view(point3 from, point3 to, vec3 up) {
	vec3 f = normalize(from-to);
	vec3 s = normalize(cross(up, f));
	vec3 u = normalize(cross(f, s));
	//std::cerr << "eye: " << eye << "\ncenter: " << center << "\n up: " << up << "\nf: " << f << "\ns: " << s << "\nu: " << u << "\n";

	return mat4(s[0], u[0], f[0], 0.0,
		    s[1], u[1], f[1], 0.0,
		    s[2], u[2], f[2], 0.0,
		     0.0,  0.0,  0.0, 1.0);
}
