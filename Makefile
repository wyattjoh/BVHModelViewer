UNAME := $(shell uname -s)
GCC = g++

DEBUG_FLAGS = 
DEBUG_FLAGS = -DOPENGLDEBUG -Wall -Wextra

ifeq ($(UNAME),Darwin)
	FLAGS = -framework Cocoa -framework OpenGL -framework GLUT
	CFLAGS =  -std=gnu++11 $(DEBUG_FLAGS)
else
	FLAGS = -I/usr/include -L/usr/lib -lglut -lGL -lGLU -lX11
	CFLAGS = -std=c++0x $(DEBUG_FLAGS)
endif

all: motionviewer

motionviewer: src/bvh_loader.o src/motionviewer.o src/opengl.o
	$(GCC) src/bvh_loader.o src/opengl.o src/motionviewer.o -o motionviewer $(FLAGS)

src/bvh_loader.o: src/bvh_loader.h src/bvh_loader.cpp
	$(GCC) -c src/bvh_loader.cpp -o src/bvh_loader.o $(CFLAGS)

src/opengl.o: src/opengl.h src/opengl.cpp
	$(GCC) -c src/opengl.cpp -o src/opengl.o $(CFLAGS)

src/motionviewer.o: src/motionviewer.cpp
	$(GCC) -c src/motionviewer.cpp -o src/motionviewer.o $(CFLAGS)

clean:
	rm -rf src/*.o
	rm -rf motionviewer
	rm -rf output.obj
