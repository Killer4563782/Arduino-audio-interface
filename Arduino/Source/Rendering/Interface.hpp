#pragma once 
#include <imgui.h>

class AudioMixerUI {
public:
    void Render()
    {
        RenderSidebar();
        RenderSlider();
        RenderProcesslist(); 
    }
  
    void InitStyle();
private:
    static inline std::array<const char*, 5> m_channels = {
       "Channel 1",
       "Channel 2",
       "Channel 3",
       "Channel 4",
       "Channel 5",
    };

    static inline std::array<std::string, 5> m_channel_names = {
       "Name",
       "Name",
       "Name",
       "Name",
       "Name"
    };

    inline static std::array<bool, 5> m_muted_channels = {
        false,
        false,
        false,
        false,
        false
    };

    inline static std::array<float, 5> m_volumes = {
        1.0f,
        1.0f,
        1.0f,
        1.0f,
        1.0f
    };

    void RenderSlider();
    void RenderSidebar();
    void RenderProcesslist(); 
    void DrawSingleChannel(int index);
private: 
    static inline ImVec2 m_window_size;
    static inline ImVec2 m_window_pos; 

    static inline ImVec2 m_mixer_window_size;
    static inline ImVec2 m_mixer_window_pos;

    static constexpr float channelPadding = 20.0f;

    static inline std::vector<std::string> audioProcesses;
    static inline std::vector<std::string> selectedProcesses;
    static inline std::unordered_set<std::string> cachedAudioProcessSet;

    static inline int m_selected_channel = 0;

    static inline bool refreshSidebar = true;
    static inline std::map<UINT, std::set<std::string>> cachedAppMap;
    static inline std::vector<std::pair<UINT, std::vector<DWORD>>> cachedAllAssigned;
};

inline AudioMixerUI* g_Interface; 