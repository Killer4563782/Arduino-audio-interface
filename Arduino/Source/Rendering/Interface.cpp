#include "Interface.hpp"
#include "Rendering/Rendering.hpp"
#include "Utility/Utility.hpp"
#include "Threading/Threading.hpp"

#define ICON_FA_MICROPHONE "\xef\x84\xb0" 
#define ICON_FA_MICROPHONE_SLASH "\xef\x84\xb1" 

void AudioMixerUI::RenderSlider()
{
    ImGui::SetNextWindowPos(ImVec2(m_window_pos.x + m_window_size.x, m_window_pos.y));
    ImGui::SetNextWindowSize(ImVec2(560, 320));
    if (ImGui::Begin("Audio Mixer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(22, 10));

        for (int i = 0; i < 5; ++i)
        {
            if (i > 0) ImGui::SameLine();
            ImGui::PushID(i);
            {
                ImGui::Dummy(ImVec2(channelPadding, 0));
                ImGui::SameLine(0.0f, 0.0f);
                DrawSingleChannel(i);
                if (i < 4)
                {
                    ImGui::SameLine(0.0f, 0.0f);
                    ImGui::Dummy(ImVec2(channelPadding, 0));
                    ImGui::SameLine(0.0f, 0.0f);
                }
            }
            ImGui::PopID();
        }

        ImGui::PopStyleVar(3);

        m_mixer_window_pos = ImGui::GetWindowPos();
        m_mixer_window_size = ImGui::GetWindowSize();
    }
    ImGui::End();
}

inline std::string WStringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) 
        return {};

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
    return result;
}

void FlipBit(bool& bit)
{
    bit = !bit; 
}

void ResizeWindow(HWND hwnd, int width, int height)
{
    g_Threading->Post([=]
    {
        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPED, FALSE);
        SetWindowPos(hwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
    });
}

std::mutex g_audioProcessesMutex;
std::vector<std::string> audioProcesses;
std::vector<std::string> selectedProcesses;

static int m_selected_channel = 0;

void AudioMixerUI::RenderProcesslist()
{                                                      
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(31, 31, 31, 255)));
    ImGui::SetNextWindowPos(ImVec2(m_mixer_window_pos.x + m_mixer_window_size.x, m_mixer_window_pos.y));
    ImGui::SetNextWindowSize(ImVec2(400, m_mixer_window_size.y));
    if (ImGui::Begin("Processlist", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));

        if (ImGui::Button("Refresh", ImVec2(380, 30))) 
        {
            g_Threading->Post([this]
            {
                auto fresh_list = Utility::GetAudioOutputProcesses();
                std::vector<std::string> new_audio_processes;
                for (auto& process : fresh_list)
                {
                    if (std::find(selectedProcesses.begin(), selectedProcesses.end(), process) == selectedProcesses.end())
                    {
                        new_audio_processes.push_back(process);
                    }
                }
                {
                    std::lock_guard<std::mutex> lock(g_Threading->m_mutex);
                    audioProcesses = std::move(new_audio_processes);
                }
            });
        }

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(3);

        int removeindedx = -1; 
        {
            std::lock_guard<std::mutex> lock(g_audioProcessesMutex);
            for (int i = 0; i < static_cast<int>(audioProcesses.size()); i++)
            {
                if (ImGui::Selectable(audioProcesses[i].c_str(), false))
                {
                    std::string selected_process = audioProcesses[i];
                    if (g_Threading)
                    {
                        g_Threading->Post([this, selected_process, i]
                        {
                            {
                                std::lock_guard<std::mutex> lock(g_audioProcessesMutex);
                                selectedProcesses.push_back(selected_process);
                                audioProcesses.erase(audioProcesses.begin() + i);
                            }
                            std::wstring process_name(selected_process.begin(), selected_process.end());
                            std::vector<DWORD> pids = Utility::FindProcessIds(process_name);
                            if (!pids.empty())
                            {
                                for (DWORD pid : pids)
                                {
                                    m_cableManager->AssignAppToCable(pid, m_selected_channel);
                                }
                            }
                            else
                            {
                                std::cerr << "RenderProcesslist: No instances of " << selected_process << " found" << std::endl;
                            }
                        });
                    }
                    else
                    {
                        std::cerr << "RenderProcesslist: g_Threading is invalid" << std::endl;
                    }
                }
            }
        }

        if (removeindedx != -1)
        {
			audioProcesses.erase(audioProcesses.begin() + removeindedx);
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void AudioMixerUI::RenderSidebar()
{
    ImGui::SetNextWindowSize(ImVec2(200, 320));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(31, 31, 31, 255)));

    static ImGuiID openNodeID = 0;
    static bool refreshSidebar = false;

    if (refreshSidebar)
    {
        refreshSidebar = false;
        ImGui::PopStyleColor();
        return;
    }


    if (ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
    {
        auto appMap = m_cableManager->GetAssignedAppNamesPerCable();
        auto allAssigned = m_cableManager->GetAllAssignedApps();

        for (int i = 0; i < 5; i++)
        {
            std::string name = m_channels[i]; 
            std::string lable = "| " + name + "##channel" + std::to_string(i);
            ImGuiID nodeID = ImGui::GetID(lable.c_str()); 
            bool isOpen = (openNodeID == nodeID); 

            ImGui::PushID(i); 
            ImGui::AlignTextToFramePadding(); 
			bool cickedArrow = ImGui::ArrowButton("##arrow", isOpen ? ImGuiDir_Down : ImGuiDir_Right);
            ImGui::SameLine(); 
            bool clickedlable = ImGui::Selectable(lable.c_str(), isOpen); 

            if (cickedArrow || clickedlable)
                openNodeID = isOpen ? 0 : nodeID;

            if (openNodeID == nodeID)
            {
                m_selected_channel = i;

                auto it = appMap.find(i);

                if (it != appMap.end() && !it->second.empty())
                {
                    for (const auto& appName : it->second)
                    {
                        std::string uft8Name = WStringToUtf8(appName);
                        ImGui::Bullet(); 
                        ImGui::SameLine(); 
                        if (ImGui::Selectable(uft8Name.c_str(), false))
                        {
                            g_Threading->Post([this, appName, i, allAssigned, uft8Name]
                            {
                                for (const auto& pair : allAssigned)
                                {
                                    if (pair.first != (UINT)i) continue;

                                    for (DWORD pid : pair.second)
                                    {
                                        std::wstring procName = Utility::GetMainProcessNameByPID(pid);
                                        if (procName == appName)
                                        {
                                            m_cableManager->RemoveAppFromCable(pid, i);
                                            {
                                                std::lock_guard<std::mutex> lock(g_Threading->m_mutex);
                                                audioProcesses.emplace_back(uft8Name);
                                            }
                                            break;
                                        }
                                    }
                                }
                            });
                            refreshSidebar = true;
                            ImGui::PopID();
                            ImGui::End();
                            ImGui::PopStyleColor();
                            return;
                        }
                    }
                }
                else
                {
                    ImGui::TextDisabled("No apps assigned.");
                }
                ImGui::Spacing();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));

                if (ImGui::Button("+ Add App", ImVec2(100, 30)))
                {
                    g_extend_window ? ResizeWindow(GetActiveWindow(), 770, 350) : ResizeWindow(GetActiveWindow(), 770 + 400, 350);
                    g_extend_window = !g_extend_window;
                }

                ImGui::PopStyleColor(4);
                ImGui::PopStyleVar(3);
            }
            ImGui::PopID(); 
        }

        m_window_pos = ImGui::GetWindowPos();
        m_window_size = ImGui::GetWindowSize();
    }
    ImGui::End();

    ImGui::PopStyleColor();
}

void AudioMixerUI::DrawSingleChannel(int index)
{
    ImGui::BeginGroup();

    ImVec2 start = ImGui::GetCursorScreenPos();
    float centerX = start.x + sliderWidth * 0.5f;

    ImVec2 nameSize = ImGui::CalcTextSize(m_channel_names[index].c_str());
    ImVec2 idSize = ImGui::CalcTextSize(m_channels[index]);

    ImVec2 namePos(centerX - nameSize.x * 0.5f, ImGui::GetCursorScreenPos().y);
    ImGui::GetWindowDrawList()->AddText(namePos, ImGui::GetColorU32(ImGuiCol_Text), m_channel_names[index].c_str());
    ImGui::Dummy(ImVec2(0.0f, nameSize.y + 2.0f));

    ImVec2 idPos(centerX - idSize.x * 0.5f, ImGui::GetCursorScreenPos().y);
    ImGui::GetWindowDrawList()->AddText(idPos, ImGui::GetColorU32(ImGuiCol_TextDisabled), m_channels[index]);
    ImGui::Dummy(ImVec2(0.0f, idSize.y + 6.0f));

    ImVec2 p = ImGui::GetCursorScreenPos();
    float grabY = p.y + sliderHeight * (1.0f - m_volumes[index]) - 6.0f;
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + sliderWidth * 0.4f, p.y), ImVec2(p.x + sliderWidth * 0.6f, p.y + sliderHeight), IM_COL32(45, 45, 45, 255), 4.0f);
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(p.x + sliderWidth * 0.2f, grabY), ImVec2(p.x + sliderWidth * 0.8f, grabY + 12.0f), IM_COL32(230, 230, 230, 255), 4.0f);

    ImGui::SetCursorScreenPos(p);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 0, 0, 0));

    static float smoothVolumes[5] = { 0 };
    static float lastUnmutedVolumes[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
    float target = m_muted_channels[index] ? 0.0f : m_volumes[index];
    smoothVolumes[index] += (target - smoothVolumes[index]) * 0.12f;

    ImGui::PushID(index);

    ImGui::VSliderFloat("##slider", ImVec2(sliderWidth, sliderHeight), &smoothVolumes[index], 0.0f, 1.0f, ""); 

    if (!m_muted_channels[index])
    {
        if (ImGui::IsItemActive())
        {
            m_volumes[index] = std::clamp(smoothVolumes[index], 0.0f, 1.0f);
            lastUnmutedVolumes[index] = m_volumes[index];
            if (m_cableManager)
            {
                g_Threading->Post([this, index, volume = m_volumes[index]]
                {
                    m_cableManager->SetCableVolume(index, volume);
                });
            }
        }

        if (ImGui::IsItemHovered())
        {
            float scroll = ImGui::GetIO().MouseWheel; 
            if (scroll != 0.0f && !m_muted_channels[index])
            {
                m_volumes[index] = std::clamp(m_volumes[index] + scroll * 0.01f, 0.0f, 1.0f); 
                if (m_cableManager)
                {
                    g_Threading->Post([this, index, volume = m_volumes[index]]
                    {
                        m_cableManager->SetCableVolume(index, volume);
                    });
                }
            }
        }
    }
    else
    {
        if (m_volumes[index] > 0.0f)
        {
            lastUnmutedVolumes[index] = m_volumes[index]; 
        }
    }

    ImGui::PopID(); 

    ImGui::PopStyleColor(3);

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s volume: %.0f%%", m_channel_names[index], m_volumes[index] * 100.f);
        ImGui::EndTooltip();
    }

    ImGuiStyle& style = ImGui::GetStyle();
    float oldRounding = style.FrameRounding;
    style.FrameRounding = 1000.0f; 

    const float buttonSize = 32.0f;
    ImVec2 buttonPos = ImGui::GetCursorScreenPos();
    buttonPos.x += (sliderWidth - buttonSize) * 0.5f; 
    ImGui::SetCursorScreenPos(buttonPos);

    if (!m_muted_channels[index])
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.0f));
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.28f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.22f, 0.22f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));
    }

    ImGui::PushFont(g_Rendering->m_icon_font); 

    const char* label = m_muted_channels[index] ? ICON_FA_MICROPHONE_SLASH : ICON_FA_MICROPHONE;
    if (ImGui::Button(label, ImVec2(buttonSize, buttonSize)))
    {
        m_muted_channels[index] = !m_muted_channels[index];

        if (m_cableManager)
        {
            g_Threading->Post([this, index, volume = m_muted_channels[index] ? 0.0f : m_volumes[index]]
            {
                m_cableManager->SetCableVolume(index, volume);
            });
        }
    }

    ImGui::PopFont(); 

    ImGui::PopStyleColor(4); 

    style.FrameRounding = oldRounding;
    ImGui::EndGroup();
}

void AudioMixerUI::InitStyle()
{
    auto Style = &ImGui::GetStyle();
    auto GetColor = Style->Colors;
    Style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
    Style->ItemSpacing = ImVec2(20, 8);
    Style->SeparatorTextAlign = ImVec2(0.5f, 0.5f);
    Style->WindowBorderSize = 0.f;

    Style->Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    Style->Colors[ImGuiCol_TitleBgActive] = GetColor[ImGuiCol_TitleBg];
    Style->Colors[ImGuiCol_TitleBgCollapsed] = GetColor[ImGuiCol_TitleBg];

    Style->Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);
    Style->Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    Style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 0.f);
    Style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.16f, 0.f);

    Style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);
    Style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);

    Style->Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    Style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
}