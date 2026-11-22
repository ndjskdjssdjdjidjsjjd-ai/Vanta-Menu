#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <imgui-cocos.hpp>
#include <cstdio>
#include <filesystem>
#include "Cheats.hpp"

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

bool isMenuOpen = false;
int currentTab = 0;

static void setupStyle() {
    static bool applied = false;
    if (applied) return;

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_WindowBg]       = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
    colors[ImGuiCol_TitleBg]        = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_Button]         = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.00f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.00f, 0.50f, 0.70f, 1.00f);
    colors[ImGuiCol_CheckMark]      = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBg]        = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);

    style.WindowRounding    = 6.0f;
    style.FrameRounding     = 4.0f;
    style.ItemSpacing       = ImVec2(10.0f, 8.0f);

    ImGuiIO& io = ImGui::GetIO();
    bool loadedFont = false;

#ifdef GEODE_IS_WINDOWS
    char winDir[MAX_PATH];
    if (GetWindowsDirectoryA(winDir, MAX_PATH)) {
        std::string fontPath = std::string(winDir) + "\\Fonts\\arial.ttf";
        if (std::filesystem::exists(fontPath)) {
            io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 18.0f);
            loadedFont = true;
        }
    }
#endif

    if (!loadedFont) {
        io.Fonts->AddFontDefault();
    }

    applied = true;
}

$on_mod(Loaded) {
    Cheats::load();

    ImGuiCocos::get().setup([] {
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }).draw([] {
        setupStyle();

        if (ImGui::IsKeyPressed(ImGuiKey_Tab, false)) {
            isMenuOpen = !isMenuOpen;
        }

        if (isMenuOpen) {
            ImVec2 display = ImGui::GetIO().DisplaySize;
            float w = std::min(560.0f, display.x * 0.9f);
            float h = std::min(420.0f, display.y * 0.85f);
            ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);

            if (ImGui::Begin("VantaMenu", &isMenuOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
                float sidebarW = std::min(130.0f, w * 0.25f);
                ImGui::BeginChild("Sidebar", ImVec2(sidebarW, 0), true);
                {
                    if (ImGui::Selectable("About", currentTab == 0)) currentTab = 0;
                    if (ImGui::Selectable("Main", currentTab == 1)) currentTab = 1;
                    if (ImGui::Selectable("Bot", currentTab == 2)) currentTab = 2;
                }
                ImGui::EndChild();

                ImGui::SameLine();

                ImGui::BeginChild("Content", ImVec2(0, 0), false);
                {
                    if (currentTab == 0) {
                        ImGui::Spacing();
                        ImGui::Text("VantaMenu v1.0");
                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::TextWrapped("VantaMenu is a free mod menu for geometry dash & open source code :)");
                    } else if (currentTab == 1) {
                        Cheats::draw();
                    } else if (currentTab == 2) {
                        ImGui::Spacing();

                        if (ImGui::Checkbox("Record Macro", &Cheats::isRecording)) {
                            if (Cheats::isRecording) {
                                Cheats::pendingRestart = true;
                                Cheats::pendingRecord = true;
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Save Macro")) {
                            Cheats::saveMacro();
                        }

                        ImGui::Spacing();
                        if (ImGui::Checkbox("Play Macro", &Cheats::isPlaying)) {
                            if (Cheats::isPlaying) {
                                Cheats::pendingRestart = true;
                                Cheats::pendingPlay = true;
                            }
                        }
                        if (Cheats::isPlaying) {
                            ImGui::SameLine();
                            ImGui::Text("Frame: %d / %d", Cheats::macroFrame, (int)Cheats::macroEvents.size());
                        }

                        ImGui::Spacing();
                        ImGui::InputText("Macro Name", Cheats::macroName, IM_ARRAYSIZE(Cheats::macroName));

                        ImGui::Spacing();
                        if (ImGui::Button("Refresh List")) {
                            Cheats::updateMacroList();
                        }

                        if (ImGui::BeginListBox("##MacroList", ImVec2(-FLT_MIN, 100))) {
                            for (int i = 0; i < (int)Cheats::macroList.size(); i++) {
                                const bool is_selected = (Cheats::selectedMacro == i);
                                if (ImGui::Selectable(Cheats::macroList[i].c_str(), is_selected)) {
                                    Cheats::selectedMacro = i;
                                    std::snprintf(Cheats::macroName, sizeof(Cheats::macroName), "%s", Cheats::macroList[i].c_str());
                                }
                                if (is_selected) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndListBox();
                        }

                        if (ImGui::Button("Load Selected")) {
                            Cheats::loadMacro();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Delete Selected")) {
                            Cheats::deleteMacro();
                        }
                    }
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }
    });
}

class $modify(VantaKeyboardHook, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double timestamp) {
        if (key == KEY_Tab && isKeyDown && !isKeyRepeat) {
            isMenuOpen = !isMenuOpen;
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, timestamp);
    }
};
