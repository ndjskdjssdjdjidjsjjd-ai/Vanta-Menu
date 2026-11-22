#include "Cheats.hpp"
#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <fstream>
#include <filesystem>

using namespace geode::prelude;

namespace Cheats {
    std::string getMacroPath() {
        return Mod::get()->getSaveDir().string() + "/macros";
    }

    void updateMacroList() {
        macroList.clear();
        std::string path = getMacroPath();
        if (!std::filesystem::exists(path)) {
            std::filesystem::create_directories(path);
        }
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.path().extension() == ".json") {
                macroList.push_back(entry.path().stem().string());
            }
        }
    }

    void deleteMacro() {
        if (selectedMacro < 0 || selectedMacro >= macroList.size()) return;
        std::string path = getMacroPath() + "/" + macroList[selectedMacro] + ".json";
        std::filesystem::remove(path);
        selectedMacro = -1;
        updateMacroList();
    }

    void saveMacro() {
        if (macroEvents.empty()) return;
        std::string path = getMacroPath() + "/" + std::string(macroName) + ".json";
        std::ofstream file(path);
        if (!file.is_open()) return;

        file << "{\n";
        file << "  \"fps\": " << fps << ",\n";
        file << "  \"events\": [\n";
        for (size_t i = 0; i < macroEvents.size(); ++i) {
            const auto& e = macroEvents[i];
            file << "    {\"frame\": " << e.frame << ", \"down\": " << (e.down ? "true" : "false") 
                 << ", \"p2\": " << (e.p2 ? "true" : "false") << ", \"x\": " << e.x 
                 << ", \"y\": " << e.y << ", \"rot\": " << e.rot << ", \"yVel\": " << e.yVel << "}";
            if (i < macroEvents.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ]\n}\n";
        file.close();
        updateMacroList();
    }

    void loadMacro() {
        if (selectedMacro < 0 || selectedMacro >= macroList.size()) return;
        std::string path = getMacroPath() + "/" + macroList[selectedMacro] + ".json";
        std::ifstream file(path);
        if (!file.is_open()) return;

        macroEvents.clear();
        std::string line;
        int loadedFps = 60;
        while (std::getline(file, line)) {
            if (line.find("\"fps\":") != std::string::npos) {
                loadedFps = std::stoi(line.substr(line.find(":") + 1));
            }
            if (line.find("\"frame\":") != std::string::npos) {
                MacroEvent e;
                auto getValue = [&](std::string key) -> std::string {
                    size_t pos = line.find("\"" + key + "\":");
                    if (pos == std::string::npos) return "0";
                    pos += key.length() + 3;
                    size_t end = line.find(",", pos);
                    if (end == std::string::npos) end = line.find("}", pos);
                    std::string val = line.substr(pos, end - pos);
                    val.erase(0, val.find_first_not_of(" "));
                    val.erase(val.find_last_not_of(" ") + 1);
                    return val;
                };
                e.frame = std::stoi(getValue("frame"));
                e.down = getValue("down") == "true";
                e.p2 = getValue("p2") == "true";
                e.x = std::stof(getValue("x"));
                e.y = std::stof(getValue("y"));
                e.rot = std::stof(getValue("rot"));
                e.yVel = std::stod(getValue("yVel"));
                macroEvents.push_back(e);
            }
        }
        std::sort(macroEvents.begin(), macroEvents.end(), [](const MacroEvent& a, const MacroEvent& b) {
            return a.frame < b.frame;
        });
        fps = loadedFps;
        file.close();
    }

    void save() {
        auto mod = Mod::get();
        mod->setSavedValue("noclip", noclip);
        mod->setSavedValue("unlockIcons", unlockIcons);
        mod->setSavedValue("practiceMusic", practiceMusic);
        mod->setSavedValue("safeMode", safeMode);
        mod->setSavedValue("instantComplete", instantComplete);
        mod->setSavedValue("fps", fps);
        mod->setSavedValue("tps", tps);
        mod->setSavedValue("speed", speed);
    }

    void load() {
        auto mod = Mod::get();
        noclip = mod->getSavedValue<bool>("noclip", false);
        unlockIcons = mod->getSavedValue<bool>("unlockIcons", false);
        practiceMusic = mod->getSavedValue<bool>("practiceMusic", false);
        safeMode = mod->getSavedValue<bool>("safeMode", false);
        instantComplete = mod->getSavedValue<bool>("instantComplete", false);
        fps = mod->getSavedValue<int>("fps", 60);
        tps = mod->getSavedValue<int>("tps", 60);
        speed = mod->getSavedValue<float>("speed", 1.0f);

        if (fps != 60) {
            CCDirector::sharedDirector()->setAnimationInterval(1.0 / static_cast<double>(fps));
        }
        updateMacroList();
    }

    void draw() {
        bool changed = false;

        if (ImGui::BeginTable("SettingsTable", 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX)) {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Input", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed, 50.0f);

            auto drawSettingRow = [&](const char* label, const char* id, auto& value, auto drawInput) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(label);
                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (drawInput(id, value)) changed = true;
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button((std::string("SET##") + id).c_str(), ImVec2(-FLT_MIN, 0))) changed = true;
            };

            drawSettingRow("FPS", "fps", fps, [](const char* id, int& v) { return ImGui::InputInt(id, &v, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue); });
            drawSettingRow("TPS", "tps", tps, [](const char* id, int& v) { return ImGui::InputInt(id, &v, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue); });
            drawSettingRow("Speed", "speed", speed, [](const char* id, float& v) { return ImGui::InputFloat(id, &v, 0, 0, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue); });

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Checkbox("Unlock Icons", &unlockIcons)) changed = true;
        if (ImGui::Checkbox("Practice Music", &practiceMusic)) changed = true;
        if (ImGui::Checkbox("NoClip", &noclip)) changed = true;
        if (ImGui::Checkbox("Safe Mode", &safeMode)) changed = true;
        if (ImGui::Checkbox("Instant Complete", &instantComplete)) changed = true;

        if (changed) {
            save();
            if (fps > 0) {
                CCDirector::sharedDirector()->setAnimationInterval(1.0 / static_cast<double>(fps));
            }
        }
    }
}

class $modify(CheatScheduler, CCScheduler) {
    void update(float dt) {
        float targetDt = (1.0f / static_cast<float>(Cheats::tps)) * Cheats::speed;
        CCScheduler::update(targetDt);
    }
};

class $modify(CheatGameManager, GameManager) {
    bool isIconUnlocked(int iconID, IconType type) {
        if (Cheats::unlockIcons) return true;
        return GameManager::isIconUnlocked(iconID, type);
    }
    bool isColorUnlocked(int colorID, UnlockType type) {
        if (Cheats::unlockIcons) return true;
        return GameManager::isColorUnlocked(colorID, type);
    }
};

class $modify(CheatGameStatsManager, GameStatsManager) {
    bool isItemUnlocked(UnlockType p0, int p1) {
        if (p0 == UnlockType::GJItem && p1 == 17 && Cheats::practiceMusic) return true;
        return GameStatsManager::isItemUnlocked(p0, p1);
    }
};

class $modify(CheatPlayLayer, PlayLayer) {
    struct Fields {
        bool m_instantCompleteTriggered = false;
    };

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (Cheats::noclip) return;
        if (Cheats::isRecording) {
            Cheats::lastDeathFrame = this->m_gameState.m_currentProgress;
        }
        PlayLayer::destroyPlayer(player, object);
    }
    void levelComplete() {
        if (Cheats::safeMode) return;
        PlayLayer::levelComplete();
    }
    void update(float dt) {
        if (Cheats::instantComplete && !m_fields->m_instantCompleteTriggered) {
            if (this->m_level && !this->m_isPracticeMode && this->m_player1) {
                m_fields->m_instantCompleteTriggered = true;
                float levelLength = this->m_levelLength > 0 ? this->m_levelLength : 10000.0f;
                if (auto player = this->m_player1) {
                    player->setPosition({ levelLength, player->getPositionY() });
                }
                this->levelComplete();
            }
        }

        if (Cheats::pendingRestart) {
            Cheats::pendingRestart = false;
            if (Cheats::pendingRecord) {
                Cheats::macroEvents.clear();
                Cheats::lastDeathFrame = -1;
                Cheats::isRecording = true;
                Cheats::isPlaying = false;
                Cheats::pendingRecord = false;
            }
            if (Cheats::pendingPlay) {
                Cheats::macroFrame = 0;
                Cheats::isPlaying = true;
                Cheats::isRecording = false;
                Cheats::pendingPlay = false;
            }
            this->resetLevel();
            return;
        }

        PlayLayer::update(dt);
    }
    void resetLevel() {
        m_fields->m_instantCompleteTriggered = false;
        PlayLayer::resetLevel();
    }
};

class $modify(CheatBot, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (Cheats::isRecording) {
            Cheats::MacroEvent e;
            e.frame = this->m_gameState.m_currentProgress;
            e.down = down;
            e.p2 = !isPlayer1;
            if (auto p = isPlayer1 ? this->m_player1 : this->m_player2) {
                e.x = p->getPositionX();
                e.y = p->getPositionY();
                e.rot = p->getRotation();
                e.yVel = p->m_yVelocity;
            }

            // Si morimos y retrocedemos, borramos los eventos viejos desde el punto de muerte
            if (Cheats::lastDeathFrame != -1 && e.frame <= Cheats::lastDeathFrame) {
                Cheats::macroEvents.erase(
                    std::remove_if(Cheats::macroEvents.begin(), Cheats::macroEvents.end(),
                        [&](const Cheats::MacroEvent& ev) { return ev.frame >= e.frame; }),
                    Cheats::macroEvents.end()
                );
                Cheats::lastDeathFrame = -1;
            }

            auto it = std::find_if(Cheats::macroEvents.begin(), Cheats::macroEvents.end(), 
                [&](const Cheats::MacroEvent& ev) { return ev.frame == e.frame && ev.p2 == e.p2; });
            
            if (it != Cheats::macroEvents.end()) {
                *it = e;
            } else {
                Cheats::macroEvents.push_back(e);
            }
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void update(float dt) {
        if (Cheats::isPlaying && !Cheats::macroEvents.empty()) {
            while (Cheats::macroFrame < Cheats::macroEvents.size() && 
                   Cheats::macroEvents[Cheats::macroFrame].frame <= this->m_gameState.m_currentProgress) {
                
                auto& e = Cheats::macroEvents[Cheats::macroFrame];
                GJBaseGameLayer::handleButton(e.down, 1, !e.p2);
                Cheats::macroFrame++;
            }
            
            if (Cheats::macroFrame > 0) {
                int currentProgress = this->m_gameState.m_currentProgress;
                if (Cheats::macroEvents[Cheats::macroFrame - 1].frame > currentProgress + 1) {
                    for (int i = 0; i < Cheats::macroEvents.size(); i++) {
                        if (Cheats::macroEvents[i].frame >= currentProgress) {
                            Cheats::macroFrame = i;
                            break;
                        }
                    }
                }
            }
        }
        GJBaseGameLayer::update(dt);
    }
};
