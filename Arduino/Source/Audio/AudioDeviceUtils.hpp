#pragma once

class VirtualCableManager {
public:
    VirtualCableManager()  : pEnumerator(nullptr), pDefaultDevice(nullptr)
    {
        virtualCables.resize(MAX_CABLES);
    }

    ~VirtualCableManager()
    {
        for (auto& cable : virtualCables)
        {
            if (cable.sessionManager)
            {
                cable.sessionManager->Release();
            }
        }

        SAFE_RELEASE(pDefaultDevice);
        SAFE_RELEASE(pEnumerator);
    }
public:
    HRESULT Initialize();

    std::vector<std::wstring> ListVirtualCables();
    std::map<UINT, std::set<std::wstring>> GetAssignedAppNamesPerCable() const;
    std::vector<std::pair<UINT, std::vector<DWORD>>> GetAllAssignedApps() const;

    HRESULT AssignAppToCable(DWORD processId, UINT cableIndex);
    HRESULT RemoveAppFromCable(DWORD processId, UINT cableindex); 
    HRESULT GetCableVolume(UINT cableIndex, float* volume);
    HRESULT SetCableVolume(UINT cableIndex, float volume);
private:
    struct VirtualCable 
    {
        std::wstring id;
        std::vector<DWORD> assignedProcesses;
        IAudioSessionManager2* sessionManager;
    };

    HRESULT EnumerateSessions(IAudioSessionManager2* sessionManager, std::vector<DWORD>& processIds);
    HRESULT CreateVirtualCable(UINT cableIndex);
private:
    IMMDeviceEnumerator* pEnumerator;
    IMMDevice* pDefaultDevice;
    std::vector<VirtualCable> virtualCables;
};