all: build

build: main.o color.o ray.o vec3.o mat4.o
	g++ -g -o main main.o color.o ray.o vec3.o mat4.o

main.o: main.cc
	g++ -g -c main.cc

color.o: color.cc
	g++ -g -c color.cc

ray.o: ray.cc
	g++ -g -c ray.cc

vec3.o: vec3.cc
	g++ -g -c vec3.cc

mat4.o: mat4.cc
	g++ -g -c mat4.cc

clean:
	rm *.o main

