#include "AudioDeviceUtils.hpp"
#include "Utility/Utility.hpp"

HRESULT VirtualCableManager::Initialize()
{
    const HRESULT hrCom = CoInitialize(nullptr);
    const bool comInitialized = SUCCEEDED(hrCom);

    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(enumerator.GetAddressOf()));
    if (FAILED(hr))
        goto Cleanup;

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDefaultDevice);
    if (FAILED(hr))
        goto Cleanup;

    for (UINT i = 0; i < MAX_CABLES; ++i)
    {
        hr = CreateVirtualCable(i);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    return S_OK;

Cleanup:
    if (comInitialized)
        CoUninitialize();
    return hr;
}

HRESULT VirtualCableManager::CreateVirtualCable(UINT cableIndex)
{
    if (cableIndex >= MAX_CABLES) 
        return E_INVALIDARG;

    VirtualCable& cable = virtualCables[cableIndex];
    cable.id = L"VirtualCable_" + std::to_wstring(cableIndex + 1);

    HRESULT hr = pDefaultDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, nullptr, (void**)&cable.sessionManager);
    if (FAILED(hr)) 
        return hr;

    return S_OK;
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
        const auto& cable = virtualCables[i];
        for (DWORD pid : cable.assignedProcesses)
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

std::vector<std::wstring> VirtualCableManager::ListVirtualCables()
{
    std::vector<std::wstring> cables;
    for (const auto& cable : virtualCables)
    {
        cables.push_back(cable.id);
    }
    return cables;
}

HRESULT VirtualCableManager::AssignAppToCable(DWORD processId, UINT cableIndex)
{
    if (cableIndex >= MAX_CABLES) 
        return E_INVALIDARG;

    VirtualCable& cable = virtualCables[cableIndex];
    IAudioSessionManager2* sessionManager = cable.sessionManager;
    if (!sessionManager) 
        return E_UNEXPECTED;

    std::vector<DWORD> sessionPids;
    HRESULT hr = EnumerateSessions(sessionManager, sessionPids);
    if (FAILED(hr))
    {
        std::wcerr << L"Failed to enumerate sessions for cable " << cable.id << L": " << _com_error(hr).ErrorMessage() << std::endl;
        return hr;
    }

    for (DWORD pid : sessionPids)
    {
        if (pid == processId)
        {
            cable.assignedProcesses.push_back(processId);
            return S_OK;
        }
    }

    std::wcerr << L"Process ID " << processId << L" not found in audio sessions for " << cable.id << std::endl;
    return E_FAIL;
}

HRESULT VirtualCableManager::RemoveAppFromCable(DWORD processId, UINT cableindex)
{
    if (cableindex >= MAX_CABLES)
        return E_INVALIDARG;

    VirtualCable& cable = virtualCables[cableindex]; 
    auto& assigned = cable.assignedProcesses; 
	auto it = std::remove(assigned.begin(), assigned.end(), processId);
    if (it != assigned.end())
    {
        assigned.erase(it); 
        return S_OK;
    }

    return E_FAIL; 
}

HRESULT VirtualCableManager::EnumerateSessions(IAudioSessionManager2* sessionManager, std::vector<DWORD>& processIds)
{
    IAudioSessionEnumerator* pSessionEnum = nullptr;
    HRESULT hr = sessionManager->GetSessionEnumerator(&pSessionEnum);
    if (FAILED(hr))
        return hr;

    int sessionCount = 0;
    hr = pSessionEnum->GetCount(&sessionCount);
    if (FAILED(hr))
    {
        pSessionEnum->Release();
        return hr;
    }

    processIds.clear();
    for (int i = 0; i < sessionCount; ++i)
    {
        IAudioSessionControl* pSessionControl = nullptr;
        hr = pSessionEnum->GetSession(i, &pSessionControl);
        if (SUCCEEDED(hr))
        {
            IAudioSessionControl2* pSessionControl2 = nullptr;
            hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
            if (SUCCEEDED(hr))
            {
                DWORD pid = 0;
                hr = pSessionControl2->GetProcessId(&pid);
                if (SUCCEEDED(hr) && pid != 0)
                {
                    processIds.push_back(pid);
                }
                pSessionControl2->Release();
            }
            pSessionControl->Release();
        }
    }

    pSessionEnum->Release();
    return S_OK;
}

HRESULT VirtualCableManager::GetCableVolume(UINT cableIndex, float* volume)
{
    if (cableIndex >= MAX_CABLES || !volume)
        return E_INVALIDARG;

    VirtualCable& cable = virtualCables[cableIndex];
    IAudioSessionManager2* sessionManager = cable.sessionManager;
    if (!sessionManager) 
        return E_UNEXPECTED;

    IAudioSessionEnumerator* pSessionEnum = nullptr;
    HRESULT hr = sessionManager->GetSessionEnumerator(&pSessionEnum);
    if (FAILED(hr))
    {
        std::wcerr << L"Failed to get session enumerator for " << cable.id << L": " << _com_error(hr).ErrorMessage() << std::endl;
        return hr;
    }

    int sessionCount = 0;
    hr = pSessionEnum->GetCount(&sessionCount);
    if (FAILED(hr) || sessionCount == 0)
    {
        pSessionEnum->Release();
        std::wcerr << L"No sessions found for " << cable.id << std::endl;
        return FAILED(hr) ? hr : E_FAIL;
    }

    *volume = 0.0f;
    float totalVolume = 0.0f;
    int matchingSessions = 0;

    for (int i = 0; i < sessionCount; ++i)
    {
        IAudioSessionControl* pSessionControl = nullptr;
        hr = pSessionEnum->GetSession(i, &pSessionControl);
        if (FAILED(hr))
        {
            std::wcerr << L"Failed to get session " << i << L" for " << cable.id << L": " << _com_error(hr).ErrorMessage() << std::endl;
            continue;
        }

        IAudioSessionControl2* pSessionControl2 = nullptr;
        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
        if (SUCCEEDED(hr))
        {
            DWORD pid = 0;
            hr = pSessionControl2->GetProcessId(&pid);
            if (SUCCEEDED(hr) && pid != 0)
            {
                if (std::find(cable.assignedProcesses.begin(), cable.assignedProcesses.end(), pid) != cable.assignedProcesses.end())
                {
                    ISimpleAudioVolume* pSimpleVolume = nullptr;
                    hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleVolume);
                    if (SUCCEEDED(hr))
                    {
                        float sessionVolume = 0.0f;
                        hr = pSimpleVolume->GetMasterVolume(&sessionVolume);
                        if (SUCCEEDED(hr))
                        {
                            totalVolume += sessionVolume;
                            matchingSessions++;
                        }
                        else
                        {
                            std::wcerr << L"Failed to get volume for PID " << pid << L": " << _com_error(hr).ErrorMessage() << std::endl;
                        }
                        pSimpleVolume->Release();
                    }
                }
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();
    }

    pSessionEnum->Release();

    if (matchingSessions > 0)
    {
        *volume = totalVolume / matchingSessions; 
        return S_OK;
    }

    std::wcerr << L"No sessions matched assigned PIDs for " << cable.id << std::endl;
    return E_FAIL;
}

HRESULT VirtualCableManager::SetCableVolume(UINT cableIndex, float volume)
{
    if (cableIndex >= MAX_CABLES || volume < 0.0f || volume > 1.0f) return E_INVALIDARG;

    VirtualCable& cable = virtualCables[cableIndex];
    IAudioSessionManager2* sessionManager = cable.sessionManager;
    if (!sessionManager) return E_UNEXPECTED;

    IAudioSessionEnumerator* pSessionEnum = nullptr;
    HRESULT hr = sessionManager->GetSessionEnumerator(&pSessionEnum);
    if (FAILED(hr))
    {
        std::wcerr << L"Failed to get session enumerator for " << cable.id << L": " << _com_error(hr).ErrorMessage() << std::endl;
        return hr;
    }

    int sessionCount = 0;
    hr = pSessionEnum->GetCount(&sessionCount);
    if (FAILED(hr) || sessionCount == 0)
    {
        pSessionEnum->Release();
        std::wcerr << L"No sessions found for " << cable.id << std::endl;
        return FAILED(hr) ? hr : E_FAIL;
    }

    bool volumeSet = false;

    for (int i = 0; i < sessionCount; ++i)
    {
        IAudioSessionControl* pSessionControl = nullptr;
        hr = pSessionEnum->GetSession(i, &pSessionControl);
        if (FAILED(hr))
        {
            std::wcerr << L"Failed to get session " << i << L" for " << cable.id << L": " << _com_error(hr).ErrorMessage() << std::endl;
            continue;
        }

        IAudioSessionControl2* pSessionControl2 = nullptr;
        hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
        if (SUCCEEDED(hr))
        {
            DWORD pid = 0;
            hr = pSessionControl2->GetProcessId(&pid);
            if (SUCCEEDED(hr) && pid != 0)
            {
                if (std::find(cable.assignedProcesses.begin(), cable.assignedProcesses.end(), pid) != cable.assignedProcesses.end())
                {
                    ISimpleAudioVolume* pSimpleVolume = nullptr;
                    hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleVolume);
                    if (SUCCEEDED(hr))
                    {
                        hr = pSimpleVolume->SetMasterVolume(volume, nullptr);
                        if (SUCCEEDED(hr))
                        {
                            volumeSet = true;
                        }
                        else
                        {
                            std::wcerr << L"Failed to set volume for PID " << pid << L": " << _com_error(hr).ErrorMessage() << std::endl;
                        }
                        pSimpleVolume->Release();
                    }
                }
            }
            pSessionControl2->Release();
        }
        pSessionControl->Release();
    }

    pSessionEnum->Release();

    return volumeSet ? S_OK : E_FAIL;
}