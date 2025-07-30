#include "Interface.hpp"
#include "Audio/AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"

void AudioMixerUI::RenderProcesslist()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(31, 31, 31, 255)));
	ImGui::SetNextWindowPos(ImVec2(m_mixer_window_pos.x + m_mixer_window_size.x, m_mixer_window_pos.y));
	ImGui::SetNextWindowSize(ImVec2(400, m_mixer_window_size.y));
	if (ImGui::Begin("Processlist", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
	{
		if (ImGui::Button("Refresh", ImVec2(380, 30)))
		{
			auto fresh_list = Utility::GetAudioOutputProcesses();
			audioProcesses.clear(); 
			cachedAudioProcessSet.clear();

			std::vector<std::string> new_audio_processes;
			for (auto& process : fresh_list)
			{
				if (std::find(selectedProcesses.begin(), selectedProcesses.end(), process) == selectedProcesses.end())
				{
					cachedAudioProcessSet.insert(process);
					audioProcesses.push_back(process);
				}
			}
		}

		for (int i = 0; i < static_cast<int>(audioProcesses.size()); i++)
		{
			if (ImGui::Selectable(audioProcesses[i].c_str(), false))
			{
				const std::string& name = audioProcesses[i];
				selectedProcesses.push_back(name);
				std::wstring process_name(name.begin(), name.end());

				for (DWORD pid : Utility::FindProcessIds(process_name))
				{
					g_VirtualCableManager->AssignAppToCable(pid, m_selected_channel);
				}

				cachedAudioProcessSet.erase(name); 
				audioProcesses.erase(audioProcesses.begin() + i); 
				--i; 
			}
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
}