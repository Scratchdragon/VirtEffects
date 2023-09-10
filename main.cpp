#include <iostream>
#include <raylib.h>
#include <raymath.h> 
#include <stdio.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "macros.h"

#include "module.cpp"
#include "colors.h"
#include "loader.cpp"

using namespace std;

Vector2 window;
Vector2 mousePos;

vector<string> modTypes;
Module modules[GRID_SIZE * GRID_SIZE];
int selected = -1;
bool selectingOut = false;

short inputNode = -1;
short outputNode = -1;

// Miniaudio data callback
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    // Early return if no input node or if user is busy selecting
    if(inputNode == -1 || !modules[inputNode].enabled) return;
    
    // Assert ma device properties
    MA_ASSERT(pDevice->capture.format == pDevice->playback.format);
    MA_ASSERT(pDevice->capture.channels == pDevice->playback.channels);

    ma_frame * audio = (ma_frame *)pInput;

    bool output = false;
    for(unsigned int i = 0; i < frameCount; ++i) {
        // Follow the node path
        short node = modules[inputNode].out;
        while(node != -1 && node != outputNode) {
            if(node > GRID_SIZE*GRID_SIZE || node < 0) return;
            if(modules[node].enabled && modules[node].processable)
                audio[i] = modules[node].ProcessFrame(audio[i], i, frameCount);
            node = modules[node].out;
        }
        output = node == outputNode;
    }

    // Copy the input into the output device
    if(output && outputNode != -1 && modules[outputNode].enabled)
        MA_COPY_MEMORY(pOutput, audio, frameCount * ma_get_bytes_per_frame(pDevice->capture.format, pDevice->capture.channels));
}

void Render() {
    float margin = window.y / 20.0;

    // Outline boundries
    Rectangle boardRect = {
        margin, margin,
        window.y - margin * 2, window.y - margin * 2
    };
    DrawRectangleLines(boardRect.x - 1, boardRect.y - 1, boardRect.width + 2, boardRect.height + 2, COLOR_OUTLINE);
    Rectangle editorRect = {
        window.y, margin, 
        window.x - window.y - margin, window.y - margin * 2
    };
    DrawRectangleLines(editorRect.x, editorRect.y, editorRect.width, editorRect.height, COLOR_OUTLINE);

    // Draw and update module connections
    for(int i = 0; i < GRID_SIZE * GRID_SIZE; ++i) {
        Module * mod = &modules[i];
        if(mod->isNull) continue;

        Vector2 pos = mod->position;
        pos.x *= (boardRect.width / GRID_SIZE);
        pos.y *= (boardRect.width / GRID_SIZE);
        pos.x += margin + boardRect.width / GRID_SIZE / 2;
        pos.y += margin + boardRect.width / GRID_SIZE / 2;

        Vector2 outPos = {-1, -1};
        if(mod->out != -1 && !modules[mod->out].isNull) {
            outPos = modules[mod->out].position;
            outPos.x *= (boardRect.width / GRID_SIZE);
            outPos.y *= (boardRect.width / GRID_SIZE);
            outPos.x += margin + boardRect.width / GRID_SIZE / 2;
            outPos.y += margin + boardRect.width / GRID_SIZE / 2;
        }

        // Don't bother with linking if module is output
        if(mod->title == "Output") continue;

        // Module linking
        if((selected == i && mod->hover && !selectingOut) || mod->settingOut) {
            if(IsMouseButtonDown(0)) {
                if(!selectingOut)
                    mod->out = -1;
                else if(mod->out != -1) {
                    DrawLineEx(outPos, pos, 2, COLOR_OUTLINE_DARK);
                    continue;
                }
                selectingOut = true;
                mod->settingOut = true;

                // Make sure mouse is not out of bounds
                if(
                    mousePos.x > boardRect.x && 
                    mousePos.x < boardRect.x + boardRect.width && 
                    mousePos.y > boardRect.y && 
                    mousePos.y < boardRect.y + boardRect.height
                )
                    outPos = mousePos;
            }
            else {
                mod->settingOut = false;
                selectingOut = false;
            }
        }
        
        // Draw connection
        if(outPos.x != -1)
            DrawLineEx(outPos, pos, 2, COLOR_OUTLINE_DARK);
    }

    // Draw modules
    for(int x = 0; x < GRID_SIZE; ++x) {
        for(int y = 0; y < GRID_SIZE; ++y) {
            int drawx = margin + x * (boardRect.width / GRID_SIZE);
            int drawy = margin + y * (boardRect.width / GRID_SIZE);

            int moduleIndex = y * GRID_SIZE + x;

            // Calculate the draw rectangle
            Rectangle drawRect = {drawx + 4.0f, drawy + 4.0f, boardRect.width / GRID_SIZE - 8.0f, boardRect.width / GRID_SIZE - 8.0f};

            // Check if mouse is over
            bool hover = mousePos.x > drawx && mousePos.x < drawx + boardRect.width / GRID_SIZE && mousePos.y > drawy && mousePos.y < drawy + boardRect.height / GRID_SIZE; 

            // Check if module is being selected/deselected
            if(hover && IsMouseButtonReleased(0))
                selected = moduleIndex;

            if(modules[moduleIndex].isNull) {
                Color color = COLOR_BG_LIGHT;
                color.a = hover ? 75 : 0;
                DrawRectangleRounded(drawRect, 0.2, 10, color);
                if(selected == moduleIndex)
                    DrawRectangleRoundedLines(drawRect, 0.2, 10, 2, COLOR_BG_LIGHT);
                continue;
            }

            // Get the module
            Module * mod = &modules[moduleIndex];

            // Update the module variables
            mod->position = {(float)x, (float)y};
            mod->hover = hover;
            if(hover && selectingOut && !mod->settingOut && selected != mod->out && mod->title != "Input") {
                modules[selected].out = moduleIndex;
            } 

            // Shrink the rectangle and draw the module box
            drawRect.x += margin / 4.0f;
            drawRect.y += margin / 4.0f;
            drawRect.width -= margin / 2.0f;
            drawRect.height -= margin / 2.0f;

            // Draw the module
            float roundness = mod->title == "Input" || mod->title == "Output" || mod->title == "Display" ? 1 : 0.3f;
            DrawRectangleRounded(drawRect, roundness, 10, hover ? COLOR_BG_LIGHT : COLOR_BG_DARK);
            DrawRectangleRoundedLines(drawRect, roundness, 10, 2, selected == moduleIndex ? COLOR_OUTLINE : COLOR_OUTLINE_DARK);
            
            // Draw symbol
            int width = MeasureText(mod->symbol, drawRect.height - margin) / 2;
            DrawText(mod->symbol, drawRect.x + drawRect.width / 2.0f - width, drawRect.y + margin / 2, drawRect.height - margin, COLOR_OUTLINE);
        }
    }

    // Draw the editor
    if(selected == -1) return;

    // Get the module
    Module * mod = &modules[selected];

    if(mod->isNull) {
        int i = 0;
        for(string file : modTypes) {
            int count = 0;
            const char ** split = TextSplit(file.c_str(), '/', &count);
            string item = split[count - 1];
            split = TextSplit(item.c_str(), '.', &count);
            item = split[0];

            // Only 1 of each input/output allowed
            if(item == "input" && inputNode != -1) continue;
            if(item == "output" && outputNode != -1) continue;

            Rectangle rect = {editorRect.x, editorRect.y + i * margin * 2, editorRect.width, margin * 2};
            if(mousePos.x > rect.x && mousePos.x < rect.x + rect.width && mousePos.y > rect.y && mousePos.y < rect.y + rect.height) {
                DrawRectangleRec(rect, COLOR_BG_LIGHT);
                if(IsMouseButtonPressed(0)) {
                    // Create the new module
                    modules[selected] = LoadModule(file);
                    return;
                }
            }
            else
                DrawRectangleRec(rect, COLOR_BG_DARK);
            DrawRectangleLinesEx(rect, 1, COLOR_OUTLINE);

            // Draw symbol
            int width = MeasureText(item.c_str(), margin * 1.3f) / 2;
            DrawText(item.c_str(), editorRect.x + editorRect.width / 2 - width, editorRect.y + i * margin * 2 + margin / 2.6f, margin * 1.3f, COLOR_OUTLINE);
            ++i;
        }
        return;
    }

    // Draw the delete button
    Rectangle deleteRect = {
        editorRect.x + editorRect.width - margin - 2, editorRect.y + 1,
        margin, margin
    };
    if(mousePos.x > deleteRect.x && mousePos.x < deleteRect.x + deleteRect.width && mousePos.y > deleteRect.y && mousePos.y < deleteRect.y + deleteRect.height) {
        // Highlight the box
        DrawRectangleRec(deleteRect, COLOR_BG_LIGHT);

        // Draw tooltip
        string text = "Delete";
        int width = MeasureText(text.c_str(), editorRect.width / 10.0f);
        DrawText(text.c_str(), editorRect.x + editorRect.width - width - margin / 5, editorRect.y + editorRect.height - editorRect.width / 10.0f, editorRect.width / 10.0f, COLOR_OUTLINE);
    
        // Delete functionality
        if(IsMouseButtonPressed(0)) {
            mod->isNull = true;
            mod->out = -1;
            for(int i = 0; i < GRID_SIZE * GRID_SIZE; ++i) {
                if(mod->out == selected) mod->out = -1;
            }
        }
    }
    // Draw the 'x'
    DrawText("x", deleteRect.x + margin / 4, deleteRect.y, margin, COLOR_OUTLINE);
    DrawRectangleRoundedLines(deleteRect, 0, 0, 1, COLOR_OUTLINE);

    // Draw and update the on/off button
    if(Vector2Distance(mousePos, (Vector2){editorRect.x + editorRect.width / 2, editorRect.y + editorRect.height - margin * 3}) < margin) {
        DrawCircle(editorRect.x + editorRect.width / 2, editorRect.y + editorRect.height - margin * 3, margin, COLOR_OUTLINE_LIGHT);
        if(IsMouseButtonPressed(0))
            mod->enabled = !mod->enabled;
        
        // Draw status text
        string text = mod->enabled ? "Enabled" : "Disabled";
        int width = MeasureText(text.c_str(), editorRect.width / 10.0f);
        DrawText(text.c_str(), editorRect.x + editorRect.width - width - margin / 5, editorRect.y + editorRect.height - editorRect.width / 10.0f, editorRect.width / 10.0f, COLOR_OUTLINE);
    } 
    else
        DrawCircle(editorRect.x + editorRect.width / 2, editorRect.y + editorRect.height - margin * 3, margin, COLOR_OUTLINE);
    DrawCircle(editorRect.x + editorRect.width / 2, editorRect.y + editorRect.height - margin * 3, margin - 2, mod->enabled ? COLOR_OUTLINE_DARK : COLOR_BG_LIGHT);

    // Draw title
    int width = MeasureText(mod->title.c_str(), editorRect.width / 8.0f);
    DrawText(mod->title.c_str(), editorRect.x + editorRect.width / 2.0f - width / 2.0f, editorRect.y + margin, editorRect.width / 8.0f, COLOR_OUTLINE);
    
    // Handle display
    if(mod->title == "Display") {
        Rectangle rect = {
            editorRect.x + margin,
            editorRect.y + editorRect.height / 2 - margin * 3.5f,
            editorRect.width - margin * 2,
            margin * 5
        };

        DrawRectangleRec(rect, COLOR_BG_DARK);
        
        float max = 2000;
        float oldy = rect.height / 2 + (rect.height / 2) * (mod->buffer[0] / max);
        for(int i = 1; i < 999; ++i) {
            Vector2 pos = {
                i * (rect.width / 999.0f),
                rect.height / 2 + (rect.height / 2) * (mod->buffer[i] / max)
            };

            DrawLineEx(
                (Vector2){(i - 1) * (rect.width / 999.0f) + rect.x, oldy + rect.y}, 
                (Vector2){pos.x + rect.x, pos.y + rect.y}, 
                1, 
                COLOR_OUTLINE_LIGHT
            );

            oldy = pos.y;
        }
        
        DrawRectangleLinesEx(rect, 1, COLOR_OUTLINE);
    }

    // Draw all the settings
    int sliderNum = 0;
    int switchNum = 0;

    float sliderShelfY = editorRect.y + margin * 3.0f;
    float switchShelfY = editorRect.y + margin + editorRect.height / 2.0f;

    for(int i = 0; i < mod->settings.size(); ++i) {
        Setting * setting = &mod->settings[i];

        if(setting->type == SLIDER) {
            int x = editorRect.x + margin * 2 + sliderNum * (editorRect.width - margin * 4) / 2.0f;
            DrawRectangle(x - margin / 2, sliderShelfY, margin, switchShelfY - sliderShelfY - margin, COLOR_BG_DARK);
            int sliderY = sliderShelfY + (switchShelfY - sliderShelfY - margin) * (1 - setting->value()) - margin / 6.0f;

            if(mousePos.y > sliderShelfY - margin / 2 && mousePos.y < switchShelfY - margin / 2 && mousePos.x > x - margin / 2 - 3 && mousePos.x < x + margin / 2 + 3) {
                DrawRectangle(x - margin / 2 - 3, sliderY, margin + 6, margin / 3.0f, COLOR_OUTLINE);
                if(IsMouseButtonDown(0)) {
                    float val = 1 - (mousePos.y - sliderShelfY) / (switchShelfY - sliderShelfY - margin);
                    if(val > 1) setting->_value = 255;
                    else if(val < 0) setting->_value = 0;
                    else setting->_value = (char)(val * 255);
                }

                // Draw slider name
                string text = setting->name + " (" + to_string((int)round(setting->_value / 2.55f)) + ")";
                width = MeasureText(text.c_str(), editorRect.width / 10.0f);
                DrawText(text.c_str(), editorRect.x + editorRect.width - width - margin / 5, editorRect.y + editorRect.height - editorRect.width / 10.0f, editorRect.width / 10.0f, COLOR_OUTLINE);
            }
            else {
                DrawRectangle(x - margin / 2 - 3, sliderY, margin + 6, margin / 3.0f, COLOR_OUTLINE_DARK);
            }

            ++sliderNum;
            continue;
        }

        if(setting->type == SWITCH) {
            int x = editorRect.x + margin * 2 + switchNum * (editorRect.width - margin * 4) / 2.0f;
            
            // Calculate switch bounds
            Rectangle switchRect = {
                x - margin * 0.7f, switchShelfY,
                margin * 1.4f, margin * 2.8f
            };

            // Check if mouse over
            bool hover = mousePos.x > switchRect.x && mousePos.x < switchRect.x + switchRect.width && mousePos.y > switchRect.y && mousePos.y < switchRect.y + switchRect.height;

            // Draw switch background
            DrawRectangleRounded(switchRect, 0.5, 10, COLOR_BG_DARK);

            // Draw switch tab
            DrawRectangleRounded(
                {
                    switchRect.x + 2, switchRect.y + (setting->value() > 0.5 ? 4 : switchRect.height / 2),
                    switchRect.width - 4, switchRect.height / 2 - 4
                },
                0.5,
                10,
                hover ? COLOR_OUTLINE : COLOR_OUTLINE_DARK
            );

            if(hover) {
                // Switch logic
                if(IsMouseButtonPressed(0))
                    setting->_value = !setting->_value;

                // Draw switch name
                string text = setting->name + " (" + (string)(setting->_value ? "On" : "Off") + ")";
                width = MeasureText(text.c_str(), editorRect.width / 10.0f);
                DrawText(text.c_str(), editorRect.x + editorRect.width - width - margin / 5, editorRect.y + editorRect.height - editorRect.width / 10.0f, editorRect.width / 10.0f, COLOR_OUTLINE);
            }

            ++switchNum;
            continue;
        }
    }
}

int main() {
    // Setup miniaudio
    ma_result result;
    ma_device_config deviceConfig;
    ma_device device;

    // Set device config
    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.capture.pDeviceID  = NULL;
    deviceConfig.capture.format     = MA_FORMAT;
    deviceConfig.capture.channels   = 1;
    deviceConfig.capture.shareMode  = ma_share_mode_shared;
    deviceConfig.playback.pDeviceID = NULL;
    deviceConfig.playback.format    = MA_FORMAT;
    deviceConfig.playback.channels  = 1;
    deviceConfig.dataCallback       = data_callback;
    deviceConfig.sampleRate         = MA_RATE;

    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        return result;
    }

    // Start the miniaudio device
    ma_device_start(&device);

    // Init raylib and open the window
    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_INTERLACED_HINT);
    InitWindow(700, 400, "VirtEffects");

    int monitor = GetCurrentMonitor();
    window = {
        (float)GetMonitorWidth(monitor),
        (float)GetMonitorHeight(monitor)
    };
    SetWindowSize(window.x, window.y);
    
    // Load module types
    FilePathList modDir = LoadDirectoryFiles("modules");
    vector<string> files = Stringify(modDir.paths, modDir.count);
    for(string file : files) {
        int count = 0;
        const char ** split = TextSplit(file.c_str(), '.', &count);
        if(count >= 2 && (string)split[count - 1] == "layout") {
            modTypes.push_back(file);
            cout << "Found module layout '" << file << "'\n";
        }
    }

    // Set the log level to error to avoid excessive logging
    SetTraceLogLevel(LOG_ERROR);

    int tick = 0;
    while(!WindowShouldClose()) {
        window = {
            (float)GetRenderWidth(),
            (float)GetRenderHeight()
        };
        
        mousePos = GetMousePosition();

        // Do every 1000 iterations to reduce on frame time
        ++tick;
        if(tick == 1000) {
            tick = 0;

            // Check if input and/or output has been placed
            inputNode = -1;
            outputNode = -1;
            for(short i = 0; i < GRID_SIZE * GRID_SIZE; ++i) {
                if(modules[i].isNull) continue;
                if(modules[i].title == "Input") inputNode = i;
                if(modules[i].title == "Output") outputNode = i;
            }
        }

        BeginDrawing();
        {
            ClearBackground(COLOR_BACKGROUND);
            Render();
        }
        EndDrawing();
    }
    CloseWindow();

    // Close miniaudio
    ma_device_uninit(&device);
}