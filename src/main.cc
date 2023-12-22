#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <string>
#include <thread>
#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "mat4.h"

double sd_torus(point3 p, double a, double b) {
	double c = sqrt(p.y()*p.y() + p.x()*p.x())-a;
	return sqrt(c*c+p.z()*p.z())-b;
}

double sd_sphere(point3 p, double r) {
	return p.length() - r;
}

double scene_distance(point3 p) {
	
	return sd_torus(p, 0.5, 0.25);
	//return sd_sphere(p, 0.5);
}

vec3 estimate_normal(point3 p) {
	const double EPSILON = 0.0001;
	vec3 EPSILON_X = vec3(EPSILON, 0, 0);
	vec3 EPSILON_Y = vec3(0, EPSILON, 0);
	vec3 EPSILON_Z = vec3(0, 0, EPSILON);
	vec3 normal = vec3(
		scene_distance(p + EPSILON_X) - scene_distance(p - EPSILON_X),
		scene_distance(p + EPSILON_Y) - scene_distance(p - EPSILON_Y),
		scene_distance(p + EPSILON_Z) - scene_distance(p - EPSILON_Z));
	//std::cerr << normal << " ";
	return normalize(normal);
}

color phong_light_contribution(color kd, color ks, double alpha, 
		vec3 p, vec3 camera, vec3 light_pos, vec3 light_color) {
	vec3 N = estimate_normal(p);
	vec3 L = normalize(light_pos-p);
	vec3 V = normalize(camera-p);	
	double dotLN = dot(L, N);
	vec3 R = normalize(2.0*dotLN*N-L);
	double dotRV = dot(R, V);

	if (dotLN < 0.0) return color(0, 0, 0); // Not visible
	
	color diffuse = kd * dotLN;
	if (dotRV < 0.0) {
		return light_color * diffuse;
	} else {
		color specular = ks * pow(dotRV, alpha);
		return light_color * (diffuse + specular);
	}
}

color ray_color(const ray& r, double time) {
	
	const point3 camera_pos = point3(sin(time), 0.5, cos(time));
	const mat4 view_to_world = view(camera_pos, point3(), vec3(0.0, 1.0, 0.0));
	//const mat4 view_to_world = unit();
	//std::cerr << "\nMatrix:\n" << view_to_world << "\n";	
	ray r_transformed = ray(camera_pos, normalize(view_to_world * r.direction()));	

	const color ambient_light = color(0.5, 0.5, 0.5);
	point3 light_pos = view_to_world * point3(4.0, 2.0, 4.0);
	const color light_color = color(0.4, 0.4, 0.4);

	const color ka = color(0.2, 0.2, 0.2);
	const color kd = color(0.7, 0.2, 0.2);
	const color ks = color(1.0, 1.0, 1.0);
	const double shininess = 10.0;

	// Experimentally found to give closest result to raytracing with no antialiasing
	const double epsilon = 0.00075; 
	double t = 0;
	//if (debug_print) std::cerr << "New ray:\n";
	const int MAX_ITERATIONS = 255;
	const double MAX_DIST = 3.0;

	for (int i = 0; i < MAX_ITERATIONS; i++) {
		point3 p = r_transformed.at(t);
		double dist = scene_distance(p);

		if (dist < epsilon) {
			/*
			double l = ((double)i + dist/epsilon) / (double)20.0;
			if (l > 1.0) l = 1.0;

			l = 1.0-l;
			color c = color(0.5, 0.3, 0.7) * l;
*/
			// Oh yeah. its phong time
			color phong = phong_light_contribution(kd, ks, shininess, p, r_transformed.origin(), light_pos, light_color);
			color c = ambient_light * ka + phong;
			return c;
		}
		t += dist;
	}

	// The sky :)
	vec3 unit_direction = normalize(r_transformed.direction());
	auto a = 0.5*(unit_direction.y() + 1.0);
	return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}

void render_frame(std::string path, int img_width, int img_height, double time, int f, \
		vec3 pixel00, vec3 pixel_u, vec3 pixel_v, vec3 camera_center) {

	// File buffer
	std::stringstream file_buffer;
	std::stringstream().swap(file_buffer);

	// PPM File header
	file_buffer << "P3\n" << img_width << " " << img_height << "\n255\n";

	// Write pixels
	for (int y = 0; y < img_height; y++) {
		for (int x = 0; x < img_width; x++) {
			vec3 pixel_pos = pixel00 + x*pixel_u + y*pixel_v;
			vec3 ray_dir = pixel_pos - camera_center;
			ray r(camera_center, ray_dir);
			
			color pixel_color = ray_color(r, time);
			write_color(file_buffer, pixel_color);
		}
	}

	// Get filename
	std::ostringstream filename;
	std::ostringstream().swap(filename); // Clear filename stream
	filename << path << "/" << f << ".ppm";

	// Write data
	std::ofstream file;
	file.open(filename.str(), std::ofstream::trunc);
	file << file_buffer.str();
	file.close();

}

void test(int x) { }

void render_sequence(std::string path, int img_width, int img_height, int frame_count, \
		double time_delta, double focal_length, vec3 camera_center) {

	// Setup raycating
	double aspect_ratio = (double)img_width/(double)img_height;

	double viewport_height = 2.0;
	double viewport_width = viewport_height * aspect_ratio;
	vec3 viewport_u = vec3(viewport_width, 0, 0);
	vec3 viewport_v = vec3(0, -viewport_height, 0);
	vec3 pixel_u = viewport_u / img_width;
	vec3 pixel_v = viewport_v / img_height;

	vec3 viewport_upper_left = 
		camera_center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
	vec3 pixel00 = viewport_upper_left + 0.5 * (pixel_u + pixel_v);

	// 16 is close to optimal
	// Making a new thread for every frame (THREAD_COUNT == frame_count)
	// is surprisingly effective, but prefer lower thread counts if possible
	const int THREAD_COUNT = 16;
	std::thread threads[THREAD_COUNT];

	// Begin rendering
	for (int f = 0; f < frame_count; f++) {
		double time = (double)f * time_delta;	
		
		threads[f%THREAD_COUNT] = std::thread(
			render_frame, path, img_width, img_height, time, f,
			pixel00, pixel_u, pixel_v, camera_center);

		// After making THREAD_COUNT threads, join all of them
		if (f%THREAD_COUNT == 0) {
			int last_frame = f+THREAD_COUNT;
			if (last_frame > frame_count-1) last_frame = frame_count-1;
			std::cerr << "Rendering frames " << f << " to " << last_frame << "\n";
			for (int i = 0; i < THREAD_COUNT; i++) {
				if (threads[i].joinable()) threads[i].join();
			}
		}

	}

	// Join all remaining threads
	for (int i = 0; i < THREAD_COUNT; i++) {
		if (threads[i].joinable()) threads[i].join();
	}

	std::cerr << "Render complete\n";
}


int main() {
	const double ideal_aspect_ratio = 16.0 / 9.0;
	int img_width = 1080;
	int img_height = img_width / ideal_aspect_ratio;
	img_height -= img_height%2;

	double seconds = 6.2831855;
	double framerate = 60;
	
	int frame_count = framerate * seconds;
	double frame_delta = 1.0/framerate;

	std::string path = "../output";

	// Remove previous output and write framerate
	std::stringstream command;
	command << "rm " << path << "/*.ppm";
	std::cerr << command.str() << "\n";
	system(command.str().c_str());
	std::stringstream().swap(command);
	command << "echo \"" << framerate << "\" > " << path << "/framerate.var";
	std::cerr << command.str() << "\n";
	system(command.str().c_str());	

	auto start = std::chrono::high_resolution_clock::now();

	render_sequence(path, img_width, img_height, frame_count, frame_delta, 1.0, point3(0, 0, 0));

	auto end = std::chrono::high_resolution_clock::now();
	double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0f;

	std::cerr << "Render time: " << duration << "s (" << duration/frame_count \
		<< "s/frame)\n";
}
