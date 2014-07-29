#pragma once

#include <algorithm> 
#include <cassert>
#include <cctype>
#include <functional> 
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>


using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;
using std::stringstream;
using std::string;
using std::cout;
using std::endl;
using std::vector;

#include "glm/glm.hpp"
#include "glm/ext.hpp"


struct OFFSET
{
    float x, y, z;
};

struct JOINT
{
    string name;               // joint name
    JOINT* parent;                  // joint parent
    OFFSET offset;                  // joint offset 
    unsigned int num_channels;      // number of channels 
    short* channels_order;          // array of channel order
    vector<JOINT*> children;        // vector of joint children 
    unsigned int channel_start;     // the id of the channel
    glm::mat4 matrix;               // local transofrmation matrix (premultiplied with parents'

    glm::vec4 * animation_frames;

    JOINT() {
        num_channels = 0;
        channel_start = 0;

        parent = NULL;
        channels_order = NULL;
    }
    ~JOINT() {
        for (vector<JOINT*>::iterator it = children.begin() ; it != children.end(); ++it)
            if (*it)
                delete *it;
    
        if (channels_order)
            delete [] channels_order;

        delete [] animation_frames;
    }

    void ini_animation_frames(unsigned int & num_frames) {
        animation_frames = new glm::vec4[num_frames];

        for (auto & child: children)
            child->ini_animation_frames(num_frames);
    }
};

struct MOTION
{
    unsigned int num_frames;              // number of frames
    unsigned int num_motion_channels; // number of motion channels 
    float* data;                   // motion float data array
    unsigned* joint_channel_offsets;      // number of channels from beggining of hierarchy for i-th joint
    float frame_time;

    // Animation frames
    unsigned int current_animation_frame;

    MOTION() {
        num_motion_channels = 0;
        num_frames = 0;
        current_animation_frame = 2;
    }
    ~MOTION() {
        delete [] data;
    }

    void next_frame() { current_animation_frame = (current_animation_frame + 1) % num_frames; }
    int current_frame() { return current_animation_frame; }
};


class BVH {
	public:
		BVH(const char * filename);
		~BVH();

        void save_bvh();

        // Returns the pointer to the rootJoint
        JOINT * gethierarchy() { return rootJoint; }

        // Returns the number of animation frames
        unsigned int animation_frames() { return motionData.num_frames; }
        void advance_joint_frame(JOINT * parent_joint); // Advances animation frame by one

        // Returns the min/max for the animation sequence
        glm::vec3 animation_minimum() { return * min_animation;}
        glm::vec3 animation_maximum() { return * max_animation;}

	private:
		BVH() {};

        // Loads the heirarchy
        void loadhierarchy(istream& stream);
        JOINT * loadjoint(istream& stream, JOINT* parent = NULL); // load joint from string sequence
        void loadmotion(istream& stream); // load motion from string sequence

        void preprocess_motion(); // Preprocess all the animation data to load the computed vectors
        void preprocess_joint_frame(JOINT * parent_joint, unsigned int & frame_number); // Preprocesses the paticular frame and joint
        void compute_min_max(glm::vec4 & vertex); // Computes the min/max for given vertex and the global
        
        void dumphierarchy(ostream& stream); // Dumps the hierarchy to the stream
        void dumpjoint(JOINT * parent_joint, ostream& stream, int & tab_level); // Dump joint to the stream
        void dumpmotion(ostream& stream); // Dumps the motion to the stream

        static string channel_index_to_string(short & i); // Converts the index to a string
        static short channel_string_to_index(string & channel_name); // Converts a strign to the channel index

        // Prints "tab_level" tabs to stream
        void print_tab(ostream& stream, int & tab_level);

        // Contains the joint data
		JOINT* rootJoint;

        // Contains the motion data
		MOTION motionData;

        // Min and max animation bounds
        glm::vec3 * min_animation;
        glm::vec3 * max_animation;

        // Constants for the extraction process
        static const int Xposition = 0x01;
        static const int Yposition = 0x02;
        static const int Zposition = 0x04;
        static const int Zrotation = 0x10;
        static const int Xrotation = 0x20;
        static const int Yrotation = 0x40;
};

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}