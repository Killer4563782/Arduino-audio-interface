#pragma once 

class AudioSessionHelper {
public:
    static HRESULT EnumerateSessions(IAudioSessionManager2* sessionManager, std::vector<DWORD>& processIds)
    {
        if (!sessionManager) 
            return E_INVALIDARG;

        ComPtr<IAudioSessionEnumerator> sessionEnum;
        HRESULT hr = sessionManager->GetSessionEnumerator(&sessionEnum);
        if (FAILED(hr)) 
            return hr;

        int sessionCount = 0;
        hr = sessionEnum->GetCount(&sessionCount);
        if (FAILED(hr)) 
            return hr;

        processIds.clear();
        for (int i = 0; i < sessionCount; ++i)
        {
            ComPtr<IAudioSessionControl> sessionControl;
            hr = sessionEnum->GetSession(i, &sessionControl);
            if (SUCCEEDED(hr))
            {
                ComPtr<IAudioSessionControl2> sessionControl2;
                hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);
                if (SUCCEEDED(hr))
                {
                    DWORD pid = 0;
                    hr = sessionControl2->GetProcessId(&pid);
                    if (SUCCEEDED(hr) && pid != 0)
                    {
                        processIds.push_back(pid);
                    }
                }
            }
        }
        return S_OK;
    }
};