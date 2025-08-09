#pragma once 

class VolumeControlHelper {
public:
    static HRESULT GetCableVolume(IAudioSessionManager2* sessionManager, const std::vector<DWORD>& assignedProcesses, float* volume, const std::wstring& cableId)
    {
        if (!sessionManager || !volume) 
            return E_INVALIDARG;

        ComPtr<IAudioSessionEnumerator> sessionEnum;
        HRESULT hr = sessionManager->GetSessionEnumerator(&sessionEnum);
        if (FAILED(hr))
        {
            std::wcerr << L"Failed to get session enumerator for " << cableId << L": " << _com_error(hr).ErrorMessage() << std::endl;
            return hr;
        }

        int sessionCount = 0;
        hr = sessionEnum->GetCount(&sessionCount);
        if (FAILED(hr) || sessionCount == 0)
        {
            std::wcerr << L"No sessions found for " << cableId << std::endl;
            return FAILED(hr) ? hr : E_FAIL;
        }

        float totalVolume = 0.0f;
        int matchingSessions = 0;

        for (int i = 0; i < sessionCount; ++i)
        {
            ComPtr<IAudioSessionControl> sessionControl;
            hr = sessionEnum->GetSession(i, &sessionControl);
            if (FAILED(hr)) 
                continue;

            ComPtr<IAudioSessionControl2> sessionControl2;
            hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);
            if (SUCCEEDED(hr))
            {
                DWORD pid = 0;
                hr = sessionControl2->GetProcessId(&pid);
                if (SUCCEEDED(hr) && pid != 0 && std::find(assignedProcesses.begin(), assignedProcesses.end(), pid) != assignedProcesses.end())
                {
                    ComPtr<ISimpleAudioVolume> simpleVolume;
                    hr = sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&simpleVolume);
                    if (SUCCEEDED(hr))
                    {
                        float sessionVolume = 0.0f;
                        hr = simpleVolume->GetMasterVolume(&sessionVolume);
                        if (SUCCEEDED(hr))
                        {
                            totalVolume += sessionVolume;
                            matchingSessions++;
                        }
                    }
                }
            }
        }

        if (matchingSessions > 0)
        {
            *volume = totalVolume / matchingSessions;
            return S_OK;
        }

        std::wcerr << L"No sessions matched assigned PIDs for " << cableId << std::endl;
        return E_FAIL;
    }

    static HRESULT SetCableVolume(IAudioSessionManager2* sessionManager, const std::vector<DWORD>& assignedProcesses, float volume, const std::wstring& cableId)
    {
        if (!sessionManager || volume < 0.0f || volume > 1.0f)
            return E_INVALIDARG;

        ComPtr<IAudioSessionEnumerator> sessionEnum;
        HRESULT hr = sessionManager->GetSessionEnumerator(&sessionEnum);
        if (FAILED(hr))
        {
            std::wcerr << L"Failed to get session enumerator for " << cableId << L": " << _com_error(hr).ErrorMessage() << std::endl;
            return hr;
        }

        int sessionCount = 0;
        hr = sessionEnum->GetCount(&sessionCount);
        if (FAILED(hr) || sessionCount == 0)
        {
            std::wcerr << L"No sessions found for " << cableId << std::endl;
            return FAILED(hr) ? hr : E_FAIL;
        }

        bool volumeSet = false;
        for (int i = 0; i < sessionCount; ++i)
        {
            ComPtr<IAudioSessionControl> sessionControl;
            hr = sessionEnum->GetSession(i, &sessionControl);
            if (FAILED(hr)) continue;

            ComPtr<IAudioSessionControl2> sessionControl2;
            hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);
            if (SUCCEEDED(hr))
            {
                DWORD pid = 0;
                hr = sessionControl2->GetProcessId(&pid);
                if (SUCCEEDED(hr) && pid != 0 && std::find(assignedProcesses.begin(), assignedProcesses.end(), pid) != assignedProcesses.end())
                {
                    ComPtr<ISimpleAudioVolume> simpleVolume;
                    hr = sessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&simpleVolume);
                    if (SUCCEEDED(hr))
                    {
                        hr = simpleVolume->SetMasterVolume(volume, nullptr);
                        if (SUCCEEDED(hr))
                        {
                            volumeSet = true;
                        }
                    }
                }
            }
        }

        return volumeSet ? S_OK : E_FAIL;
    }
};