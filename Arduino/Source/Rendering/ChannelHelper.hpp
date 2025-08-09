#pragma once 
#include <imgui.h>

class AudioMixerUIHelper {
public:
    AudioMixerUIHelper(float sliderWidth, float sliderHeight)  
    {
        m_sliderWidth = sliderWidth;
        m_sliderHeight = sliderHeight;
    }

    void DrawTextCentered(const std::string& text, ImU32 color)
    {
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 pos(ImGui::GetCursorScreenPos().x + (m_sliderWidth - textSize.x) * 0.5f, ImGui::GetCursorScreenPos().y);
        ImGui::GetWindowDrawList()->AddText(pos, color, text.c_str());
        ImGui::Dummy(ImVec2(0.0f, textSize.y + 2.0f));
    }

    void DrawSliderBackground(const ImVec2& pos)
    {
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + m_sliderWidth * 0.4f, pos.y), ImVec2(pos.x + m_sliderWidth * 0.6f, pos.y + m_sliderHeight), IM_COL32(45, 45, 45, 255), 4.0f);
    }

    void DrawSliderGrab(const ImVec2& pos, float value)
    {
        float grabY = pos.y + m_sliderHeight * (1.0f - value) - 6.0f;
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + m_sliderWidth * 0.2f, grabY), ImVec2(pos.x + m_sliderWidth * 0.8f, grabY + 12.0f), IM_COL32(230, 230, 230, 255), 4.0f);
    }

    void PushSliderStyle()
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 0, 0, 0));
    }

    void PopSliderStyle()
    {
        ImGui::PopStyleColor(3);
    }

    float GetSliderWidth() const
    {
        return m_sliderWidth;
    }

    float GetSliderHeight() const
    {
        return m_sliderHeight;
    }
private:
    float m_sliderWidth;
    float m_sliderHeight;
};