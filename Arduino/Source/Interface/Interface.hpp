#pragma once 
#include <imgui.h>

class AudioMixerUI {
public:
    AudioMixerUI()
    {
        m_channels = {
            "Channel 1",
            "Channel 2",
            "Channel 3",
            "Channel 4",
            "Channel 5",
        };

        m_channel_names = {
            "Name",
            "Name",
            "Name",
            "Name",
            "Name"
        };

        for (int i = 0; i < MAX_CABLES; i++)
        {
            m_muted_channels[i] = false; 
            m_volumes[i] = 1.0f;
        }

        ApplyConfig();
    }

    void Render()
    {
        RenderTabBar();
        RenderSidebar();
        RenderSlider();
        RenderProcesslist(); 
    }
  
    void InitStyle();
private:
    void RenderSlider();
    void ApplyConfig(); 
    void RenderSidebar();
    void RenderTabBar(); 
    void RenderProcesslist(); 
    void DrawSingleChannel(int index);
private: 
    static inline ImVec2 m_window_size;
    static inline ImVec2 m_window_pos; 

    static inline ImVec2 m_mixer_window_size;
    static inline ImVec2 m_mixer_window_pos;

    static inline ImVec2 m_tabbar_window_size;
    static inline ImVec2 m_tabbar_window_pos;
private:
    static constexpr float channelPadding = 20.0f;
    static inline int m_selected_channel = 0;
    static inline int m_selected_tab = 0;
    static inline bool refreshSidebar = true;

    static inline std::array<std::string, 5> m_channel_names;
    static inline std::array<const char*, 5> m_channels;
    static inline std::array<bool, 5> m_muted_channels;
    static inline std::array<float, 5> m_volumes;
    static inline std::vector<std::string> audioProcesses;
    static inline std::vector<std::string> selectedProcesses;
    static inline std::unordered_set<std::string> cachedAudioProcessSet;
    static inline std::map<UINT, std::set<std::string>> cachedAppMap;
    static inline std::vector<std::pair<UINT, std::vector<DWORD>>> cachedAllAssigned;
};

inline AudioMixerUI* g_Interface; 