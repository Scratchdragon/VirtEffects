#pragma once

#include <iostream>
#include <vector>
#include <raylib.h>

using namespace std;

enum {
    SLIDER,
    SWITCH
} type;

typedef struct Setting {
    string name;
    const int type;
    float value;
} Setting;

class Module {
    public:
    bool isNull = true; // If the module has been created or not
    bool grabbed = false; // If the module is currently grabbed by the user
    bool hover = false; // If the user is hovering over
    bool settingOut = false; // If the user is currently selecting an output
    bool enabled = true; // If the module is active

    int out; // The indecies of the output module

    Vector2 position; // The decimal position of the module

    string title; // Module name
    char symbol[2]; // The symbol to be displayed in the board (max of 2 chars)

    vector<Setting> settings; // Different inputs to be displayed on the editor

    Module(string title, char symbol[2]) {
        this->title = title;
        this->symbol[0] = symbol[0];
        this->symbol[1] = symbol[1];
        this->isNull = false;
        this->out = -1;
    }

    Module() {}
};