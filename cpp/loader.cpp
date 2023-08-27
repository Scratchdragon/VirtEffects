#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <stdio.h>

#include "module.cpp"

using namespace std;

vector<string> Stringify(const char ** list, int size) {
    vector<string> vec;
    for(int i = 0; i < size; ++i) {
        vec.push_back((string)list[i]);
    }
    return vec;
}
vector<string> Stringify(char ** list, int size) {
    vector<string> vec;
    for(int i = 0; i < size; ++i) {
        vec.push_back((string)list[i]);
    }
    return vec;
}

vector<string> TextSplitVec(string text, char del) {
    int count = 0;
    const char ** raw = TextSplit(text.c_str(), del, &count);
    return Stringify(raw, count);
}

Module LoadModule(string file) {
    string filetext = (string)LoadFileText(file.c_str());
    
    vector<string> tokens = TextSplitVec(filetext, '\n');

    Module module = Module("_module_", (char*)"?");

    for(string line : tokens) {
        if(line[0] == '#' || line == "") continue;

        vector<string> _args = TextSplitVec(line, ' ');
        vector<string> args;
        for(string arg : _args) {
            if(arg != "") args.push_back(arg);
        }

        string opt = args[0];

        if(opt == "title")
            module.title = args[1];
        if(opt == "symbol") {
            module.symbol[0] = args[1][0];
            module.symbol[1] = args[1][1];
        }
        if(opt == "slider") {
            module.settings.push_back(
                (Setting){
                    args[1],
                    SLIDER,
                    stof(args[2])
                }
            );
        }
        if(opt == "switch") {
            module.settings.push_back(
                (Setting){
                    args[1],
                    SWITCH,
                    stof(args[2])
                }
            );
        }
    }
    return module;
}