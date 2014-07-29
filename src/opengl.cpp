#include "opengl.h"

OpenGL * OpenGL::current_object;
// int OpenGL::error_count = 0;

OpenGL::OpenGL(const char * filename)
{
  window.width = 500.0;
  window.height = 500.0;

  // Set default animation delay
  delay = initial_delay;
  animation_status = 1;

  // Reset origins
  camera_origin = new origin(0.0, 0.0, 0.0);
  camera_angle = new origin(0.0, 0.0, 0.0);

  // Load BVH
  load(filename);

  // Setup static current object
  current_object = this;
}

OpenGL::~OpenGL()
{
	// Clean up BVH data
	delete bvh_data;

  // Clean up origins
  delete camera_origin;
  delete camera_angle;
}

void OpenGL::load(const char * filename)
{
	bvh_data = new BVH(filename);
	number_animation_frames = bvh_data->animation_frames();
	current_frame = 0;
}

void OpenGL::gl_timer_function(int)
{
  glutPostRedisplay();
	glutTimerFunc(current_object->delay, OpenGL::gl_timer_function, 0);
}

void OpenGL::gl_init(int argc, char **argv)
{
   glutInitWindowSize(window.width, window.height);
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutCreateWindow(argv[0]);

   // Wireframe mode
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   gl_camera_view();

   // Display and run the main loop
   glutDisplayFunc(OpenGL::gl_display);
   glutReshapeFunc(OpenGL::gl_reshape);
   glutKeyboardFunc(OpenGL::gl_keyboard);
   glutSpecialFunc(OpenGL::gl_special_keyboard);
   glutTimerFunc(delay, OpenGL::gl_timer_function, 0);

   glutMainLoop();
}

void OpenGL::gl_camera_view()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glm::vec3 animation_min = current_object->bvh_data->animation_minimum();
  glm::vec3 animation_max = current_object->bvh_data->animation_maximum();

  gluPerspective(25.0, (GLfloat)current_object->window.width / (GLfloat)current_object->window.height, 0.01, 1000.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glm::vec3 eye_point;

  eye_point.x = (animation_max.x + animation_min.x)/2.0;
  eye_point.y = (animation_max.y + animation_min.y)/2.0;
  eye_point.z = animation_max.z + 3 * max(animation_max.x - animation_min.x, animation_max.y - animation_min.y);

  glm::vec3 reference_point(eye_point);
  reference_point.z = animation_min.z;

  gluLookAt(eye_point.x, eye_point.y, eye_point.z,
            reference_point.x, reference_point.y, reference_point.z,
            0.0, 1.0, 0.0);
}

void OpenGL::gl_reshape(const int w, const int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);

  current_object->window.width = w;
  current_object->window.height = h;

  gl_camera_view();
}

void OpenGL::decrease_animation_speed()
{
  current_object->delay += 0.6;
}

void OpenGL::increase_animation_speed()
{
  current_object->delay -= 0.6;

  if (current_object->delay < 0.0)
    current_object->delay = 0.0;
}

void OpenGL::gl_keyboard(unsigned char key, int, int)
{
  if (key == 'w') {
    current_object->bvh_data->save_bvh();
    return;
  }

  switch (key) {
      case 27:
      case 'q':
      case 'Q':
          exit(0);
          break;

      // Animation Controls
      // Pause the animation
      case 'P':
        current_object->animation_status = 0;
        break;
      // Play the animation
      case 'p':
        current_object->animation_status = 1;
        break;
      // Stop animation and reset current frame to 0
      case 'x':
        // Current frame to 0
        current_object->current_frame = 0;

        // Turn off the animation
        current_object->animation_status = 0;
        current_object->delay = OpenGL::initial_delay;

        glutPostRedisplay();
        break;

      case '+':
        current_object->increase_animation_speed();
        break;

      case '-':
        current_object->decrease_animation_speed();
        break;

      // TRANSLATE CAMERA
      // Translate Z camera
      case 'i':
        current_object->camera_origin->translate_z_neg();
        break;
      case 'I':
        current_object->camera_origin->translate_z_pos();
        break;

      // ROTATE CAMERA
      // Rotate X camera
      case 't':
        current_object->camera_angle->rotate_x_cw();
        break;
      case 'T':
        current_object->camera_angle->rotate_x_ccw();
        break;

      // Rotate Y camera
      case 'a':
        current_object->camera_angle->rotate_y_cw();
        break;
      case 'A':
        current_object->camera_angle->rotate_y_ccw();
        break;

      // Rotate Z camera
      case 'l':
        current_object->camera_angle->rotate_z_cw();
        break;
      case 'L':
        current_object->camera_angle->rotate_z_ccw();
        break;
  }
}

void OpenGL::gl_special_keyboard(int key, int, int)
{
  switch(key) {
    // TRANSLATE OBJECT
    // Translate object Y
    case GLUT_KEY_UP:
       current_object->camera_origin->translate_y_pos();
       break;
    case GLUT_KEY_DOWN:
       current_object->camera_origin->translate_y_neg();
       break;

    // Translate object X
    case GLUT_KEY_LEFT:
       current_object->camera_origin->translate_x_neg();
       break;
    case GLUT_KEY_RIGHT:
       current_object->camera_origin->translate_x_pos();
       break;
    }
}

void OpenGL::gl_display()
{
  // Introduce colors
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0, 1.0, 1.0);

  // // Rotate the camera
  glMatrixMode(GL_PROJECTION);

    // Rotate the camera
    glRotatef(current_object->camera_angle->x, 1.0, 0.0, 0.0); // X
    glRotatef(current_object->camera_angle->y, 0.0, 1.0, 0.0); // Y
    glRotatef(current_object->camera_angle->z, 0.0, 0.0, 1.0); // Z

    // Translate the camera
    glTranslatef(-current_object->camera_origin->x, -current_object->camera_origin->y, -current_object->camera_origin->z);

	glMatrixMode(GL_MODELVIEW);

    // Render the skeleton
  	render_hierarchy();

    #ifdef OPENGLDEBUG
    render_min_max();
    #endif

	glutSwapBuffers();

  // Zero Camera
  current_object->camera_angle->zero();
  current_object->camera_origin->zero();
}

void OpenGL::render_min_max()
{
  glm::vec3 animation_min = current_object->bvh_data->animation_minimum();
  glm::vec3 animation_max = current_object->bvh_data->animation_maximum();

  glm::vec3 eye_point;

  eye_point.x = (animation_max.x + animation_min.x)/2.0;
  eye_point.y = (animation_max.y + animation_min.y)/2.0;
  eye_point.z = animation_max.z + 5.7 * (animation_max.x - animation_min.x);

  glm::vec3 reference_point(eye_point);
  reference_point.z = animation_min.z;



  glBegin(GL_QUADS);
    glColor3f(0.0f, 1.0f,0.0f); // green drawing colour
    glVertex3f(animation_min.x, animation_min.y, animation_max.z);
    glVertex3f(animation_min.x, animation_max.y, animation_max.z);
    glVertex3f(animation_max.x, animation_max.y, animation_max.z);
    glVertex3f(animation_max.x, animation_min.y, animation_max.z);
  glEnd();

  glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f,0.0f); // green drawing colour
    glVertex3f(animation_min.x, animation_min.y, animation_min.z);
    glVertex3f(animation_min.x, animation_max.y, animation_min.z);
    glVertex3f(animation_max.x, animation_max.y, animation_min.z);
    glVertex3f(animation_max.x, animation_min.y, animation_min.z);
  glEnd();

  glPointSize(5.0f);

  // Cyan
  glBegin(GL_POINTS);
    glColor3f(0.0f, 1.0f,1.0f); // green drawing colour
    glVertex3f(animation_min.x, animation_min.y, animation_min.z);
  glEnd();

  // Dark Blue
  glBegin(GL_POINTS);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(animation_max.x, animation_max.y, animation_max.z);
  glEnd();

  glBegin(GL_POINTS);
    glColor3f(0.8f, 0.18f, 0.73f);
    glVertex3f(eye_point.x, eye_point.y, eye_point.z);
  glEnd();

  glBegin(GL_POINTS);
    glColor3f(0.18f, 0.8f, 0.73f);
    glVertex3f(reference_point.x, reference_point.y, reference_point.z);
  glEnd();

  glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(eye_point.x, eye_point.y, eye_point.z);
    glVertex3f(reference_point.x, reference_point.y, reference_point.z);
  glEnd();
}

void OpenGL::render_hierarchy()
{
	render_joint(current_object->bvh_data->gethierarchy());

  // Only advance the frame if the animation is running
  if (current_object->animation_status)
  	current_object->next_animation_frame();
}

void OpenGL::render_joint(JOINT * parent_joint)
{
	glPushMatrix();

	if (parent_joint->parent != NULL) {
		glm::vec4 * parent_vertex = current_vertex(parent_joint->parent);
		glm::vec4 * joint_vertex = current_vertex(parent_joint);

		glBegin(GL_LINES);
				glVertex3f(parent_vertex->x, parent_vertex->y, parent_vertex->z);
				glVertex3f(joint_vertex->x, joint_vertex->y, joint_vertex->z);
		glEnd();
	}

  // Itterate over children, rendering each one in turn
	for (auto & child: parent_joint->children)
		render_joint(child);

	glPopMatrix();
}

void OpenGL::next_animation_frame()
{
	assert(current_object->number_animation_frames);
	current_object->current_frame = (current_object->current_frame + 1) % current_object->number_animation_frames;
}

inline glm::vec4 * OpenGL::current_vertex(JOINT * joint)
{
	assert(joint != NULL);
	return &(joint->animation_frames[current_object->current_frame]);
}
