#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>
#include <iostream>

using namespace std;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    MA_ASSERT(pDevice->capture.format == pDevice->playback.format);
    MA_ASSERT(pDevice->capture.channels == pDevice->playback.channels);

    float buffer[frameCount];
    for(int i = 0; i < frameCount; ++i) {
        buffer[i] = ((float*)pInput)[i];
    }

    /* In this example the format and channel count are the same for both input and output which means we can just memcpy(). */
    MA_COPY_MEMORY(pOutput, buffer, frameCount * ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels));
}

int main() {
    ma_result result;
    ma_device_config deviceConfig;
    ma_device device;

    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.capture.pDeviceID  = NULL;
    deviceConfig.capture.format     = ma_format_f32;
    deviceConfig.capture.channels   = 1;
    deviceConfig.capture.shareMode  = ma_share_mode_shared;
    deviceConfig.playback.pDeviceID = NULL;
    deviceConfig.playback.format    = ma_format_f32;
    deviceConfig.playback.channels  = 1;
    deviceConfig.dataCallback       = data_callback;
    deviceConfig.sampleRate         = 48000;

    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        return result;
    }

    ma_device_start(&device);

    printf("Press Enter to quit...\n");
    getchar();

    ma_device_uninit(&device);
    return 0;
}