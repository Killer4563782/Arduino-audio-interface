#include "Interface.hpp"
#include "Audio/AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"

void AudioMixerUI::RenderProcesslist()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(31, 31, 31, 255)));
    ImGui::SetNextWindowPos(ImVec2(m_mixer_window_pos.x + m_mixer_window_size.x, m_mixer_window_pos.y));
    ImGui::SetNextWindowSize(ImVec2(400, m_mixer_window_size.y));

    static std::unordered_map<std::string, std::vector<DWORD>> processPidMap;

    if (ImGui::Begin("Processlist", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
    {
        if (ImGui::Button("Refresh", ImVec2(380, 30)))
        {
            auto fresh_list = Utility::GetAudioOutputProcesses();
            processPidMap.clear();
            audioProcesses.clear();
            cachedAudioProcessSet.clear();

            for (const auto& process : fresh_list)
            {
                if (std::find(selectedProcesses.begin(), selectedProcesses.end(), process) == selectedProcesses.end())
                {
                    std::wstring process_name(process.begin(), process.end());
                    auto pids = Utility::FindProcessIds(process_name);
                    if (!pids.empty())
                    {
                        processPidMap[process] = pids;
                        cachedAudioProcessSet.insert(process);
                        audioProcesses.push_back(process);
                    }
                }
            }
            refreshSidebar = true;
        }

        for (size_t i = 0; i < audioProcesses.size(); ++i)
        {
            if (ImGui::Selectable(audioProcesses[i].c_str(), false))
            {
                const std::string& name = audioProcesses[i];
                selectedProcesses.push_back(name);

                auto it = processPidMap.find(name);
                if (it != processPidMap.end())
                {
                    for (DWORD pid : it->second)
                    {
                        g_VirtualCableManager->AssignAppToCable(pid, m_selected_channel);
                    }
                }

                cachedAudioProcessSet.erase(name);
                audioProcesses.erase(audioProcesses.begin() + i);
                processPidMap.erase(name);
                refreshSidebar = true;
                --i; 
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();

    if (refreshSidebar)
    {
        cachedAppMap = g_VirtualCableManager->GetAssignedAppNamesPerCable();
        cachedAllAssigned = g_VirtualCableManager->GetAllAssignedApps();
        refreshSidebar = false;
    }
}