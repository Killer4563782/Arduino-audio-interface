#pragma once
#include "ControlHelper.hpp"
#include "SessionHelper.hpp"

class VirtualCableManager {
public:
    VirtualCableManager() 
    {
        pEnumerator = nullptr; 
        pDefaultDevice = nullptr;
        virtualCables.resize(MAX_CABLES);
    }

    ~VirtualCableManager()
    {
        for (auto& cable : virtualCables)
        {
            SAFE_RELEASE(cable.sessionManager);
        }
        SAFE_RELEASE(pDefaultDevice);
        SAFE_RELEASE(pEnumerator);
    }
public:
    HRESULT Initialize(); 

    HRESULT CreateVirtualCable(UINT cableIndex);
    std::vector<std::pair<UINT, std::vector<DWORD>>> GetAllAssignedApps() const; 
    std::map<UINT, std::set<std::string>> GetAssignedAppNamesPerCable() const; 

    std::vector<std::wstring> ListVirtualCables() const
    {
        std::vector<std::wstring> cables;
        for (const auto& cable : virtualCables)
        {
            cables.push_back(cable.id);
        }
        return cables;
    }

    HRESULT AssignAppToCable(DWORD processId, UINT cableIndex); 
    HRESULT RemoveAppFromCable(DWORD processId, UINT cableIndex); 

    HRESULT GetCableVolume(UINT cableIndex, float* volume)
    {
        if (cableIndex >= MAX_CABLES || !volume) 
            return E_INVALIDARG;

        VirtualCable& cable = virtualCables[cableIndex];
        if (!cable.sessionManager) 
            return E_UNEXPECTED;

        return VolumeControlHelper::GetCableVolume(cable.sessionManager, cable.assignedProcesses, volume, cable.id);
    }

    HRESULT SetCableVolume(UINT cableIndex, float volume)
    {
        if (cableIndex >= MAX_CABLES) 
            return E_INVALIDARG;

        VirtualCable& cable = virtualCables[cableIndex];
        if (!cable.sessionManager) 
            return E_UNEXPECTED;

        return VolumeControlHelper::SetCableVolume(cable.sessionManager, cable.assignedProcesses, volume, cable.id);
    }
private:
    struct VirtualCable 
    {
        std::wstring id;
        std::vector<DWORD> assignedProcesses;
        IAudioSessionManager2* sessionManager;
    };

    ComPtr<IMMDeviceEnumerator> pEnumerator;
    ComPtr<IMMDevice> pDefaultDevice;
    std::vector<VirtualCable> virtualCables;
};

inline VirtualCableManager* g_VirtualCableManager; 