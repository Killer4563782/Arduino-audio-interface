#pragma once 
#include <imgui.h>
#include "Audio/AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"
#include "Config/Configuration.hpp"

class SidebarUIHelper {
public:
    SidebarUIHelper()
    {
        SetWindowProperties();
    }

    void SetWindowProperties()
    {
        ImGui::SetNextWindowSize(ImVec2(200, 320));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(31, 31, 31, 255)));
    }

    void PopStyle()
    {
        ImGui::PopStyleColor();
    }

    void DrawChannelNode(int index, const std::string& channelName, ImGuiID& openNodeID, int& selectedChannel, std::map<UINT, std::set<std::string>>& cachedAppMap, std::vector<std::pair<UINT, std::vector<DWORD>>>& cachedAllAssigned, std::vector<std::string>& audioProcesses, bool& refreshSidebar)
    {
        std::string label = "| " + channelName + "##channel" + std::to_string(index);
        ImGuiID nodeID = ImGui::GetID(label.c_str());
        bool isOpen = (openNodeID == nodeID);

        ImGui::PushID(index);
        ImGui::AlignTextToFramePadding();

        bool clickedArrow = ImGui::ArrowButton("##arrow", isOpen ? ImGuiDir_Down : ImGuiDir_Right);
        ImGui::SameLine();
        bool clickedLabel = ImGui::Selectable(label.c_str(), isOpen);

        if (clickedArrow || clickedLabel)
            openNodeID = isOpen ? 0 : nodeID;

        if (openNodeID == nodeID)
        {
            selectedChannel = index;
            DrawAppList(index, cachedAppMap, cachedAllAssigned, audioProcesses, refreshSidebar);
            DrawAddAppButton();
        }

        ImGui::PopID();
    }

    void DrawAppList(int index, std::map<UINT, std::set<std::string>>& cachedAppMap, std::vector<std::pair<UINT, std::vector<DWORD>>>& cachedAllAssigned, std::vector<std::string>& audioProcesses, bool& refreshSidebar)
    {
        auto it = cachedAppMap.find(index);
        if (it != cachedAppMap.end() && !it->second.empty())
        {
            for (const auto& appName : it->second)
            {
                ImGui::PushID(appName.c_str());
                ImGui::Bullet();
                ImGui::SameLine();
                if (ImGui::Selectable(appName.c_str(), false))
                {
                    for (const auto& pair : cachedAllAssigned)
                    {
                        if (pair.first != static_cast<UINT>(index)) 
                            continue;

                        for (DWORD pid : pair.second)
                        {
                            if (Utility::GetMainProcessNameByPID(pid) == appName && g_VirtualCableManager)
                            {
                                g_VirtualCableManager->RemoveAppFromCable(pid, index);
                                g_Configuration->RemoveAppFromChannel(index, appName);
                                audioProcesses.emplace_back(appName);
                                refreshSidebar = true;
                                cachedAppMap[index].erase(appName);
                                break;
                            }
                        }
                    }
                }
                ImGui::PopID();
            }
        }
        else
        {
            ImGui::TextDisabled("No apps assigned.");
        }
        ImGui::Spacing();
    }

    void DrawAddAppButton()
    {
        if (ImGui::Button("+ Add App", ImVec2(100, 30)))
        {
            g_extend_window ? Utility::ResizeWindow(GetActiveWindow(), 770, 350) : Utility::ResizeWindow(GetActiveWindow(), 770 + 400, 350);
            g_extend_window = !g_extend_window;
        }
    }

    void UpdateWindowInfo(ImVec2& windowPos, ImVec2& windowSize)
    {
        windowPos = ImGui::GetWindowPos();
        windowSize = ImGui::GetWindowSize();
    }
private:
    static inline bool g_extend_window = false;
};