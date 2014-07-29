#include "opengl.h"

int main(int argc, char **argv)
{
	if (argc != 2)
		return 1;

	OpenGL * opengl = new OpenGL(argv[1]);

	opengl->gl_init(argc, argv);

	delete opengl;

	return 0;
}