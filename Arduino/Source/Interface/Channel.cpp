#include "Interface.hpp"
#include "Audio/AudioDeviceUtils.hpp"
#include "Rendering/Rendering.hpp"
#include "ChannelHelper.hpp"
#include "Config/Configuration.hpp"

void AudioMixerUI::DrawSingleChannel(int index)
{
    static AudioMixerUIHelper helper(48.f, 180.0f);
    static float smoothVolumes[5] = { 0 };
    static float lastUnmutedVolumes[5] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

    ImGui::BeginGroup();
    {
        helper.DrawTextCentered(m_channel_names[index], ImGui::GetColorU32(ImGuiCol_Text));
        helper.DrawTextCentered(m_channels[index], ImGui::GetColorU32(ImGuiCol_TextDisabled));

        ImVec2 pos = ImGui::GetCursorScreenPos();
        helper.DrawSliderBackground(pos);
        helper.DrawSliderGrab(pos, m_volumes[index]);

        ImGui::SetCursorScreenPos(pos);
        helper.PushSliderStyle();
        ImGui::PushID(index);

        float target = m_muted_channels[index] ? 0.0f : m_volumes[index];
        smoothVolumes[index] += (target - smoothVolumes[index]) * 0.12f;

        ImGui::VSliderFloat("##slider", ImVec2(helper.GetSliderWidth(), helper.GetSliderHeight()), &smoothVolumes[index], 0.0f, 1.0f, "");

        if (!m_muted_channels[index])
        {
            if (ImGui::IsItemActive())
            {
                m_volumes[index] = std::clamp(smoothVolumes[index], 0.0f, 1.0f);
                lastUnmutedVolumes[index] = m_volumes[index];
                g_VirtualCableManager->SetCableVolume(index, m_volumes[index]);
                g_Configuration->SetChannelVolume(index, m_volumes[index]);
            }

            if (ImGui::IsItemHovered())
            {
                float scroll = ImGui::GetIO().MouseWheel;
                if (scroll != 0.0f)
                {
                    m_volumes[index] = std::clamp(m_volumes[index] + scroll * 0.01f, 0.0f, 1.0f);
                    g_VirtualCableManager->SetCableVolume(index, m_volumes[index]);
                    g_Configuration->SetChannelVolume(index, m_volumes[index]);
                }
            }
        }
        else if (m_volumes[index] > 0.0f)
        {
            lastUnmutedVolumes[index] = m_volumes[index];
        }

        ImGui::PopID();
        helper.PopSliderStyle();

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s volume: %.0f%%", m_channel_names[index].c_str(), m_volumes[index] * 100.f);
            ImGui::EndTooltip();
        }

        ImGuiStyle& style = ImGui::GetStyle();
        float oldRounding = style.FrameRounding;
        style.FrameRounding = 1000.0f;

        ImVec2 buttonPos = ImGui::GetCursorScreenPos();
        buttonPos.x += (helper.GetSliderWidth() - 32.0f) * 0.5f;
        ImGui::SetCursorScreenPos(buttonPos);

        ImGui::PushStyleColor(ImGuiCol_Text, m_muted_channels[index] ? ImVec4(0.65f, 0.15f, 0.15f, 1.0f) : ImVec4(1.f, 1.f, 1.f, 1.0f));
        ImGui::PushFont(g_Rendering->m_icon_font);

        const char* label = m_muted_channels[index] ? ICON_FA_MICROPHONE_SLASH : ICON_FA_MICROPHONE;
        if (ImGui::Button(label, ImVec2(32.f, 32.f)))
        {
            m_muted_channels[index] = !m_muted_channels[index];
            g_VirtualCableManager->SetCableVolume(index, m_muted_channels[index] ? 0.f : m_volumes[index]);
            g_Configuration->SetChannelVolume(index, m_muted_channels[index] ? 0.f : m_volumes[index]);
        }

        ImGui::PopFont();
        ImGui::PopStyleColor();
        style.FrameRounding = oldRounding;
    }
    ImGui::EndGroup();
}