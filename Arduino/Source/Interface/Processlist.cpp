#include "Interface.hpp"
#include "Audio/AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"
#include "Config/Configuration.hpp"

void AudioMixerUI::RenderProcesslist()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(31, 31, 31, 255)));
    ImGui::SetNextWindowPos(ImVec2(m_mixer_window_pos.x + m_mixer_window_size.x, 0));
    ImGui::SetNextWindowSize(ImVec2(400, m_mixer_window_size.y + 100));

    static std::unordered_map<std::string, std::vector<DWORD>> processPidMap;

    if (ImGui::Begin("Processlist", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
    {
        if (ImGui::Button("Refresh", ImVec2(380, 30)))
        {
            auto fresh_list = Utility::GetAudioOutputProcesses();
            processPidMap.clear();
            audioProcesses.clear();
            cachedAudioProcessSet.clear();

            std::set<std::string> assignedApps;
            for (int i = 0; i < MAX_CABLES; ++i)
            {
                auto configApps = g_Configuration->GetChannelApps(i);
                assignedApps.insert(configApps.begin(), configApps.end());
                auto it = cachedAppMap.find(i);
                if (it != cachedAppMap.end())
                {
                    assignedApps.insert(it->second.begin(), it->second.end());
                }
            }

            for (const auto& process : fresh_list)
            {
                if (std::find(selectedProcesses.begin(), selectedProcesses.end(), process) == selectedProcesses.end() && assignedApps.find(process) == assignedApps.end())
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

            if (g_VirtualCableManager)
            {
                g_Configuration->SyncWithVirtualCableManager();
                refreshSidebar = true;
            }
        }

        for (size_t i = 0; i < audioProcesses.size(); ++i)
        {
            if (ImGui::Selectable(audioProcesses[i].c_str(), false))
            {
                const std::string& name = audioProcesses[i];
                selectedProcesses.push_back(name);

                auto it = processPidMap.find(name);
                if (it != processPidMap.end() && g_VirtualCableManager)
                {
                    for (DWORD pid : it->second)
                    {
                        g_VirtualCableManager->AssignAppToCable(pid, m_selected_channel);
                    }
                    cachedAppMap[m_selected_channel].insert(name);
                }

                g_Configuration->AssignAppToChannel(m_selected_channel, name);
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
}

void AudioMixerUI::ApplyConfig()
{
    if (!g_VirtualCableManager)
    {
        std::cerr << "VirtualCableManager not initialized" << std::endl;
        return;
    }

    cachedAppMap.clear();
    for (int i = 0; i < MAX_CABLES; ++i)
    {
        m_volumes[i] = g_Configuration->GetChannelVolume(i);
        if (FAILED(g_VirtualCableManager->SetCableVolume(i, m_volumes[i])))
        {
            std::cerr << "Failed to set volume for channel " << i << ": " << m_volumes[i] << std::endl;
        }

        auto apps = g_Configuration->GetChannelApps(i);
        for (const auto& app : apps)
        {
            std::wstring appW(app.begin(), app.end());
            auto pids = Utility::FindProcessIds(appW);
            if (!pids.empty())
            {
                for (DWORD pid : pids)
                {
                    if (SUCCEEDED(g_VirtualCableManager->AssignAppToCable(pid, i)))
                    {
                        cachedAppMap[i].insert(app);
                    }
                    else
                    {
                        std::cerr << "Failed to assign app " << app << " to channel " << i << " with PID " << pid << std::endl;
                    }
                }
            }
            else
            {
                cachedAppMap[i].insert(app);
                std::cerr << "No PIDs found for app " << app << " on channel " << i << std::endl;
            }
        }
    }
    refreshSidebar = true;
}