#pragma once

// Include miniaudio
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <iostream>

using namespace std;

namespace EzAudio {
    ma_engine _maEngine;
    ma_context _maContext;

    bool started = false;

    // Starts the miniaudio backend and initialises EzAudio
    int Start(ma_engine_config * config = NULL) {
        started = false;
        ma_engine_config conf;

        // Attempt to start the miniaudio engine
        ma_result result;
        result = ma_engine_init(config, &_maEngine);
        if(result != MA_SUCCESS) return result;
        result = ma_context_init(NULL, 0, NULL, &_maContext);
        if(result != MA_SUCCESS) return result;

        started = true;
        return 0;
    }

    // Stop the EzAudio system
    void Close() {
        if(!started) return;
        started = false;
        ma_engine_uninit(&_maEngine);
        ma_context_uninit(&_maContext);
    }
};