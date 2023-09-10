#include "module.cpp"
#include "macros.h"
#include <raymath.h>

using namespace std;

typedef struct InbuiltNode {
    ma_frame (*_func)(Module*,ma_frame,int,u32);
} InbuiltNode;

ma_frame node_volume(Module * module, ma_frame frame, int index, u32 frame_count){
    return (frame * module->settings[0]._value + 1) / 126;
}
ma_frame node_display(Module * module, ma_frame frame, int index, u32 frame_count){
    if(index==0) {
        module->buffer[998] = module->buffer[999];
        module->buffer[999] = 0;
    }
    if(abs(frame) > module->buffer[999]) module->buffer[999] = abs(frame);
    module->buffer[(int)(index / ((float)frame_count / 998.0f))] = frame;
    return frame;
}

int polar = 0;
ma_frame node_distortion(Module * module, ma_frame frame, int index, u32 frame_count){
    polar = frame > 0 ? 1 : -1;
    // Apply distortion
    frame *= polar;
    if(frame < (int)module->settings[0]._value * 5)
        return 0;

    if(frame < module->settings[1]._value * 10) {
        frame = Lerp(frame, (int)module->settings[1]._value * 10, module->settings[2].value());
    }
    else
        frame = (int)module->settings[1]._value * 10;
    return frame * polar;
}