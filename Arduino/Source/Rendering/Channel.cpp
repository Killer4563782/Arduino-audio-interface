#include "Interface.hpp"
#include "Audio/AudioDeviceUtils.hpp"
#include "Rendering/Rendering.hpp"

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
			g_VirtualCableManager->SetCableVolume(index, m_volumes[index]);
		}

		if (ImGui::IsItemHovered())
		{
			float scroll = ImGui::GetIO().MouseWheel;
			if (scroll != 0.0f && !m_muted_channels[index])
			{
				m_volumes[index] = std::clamp(m_volumes[index] + scroll * 0.01f, 0.0f, 1.0f);
				g_VirtualCableManager->SetCableVolume(index, m_volumes[index]);
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

	!m_muted_channels[index] ? ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.0f)) : ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.15f, 0.15f, 1.0f));

	ImGui::PushFont(g_Rendering->m_icon_font);

	const char* label = m_muted_channels[index] ? ICON_FA_MICROPHONE_SLASH : ICON_FA_MICROPHONE;
	if (ImGui::Button(label, ImVec2(buttonSize, buttonSize)))
	{
		m_muted_channels[index] = !m_muted_channels[index];
		g_VirtualCableManager->SetCableVolume(index, m_muted_channels[index] ? 0.f : m_volumes[index]);
	}

	ImGui::PopFont();
	ImGui::PopStyleColor(1);

	style.FrameRounding = oldRounding;
	ImGui::EndGroup();
}