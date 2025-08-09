#include "Interface.hpp"
#include "Audio/AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"
#include "SidebarHelper.hpp"

void AudioMixerUI::RenderSidebar()
{
    static SidebarUIHelper helper;
    static ImGuiID openNodeID = 0;

    if (refreshSidebar)
    {
        cachedAppMap = g_VirtualCableManager->GetAssignedAppNamesPerCable();
        cachedAllAssigned = g_VirtualCableManager->GetAllAssignedApps();
        refreshSidebar = false;
    }

    helper.SetWindowProperties();

    if (ImGui::Begin("Sidebar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
    {
        for (int i = 0; i < MAX_CABLES; i++)
        {
            helper.DrawChannelNode(i, m_channels[i], openNodeID, m_selected_channel, cachedAppMap, cachedAllAssigned, audioProcesses, refreshSidebar);
        }
        helper.UpdateWindowInfo(m_window_pos, m_window_size);
    }
    ImGui::End();

    helper.PopStyle();
}