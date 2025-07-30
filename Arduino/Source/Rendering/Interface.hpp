#pragma once 
#include <imgui.h>
#include "Audio/AudioDeviceUtils.hpp"

class AudioMixerUI {
public:
    void Render()
    {
        RenderSidebar();
        RenderSlider();
        RenderProcesslist(); 
    }

    void SetCableManager(VirtualCableManager* manager)
    {
        m_cableManager = manager;
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

    std::array<std::vector<float>, 5> peakHistory;
    static constexpr int maxHistory = 40;
    static constexpr float sliderWidth = 48.0f;
    static constexpr float sliderHeight = 180.0f;
    static constexpr float channelPadding = 20.0f;

    static inline bool g_extend_window = false;
    static inline VirtualCableManager* m_cableManager = nullptr;
};

inline AudioMixerUI* g_Interface; 