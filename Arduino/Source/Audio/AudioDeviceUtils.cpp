#include "AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"

HRESULT VirtualCableManager::Initialize()
{
    bool comInitialized = SUCCEEDED(CoInitialize(nullptr));

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)pEnumerator.GetAddressOf());
    if (FAILED(hr)) goto Cleanup;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice);
    if (FAILED(hr)) goto Cleanup;

    for (UINT i = 0; i < MAX_CABLES; ++i)
    {
        hr = CreateVirtualCable(i);
        if (FAILED(hr)) goto Cleanup;
    }

    return S_OK;

Cleanup:
    if (comInitialized) CoUninitialize();
    return hr;
}

HRESULT VirtualCableManager::CreateVirtualCable(UINT cableIndex)
{
    if (cableIndex >= MAX_CABLES) return E_INVALIDARG;

    VirtualCable& cable = virtualCables[cableIndex];
    cable.id = L"VirtualCable_" + std::to_wstring(cableIndex + 1);

    HRESULT hr = pDefaultDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&cable.sessionManager);
    return hr;
}

std::vector<std::pair<UINT, std::vector<DWORD>>> VirtualCableManager::GetAllAssignedApps() const
{
    std::vector<std::pair<UINT, std::vector<DWORD>>> result;
    for (UINT i = 0; i < virtualCables.size(); ++i)
    {
        if (!virtualCables[i].assignedProcesses.empty())
        {
            result.emplace_back(i, virtualCables[i].assignedProcesses);
        }
    }
    return result;
}

std::map<UINT, std::set<std::string>> VirtualCableManager::GetAssignedAppNamesPerCable() const
{
    std::map<UINT, std::set<std::string>> cableAppMap;
    for (UINT i = 0; i < virtualCables.size(); ++i)
    {
        for (DWORD pid : virtualCables[i].assignedProcesses)
        {
            std::string processName = Utility::GetMainProcessNameByPID(pid);
            if (!processName.empty())
            {
                cableAppMap[i].insert(processName);
            }
        }
    }
    return cableAppMap;
}

HRESULT VirtualCableManager::AssignAppToCable(DWORD processId, UINT cableIndex)
{
    if (cableIndex >= MAX_CABLES) return E_INVALIDARG;

    VirtualCable& cable = virtualCables[cableIndex];
    if (!cable.sessionManager) return E_UNEXPECTED;

    std::vector<DWORD> sessionPids;
    HRESULT hr = AudioSessionHelper::EnumerateSessions(cable.sessionManager, sessionPids);
    if (FAILED(hr))
    {
        std::wcerr << L"Failed to enumerate sessions for cable " << cable.id << L": " << _com_error(hr).ErrorMessage() << std::endl;
        return hr;
    }

    if (std::find(sessionPids.begin(), sessionPids.end(), processId) != sessionPids.end())
    {
        cable.assignedProcesses.push_back(processId);
        return S_OK;
    }

    std::wcerr << L"Process ID " << processId << L" not found in audio sessions for " << cable.id << std::endl;
    return E_FAIL;
}

HRESULT VirtualCableManager::RemoveAppFromCable(DWORD processId, UINT cableIndex)
{
    if (cableIndex >= MAX_CABLES) return E_INVALIDARG;

    auto& processes = virtualCables[cableIndex].assignedProcesses;
    auto it = std::remove(processes.begin(), processes.end(), processId);
    if (it != processes.end())
    {
        processes.erase(it, processes.end());
        return S_OK;
    }
    return E_FAIL;
}
