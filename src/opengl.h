#pragma once

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <algorithm>

using std::max;

#include "bvh_loader.h"

struct box
{
    float width;
    float height;
};

struct origin
{
    float x, y, z;
    origin(float nx, float ny, float nz) { x = nx; y = ny; z = nz; }

    static constexpr float rotation_constant = 10.0;
    static constexpr float translate_constant = 0.1;

    // Rotate
    // -------  Positive
    void rotate_x_cw() { x += rotation_constant; }
    void rotate_y_cw() { y += rotation_constant; }
    void rotate_z_cw() { z += rotation_constant; }
    // -------  Negative
    void rotate_x_ccw() { x -= rotation_constant; }
    void rotate_y_ccw() { y -= rotation_constant; }
    void rotate_z_ccw() { z -= rotation_constant; }

    // Translate
    // -------  Positive
    void translate_x_pos() { x += translate_constant; }
    void translate_y_pos() { y += translate_constant; }
    void translate_z_pos() { z += translate_constant; }
    // -------  Negative
    void translate_x_neg() { x -= translate_constant; }
    void translate_y_neg() { y -= translate_constant; }
    void translate_z_neg() { z -= translate_constant; }

    void zero() { x = y = z = 0; }
};

class OpenGL
{
	public:
        OpenGL(const char * filename);
        ~OpenGL();

        // OpenGL Related Functions
        void gl_init(int, char **);

    private:
    	OpenGL(); // Disable ini constructor

    	// Animation Controls
        static constexpr double initial_delay = 8.3; // Initial delay
    	double delay; // Delay inbetween frames
        
        // Whether the animation is being displayed or not
        bool animation_status;

        // Contains the BVH data
    	BVH * bvh_data;

        // Camera
        origin * camera_origin;
        origin * camera_angle;

        // Window
        box window;

        // Joint and motion data links
        JOINT * root_joint;
        MOTION * motion_data;

        // Frame inforamtion
        unsigned int current_frame;
        unsigned int number_animation_frames;

        // Trampoline object
        static OpenGL * current_object;

		// Trampoline functions
        static void gl_reshape(int w, int h);
        static void gl_keyboard(unsigned char key, int x, int y);
        static void gl_special_keyboard(int key, int x, int y);
        static void gl_display();
        static void gl_timer_function(int);

        static void invalidate_timer();

        // Loads BVH Data
        void load(const char * filename);

		static void render_hierarchy();
		static void render_joint(JOINT * parent_joint);
        static void render_min_max();

		static void next_animation_frame();					// Advances the animation by one frame
		static inline glm::vec4 * current_vertex(JOINT * joint);	// Returns the vertex for the current animation frame

        static void decrease_animation_speed();
        static void increase_animation_speed();

        static void gl_camera_view(); // Display the camera in default mode
};