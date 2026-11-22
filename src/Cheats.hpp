#pragma once
#include <imgui.h>
#include <vector>
#include <string>

namespace Cheats {
    inline bool noclip = false;
    inline bool unlockIcons = false;
    inline bool practiceMusic = false;
    inline bool safeMode = false;
    inline bool instantComplete = false;
    inline int fps = 60;
    inline int tps = 60;
    inline float speed = 1.0f;

    struct MacroEvent {
        bool down;
        bool p2;
        int frame;
        float x;
        float y;
        float rot;
        double yVel;
    };

    inline std::vector<MacroEvent> macroEvents;
    inline bool isRecording = false;
    inline bool isPlaying = false;
    inline int macroFrame = 0;
    inline char macroName[64] = "my_macro";

    inline std::vector<std::string> macroList;
    inline int selectedMacro = -1;

    inline int lastDeathFrame = -1;
    inline bool pendingRestart = false;
    inline bool pendingRecord = false;
    inline bool pendingPlay = false;

    void draw();
    void load();
    void save();
    
    void saveMacro();
    void loadMacro();
    void deleteMacro();
    void updateMacroList();
}
