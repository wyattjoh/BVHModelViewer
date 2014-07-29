#include "bvh_loader.h"

BVH::BVH(const char * filename)
{
	ifstream infile(filename);
	string line;

	if(infile.is_open()) {
		while(infile.good()) {
			infile >> line;
            if( trim(line) == "HIERARCHY" )
                loadhierarchy(infile);
            break;
		}

		infile.close();
    }
    else
        exit(1);

    preprocess_motion();
}

BVH::~BVH()
{
	delete rootJoint;
    delete min_animation;
    delete max_animation;
}

void BVH::loadhierarchy(istream& stream)
{
    string tmp;

    while(stream.good())
    {
        stream >> tmp;

        if (trim(tmp) == "ROOT")
            rootJoint = loadjoint(stream);
        else if(trim(tmp) == "MOTION")
            loadmotion(stream);
    }
}

void BVH::save_bvh()
{
    ofstream outfile("output.bvh");

    if (outfile.is_open()) {
        dumphierarchy(outfile);
        outfile.close();
    }

}

void BVH::dumphierarchy(ostream& stream)
{
    stream << "HIERARCHY" << endl;
    
    int tab_level = 0;

    dumpjoint(rootJoint, stream, tab_level);

    // Print motion data
    dumpmotion(stream);
}

void BVH::print_tab(ostream& stream, int & tab_level)
{
    for (int i = 0; i < tab_level; i++)
        stream << "\t";
}

void BVH::dumpjoint(JOINT * parent_joint, ostream& stream, int & tab_level)
{
    print_tab(stream, tab_level);
    // Check if end site
    if (parent_joint->children.size() == 0)
        stream << "End Site" << endl;
    else if (parent_joint->parent == NULL)
        stream << "ROOT " << parent_joint->name << endl;
    else
        stream << "JOINT " << parent_joint->name << endl;

    print_tab(stream, tab_level);
    stream << "{" << endl;

    tab_level++;
    // Print OFFSET Data
    print_tab(stream, tab_level);
    stream << "OFFSET ";
    stream << parent_joint->offset.x << " ";
    stream << parent_joint->offset.y << " ";
    stream << parent_joint->offset.z << endl;

    if (parent_joint->num_channels) {
        // Print CHANNELS Data
        print_tab(stream, tab_level);
        stream << "CHANNELS ";
        stream << parent_joint->num_channels << " ";

        for (unsigned int i = 0; i < parent_joint->num_channels; i++) {
            stream << channel_index_to_string(parent_joint->channels_order[i]);

            if (i + 1 < parent_joint->num_channels)
                stream << " ";
        }

        stream << endl;
    }
    tab_level--;

    for (auto & child: parent_joint->children) {
        tab_level++;
        dumpjoint(child, stream, tab_level);
        tab_level--;
    }

    print_tab(stream, tab_level);
    stream << "}" << endl;   
}

void BVH::dumpmotion(ostream& stream)
{
    stream << "MOTION" << endl;
    stream << "Frames: " << motionData.num_frames << endl;
    stream << "Frame Time: " << motionData.frame_time << endl;

    for (unsigned int frame = 0; frame < motionData.num_frames; frame++) {
        for (unsigned int channel = 0; channel < motionData.num_motion_channels; channel++) {
            int index = frame * motionData.num_motion_channels + channel;
            stream << motionData.data[index];

            if (channel + 1 < motionData.num_motion_channels)
                stream << " ";
        }
        // End of frame
        stream << endl;
    }
}

JOINT * BVH::loadjoint(istream& stream, JOINT* parent)
{
	JOINT* joint = new JOINT;
	joint->parent = parent;

	// load joint name
    stream >> joint->name;

    // load identity matrix
    joint->matrix = glm::mat4(1.0);

    std::string tmp;

    static int chanel_global_index = 0;
    unsigned channel_order_index = 0;

    while(stream.good()) {

    	stream >> tmp;
        tmp = trim(tmp);

        // loading channel order
        char c = tmp.at(0);
        if (c == 'X' || c == 'Y' || c == 'Z')
	        joint->channels_order[channel_order_index++] = channel_string_to_index(tmp);
	    // reading an offset values
	    else if (tmp == "OFFSET")
            stream  >> joint->offset.x >> joint->offset.y >> joint->offset.z;

        else if (tmp == "CHANNELS") {
            // loading num of channels
            stream >> joint->num_channels;

            // adding to motiondata
            motionData.num_motion_channels += joint->num_channels;

            // increasing static counter of channel index starting motion section
            joint->channel_start = chanel_global_index;
            chanel_global_index += joint->num_channels;

            // creating array for channel order specification
            joint->channels_order = new short[joint->num_channels];
        }
        else if (tmp == "JOINT") {
            // loading child joint and setting this as a parent
            JOINT* tmp_joint = loadjoint(stream, joint);

            tmp_joint->parent = joint;
            joint->children.push_back(tmp_joint);
        }
        else if (tmp == "End") {
            stream >> tmp >> tmp;

            JOINT * tmp_joint = new JOINT;

            tmp_joint->parent = joint;
            tmp_joint->num_channels = 0;
            tmp_joint->name = "End Site";
            joint->children.push_back(tmp_joint);

            stream >> tmp;
            if (tmp == "OFFSET")
                stream >> tmp_joint->offset.x >> tmp_joint->offset.y >> tmp_joint->offset.z;

            stream >> tmp;
        }
        else if(tmp == "}")
            return joint;
    }

	return joint;
}

void BVH::loadmotion(istream& stream)
{
    string tmp;
    stringstream frame_time;
    float x;
    
    int index;

    while (stream.good()) {
        stream >> tmp;

        if (trim(tmp) == "Frames:")
            stream >> motionData.num_frames;
        else if(trim(tmp) == "Frame") {
            // The word "Time:"
            stream >> tmp;

            // Actual frame time (fp)
            stream >> tmp;
            frame_time << tmp;
            frame_time >> motionData.frame_time;

            // creating motion data array
            motionData.data = new float[motionData.num_frames * motionData.num_motion_channels];

            // foreach frame read and store floats
            for (unsigned int frame = 0; frame < motionData.num_frames; frame++) {
                for (unsigned int channel = 0; channel < motionData.num_motion_channels; channel++) {
                    // reading float
                    stringstream ss;
                    stream >> tmp;
                    ss << tmp;
                    ss >> x;

                    // calculating index for storage
                    index = frame * motionData.num_motion_channels + channel;
                    motionData.data[index] = x;

                }
            }
        }
    }
}

void BVH::preprocess_motion()
{
    // Setup the joints storage for the animation
    rootJoint->ini_animation_frames(motionData.num_frames);

    // Step 1: Loop over frames
    for (unsigned int frame_number = 0; frame_number < motionData.num_frames; frame_number++) {
        // Step 2: Start the children's decent...
        preprocess_joint_frame(rootJoint, frame_number);
        motionData.next_frame();
    }
}

void BVH::compute_min_max(glm::vec4 & vertex)
{
    static int min_max_flag = 0;

    if (!min_max_flag) {
        min_animation = new glm::vec3(vertex);
        max_animation = new glm::vec3(vertex);
    }

    if (vertex.x > max_animation->x)
        max_animation->x = vertex.x;
    else if (vertex.x < min_animation->x)
        min_animation->x = vertex.x;

    if (vertex.y > max_animation->y)
        max_animation->y = vertex.y;
    else if (vertex.y < min_animation->y)
        min_animation->y = vertex.y;

    if (vertex.z > max_animation->z)
        max_animation->z = vertex.z;
    else if (vertex.z < min_animation->z)
        min_animation->z = vertex.z;


    min_max_flag++;
}

void BVH::preprocess_joint_frame(JOINT * parent_joint, unsigned int & frame_number)
{
    advance_joint_frame(parent_joint);

    glm::vec4 vertex_data_4d = parent_joint->matrix[3];

    compute_min_max(vertex_data_4d);

    parent_joint->animation_frames[frame_number] = vertex_data_4d;

    for (auto & child: parent_joint->children)
        preprocess_joint_frame(child, frame_number);
}

void BVH::advance_joint_frame(JOINT * joint)
{
    // we'll need index of motion data's array with start of this specific joint
    int start_index = motionData.current_frame() * motionData.num_motion_channels + joint->channel_start;

    // translate indetity matrix to this joint's offset parameters
    joint->matrix = glm::translate(glm::mat4(1.0),
                                   glm::vec3(joint->offset.x,
                                             joint->offset.y,
                                             joint->offset.z));

    // here we transform joint's local matrix with each specified channel's values
    // which are read from motion data
    for (unsigned int i = 0; i < joint->num_channels; i++)
    {
        // channel alias
        const short& channel = joint->channels_order[i];

        // extract value from motion data
        float value = motionData.data[start_index + i];
        
        if (channel & Xposition)
            joint->matrix = glm::translate(joint->matrix, glm::vec3(value, 0, 0));
        else if (channel & Yposition)
            joint->matrix = glm::translate(joint->matrix, glm::vec3(0, value, 0));
        else if (channel & Zposition)
            joint->matrix = glm::translate(joint->matrix, glm::vec3(0, 0, value));
        else if (channel & Xrotation)
            joint->matrix = glm::rotate(joint->matrix, value, glm::vec3(1, 0, 0));
        else if (channel & Yrotation)
            joint->matrix = glm::rotate(joint->matrix, value, glm::vec3(0, 1, 0));
        else if (channel & Zrotation)
            joint->matrix = glm::rotate(joint->matrix, value, glm::vec3(0, 0, 1));
    }

    // then we apply parent's local transfomation matrix to this joint's LTM (local tr. mtx. :)
    if(joint->parent != NULL)
        joint->matrix = joint->parent->matrix * joint->matrix;

    // when we have calculated parent's matrix do the same to all children
    for(auto& child : joint->children)
        advance_joint_frame(child);
}

string BVH::channel_index_to_string(short & i)
{
    string channel_name;

    switch(i) {
        case 0x01:
            channel_name = "Xposition";
            break;
        case 0x02:
            channel_name = "Yposition";
            break;
        case 0x04:
            channel_name = "Zposition";
            break;
        case 0x10:
            channel_name = "Zrotation";
            break;
        case 0x20:
            channel_name = "Xrotation";
            break;
        case 0x40:
            channel_name = "Yrotation";
            break;
    }

    return channel_name;
}

short BVH::channel_string_to_index(string & channel_name)
{
    if (trim(channel_name) == "Xposition")
        return 0x01;
    else if (trim(channel_name) == "Yposition")
        return 0x02;
    else if (trim(channel_name) == "Zposition")
        return 0x04;
    else if (trim(channel_name) == "Zrotation")
        return 0x10;
    else if (trim(channel_name) == "Xrotation")
        return 0x20;
    else if (trim(channel_name) == "Yrotation")
        return 0x40;
    else
        return 0;
}