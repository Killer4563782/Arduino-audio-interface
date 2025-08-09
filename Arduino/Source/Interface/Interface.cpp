#include "Interface.hpp"

void AudioMixerUI::RenderSlider()
{
	ImGui::SetNextWindowPos(ImVec2(m_window_pos.x + m_window_size.x, m_tabbar_window_size.y));
	ImGui::SetNextWindowSize(ImVec2(560, 320));
	if (ImGui::Begin("Audio Mixer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		switch (m_selected_tab)
		{
			case 0: 
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(22, 10));

				for (int i = 0; i < MAX_CABLES; ++i)
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
			}
			break; 
			case 1: 
			{
				ImGui::SeparatorText("General Settings"); 
				for (int i = 0; i < MAX_CABLES; ++i)
				{
					char buf[32];
					snprintf(buf, sizeof(buf), "%s", m_channel_names[i].c_str());
					if (ImGui::InputText(("Channel " + std::to_string(i + 1)).c_str(), buf, sizeof(buf)))
					{
						m_channel_names[i] = buf;
					}
				}
			}
			break; 
		}

		m_mixer_window_pos = ImGui::GetWindowPos();
		m_mixer_window_size = ImGui::GetWindowSize();
	}
	ImGui::End();
}

void AudioMixerUI::RenderTabBar()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(560 + 200, 60));
	ImGui::SetNextWindowBgAlpha(1.0f);
	if (ImGui::Begin("Control Panel", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 12));

		(m_selected_tab == 0) ? ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(71, 71, 71, 255))) : void();
		if (ImGui::Button("Home", ImVec2(100, 40)))
		{
			m_selected_tab = 0; 
		}
		(m_selected_tab == 0) ? ImGui::PopStyleColor() : void();

		ImGui::SameLine(); 

		(m_selected_tab == 1) ? ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(71, 71, 71, 255))) : void();
		if (ImGui::Button("Settings", ImVec2(100, 40)))
		{
			m_selected_tab = 1; 
		}
		(m_selected_tab == 1) ? ImGui::PopStyleColor() : void();

		m_tabbar_window_pos = ImGui::GetWindowPos();
		m_tabbar_window_size = ImGui::GetWindowSize();
		ImGui::PopStyleVar();
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

	Style->Colors[ImGuiCol_Separator] = ImVec4(1.f, 1.f, 1.f, 1.f);
}