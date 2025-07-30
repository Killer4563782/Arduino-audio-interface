#include "Interface.hpp"

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

void AudioMixerUI::InitStyle()
{
	auto Style = &ImGui::GetStyle();
	auto GetColor = Style->Colors;
	Style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	Style->ItemSpacing = ImVec2(20, 8);
	Style->SeparatorTextAlign = ImVec2(0.5f, 0.5f);
	Style->WindowBorderSize = 0.f;
	Style->FramePadding = ImVec2(6, 6); 
	Style->FrameRounding = 8.0f; 
	Style->ItemSpacing = ImVec2(8, 6); 

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

	Style->Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
	Style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
	Style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
}