#pragma once 

namespace Utility
{
	inline std::vector<DWORD> FindProcessIds(const std::wstring& processName)
    {
        std::vector<DWORD> pids;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
        {
            std::wcerr << L"Failed to create process snapshot: " << GetLastError() << std::endl;
            return pids;
        }

        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        if (!Process32FirstW(hSnapshot, &pe32))
        {
            CloseHandle(hSnapshot);
            std::wcerr << L"Failed to enumerate processes: " << GetLastError() << std::endl;
            return pids;
        }

        do
        {
            if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0)
            {
                pids.push_back(pe32.th32ProcessID);
            }
        }
        while (Process32NextW(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
        return pids;
    }

    inline std::wstring GetMainProcessNameByPID(DWORD pid)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
            return L"";

        PROCESSENTRY32W entry{};
        entry.dwSize = sizeof(entry);

        std::wstring result;

        if (Process32FirstW(snapshot, &entry))
        {
            do
            {
                if (entry.th32ProcessID == pid)
                {
                    result = entry.szExeFile;
                    break;
                }
            }
            while (Process32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
        return result;
    }

    inline std::string GetProcessNameByPID(DWORD pid)
    {
        std::string name = "unknown"; 

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid); 
        if (hProcess)
        {
            char buffer[MAX_PATH]; 
            if (GetModuleFileNameExA(hProcess, nullptr, buffer, MAX_PATH))
            {
                std::string fullpath = buffer; 
                auto pos = fullpath.find_last_of("\\/");
                if (pos != std::string::npos)
                {
                    name = fullpath.substr(pos + 1); 
                }
                else name = fullpath;
            }
            CloseHandle(hProcess); 
        }

        return name; 
    }

    inline std::vector<std::string> GetAudioOutputProcesses()
    {
        std::vector<std::string> audioProcesses; 
        HRESULT hr; 

        CoInitialize(nullptr); 

        IMMDeviceEnumerator* deviceEnumerator = nullptr; 
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
        if (FAILED(hr))
        {
			return audioProcesses;
        }

        IMMDevice* defaultdevice = nullptr; 
		hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultdevice);
        if (FAILED(hr))
        {
            deviceEnumerator->Release(); 
            return audioProcesses;
        }

		IAudioSessionManager2* sessionManager = nullptr;
        hr = defaultdevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, nullptr, (void**)&sessionManager);
        if (FAILED(hr))
        {
            defaultdevice->Release();
            deviceEnumerator->Release();
            return audioProcesses;
        }

		IAudioSessionEnumerator* sessionEnumerator = nullptr;
		hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
        if (FAILED(hr))
        {
            sessionManager->Release();
            defaultdevice->Release();
            deviceEnumerator->Release();
            return audioProcesses;
        }

		int sessionCount = 0;
        sessionEnumerator->GetCount(&sessionCount); 

        std::unordered_map<DWORD, bool> pidMap; 
                                                                           
        for (int i = 0; i < sessionCount; i++)
        {
            IAudioSessionControl* SessionControl = nullptr; 
			hr = sessionEnumerator->GetSession(i, &SessionControl);
            if (SUCCEEDED(hr) && SessionControl)
            {
                IAudioSessionControl2* SessionControl2 = nullptr; 
				hr = SessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&SessionControl2);
                if (SUCCEEDED(hr) && SessionControl2)
                {
                    DWORD pid = 0; 
                    hr = SessionControl2->GetProcessId(&pid); 
                    if (SUCCEEDED(hr) && pid != 0 && pidMap.find(pid) == pidMap.end())
                    {
                        std::string process_name = GetProcessNameByPID(pid);
						audioProcesses.push_back(process_name);
						pidMap[pid] = true;
                    }
                    SessionControl2->Release(); 
                }
                SessionControl->Release(); 
            }
        }

		sessionEnumerator->Release();
        sessionManager->Release();
        defaultdevice->Release();
        deviceEnumerator->Release();
        CoUninitialize();
		return audioProcesses;
    }
}