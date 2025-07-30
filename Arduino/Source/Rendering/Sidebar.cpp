#include "Interface.hpp"
#include "Audio/AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"

void AudioMixerUI::RenderSidebar()
{
	ImGui::SetNextWindowSize(ImVec2(200, 320));
	ImGui::SetNextWindowPos(ImVec2(0, 0));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(ImColor(31, 31, 31, 255)));

	static ImGuiID openNodeID = 0;
	static bool refreshSidebar = true;

	static std::map<UINT, std::set<std::string>> cachedAppMap;
	static std::vector<std::pair<UINT, std::vector<DWORD>>> cachedAllAssigned;

	if (refreshSidebar)
	{
		cachedAppMap = g_VirtualCableManager->GetAssignedAppNamesPerCable();
		cachedAllAssigned = g_VirtualCableManager->GetAllAssignedApps();
		refreshSidebar = false;
	}

	if (ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
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

				auto it = cachedAppMap.find(i);

				if (it != cachedAppMap.end() && !it->second.empty())
				{
					for (const auto& appName : it->second)
					{
						ImGui::Bullet();
						ImGui::SameLine();
						if (ImGui::Selectable(appName.c_str(), false))
						{
							for (const auto& pair : cachedAllAssigned)
							{
								if (pair.first != (UINT)i) continue;

								for (DWORD pid : pair.second)
								{
									if (Utility::GetMainProcessNameByPID(pid) == appName)
									{
										g_VirtualCableManager->RemoveAppFromCable(pid, i);
										audioProcesses.emplace_back(appName);
										refreshSidebar = true;
										break;
									}
								}
							}
						}
					}
				}
				else
				{
					ImGui::TextDisabled("No apps assigned.");
				}
				ImGui::Spacing();

				if (ImGui::Button("+ Add App", ImVec2(100, 30)))
				{
					g_extend_window ? Utility::ResizeWindow(GetActiveWindow(), 770, 350) : Utility::ResizeWindow(GetActiveWindow(), 770 + 400, 350);
					g_extend_window = !g_extend_window;
				}
			}
			ImGui::PopID();
		}

		m_window_pos = ImGui::GetWindowPos();
		m_window_size = ImGui::GetWindowSize();
	}
	ImGui::End();

	ImGui::PopStyleColor();
}