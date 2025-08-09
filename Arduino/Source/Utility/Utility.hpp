#pragma once 

namespace Utility
{
    class ProcessUtilityHelper {
    public:
        static std::vector<DWORD> FindProcessIds(const std::wstring& processName)
        {
            std::vector<DWORD> pids;
            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot == INVALID_HANDLE_VALUE)
            {
                return pids;
            }

            PROCESSENTRY32W pe32 = { sizeof(pe32) };
            if (Process32FirstW(hSnapshot, &pe32))
            {
                do
                {
                    if (_wcsicmp(pe32.szExeFile, processName.c_str()) == 0)
                    {
                        pids.push_back(pe32.th32ProcessID);
                    }
                }
                while (Process32NextW(hSnapshot, &pe32));
            }

            CloseHandle(hSnapshot);
            return pids;
        }

        static std::string GetProcessNameByPID(DWORD pid, bool useModuleName = false)
        {
            if (useModuleName)
            {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
                if (hProcess)
                {
                    char buffer[MAX_PATH];
                    if (GetModuleFileNameExA(hProcess, nullptr, buffer, MAX_PATH))
                    {
                        std::string fullpath = buffer;
                        auto pos = fullpath.find_last_of("\\/");
                        std::string name = (pos != std::string::npos) ? fullpath.substr(pos + 1) : fullpath;
                        CloseHandle(hProcess);
                        return name;
                    }
                    CloseHandle(hProcess);
                }
            }

            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot == INVALID_HANDLE_VALUE)
            {
                return "unknown";
            }

            PROCESSENTRY32W pe32 = { sizeof(pe32) };
            std::string result = "unknown";
            if (Process32FirstW(hSnapshot, &pe32))
            {
                do
                {
                    if (pe32.th32ProcessID == pid)
                    {
                        int size_needed = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, nullptr, 0, nullptr, nullptr);
                        if (size_needed > 0)
                        {
                            result.resize(size_needed - 1);
                            WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, &result[0], size_needed, nullptr, nullptr);
                        }
                        break;
                    }
                }
                while (Process32NextW(hSnapshot, &pe32));
            }

            CloseHandle(hSnapshot);
            return result;
        }

        static std::vector<std::string> GetAudioOutputProcesses()
        {
            std::vector<std::string> audioProcesses;
            std::unordered_map<DWORD, bool> pidMap;

            ComPtr<IMMDeviceEnumerator> deviceEnumerator;
            HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
            if (FAILED(hr))
            {
                return audioProcesses;
            }

            ComPtr<IMMDevice> defaultDevice;
            hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
            if (FAILED(hr))
            {
                return audioProcesses;
            }

            ComPtr<IAudioSessionManager2> sessionManager;
            hr = defaultDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, nullptr, (void**)&sessionManager);
            if (FAILED(hr))
            {
                return audioProcesses;
            }

            ComPtr<IAudioSessionEnumerator> sessionEnumerator;
            hr = sessionManager->GetSessionEnumerator(&sessionEnumerator);
            if (FAILED(hr))
            {
                return audioProcesses;
            }

            int sessionCount = 0;
            hr = sessionEnumerator->GetCount(&sessionCount);
            if (FAILED(hr) || sessionCount == 0)
            {
                return audioProcesses;
            }

            for (int i = 0; i < sessionCount; ++i)
            {
                ComPtr<IAudioSessionControl> sessionControl;
                hr = sessionEnumerator->GetSession(i, &sessionControl);
                if (SUCCEEDED(hr) && sessionControl)
                {
                    ComPtr<IAudioSessionControl2> sessionControl2;
                    hr = sessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&sessionControl2);
                    if (SUCCEEDED(hr) && sessionControl2)
                    {
                        DWORD pid = 0;
                        hr = sessionControl2->GetProcessId(&pid);
                        if (SUCCEEDED(hr) && pid != 0 && !pidMap[pid])
                        {
                            audioProcesses.push_back(GetProcessNameByPID(pid, true));
                            pidMap[pid] = true;
                        }
                    }
                }
            }

            return audioProcesses;
        }
    };

    inline std::vector<DWORD> FindProcessIds(const std::wstring& processName)
    {
        return ProcessUtilityHelper::FindProcessIds(processName);
    }

    inline std::string GetMainProcessNameByPID(DWORD pid)
    {
        return ProcessUtilityHelper::GetProcessNameByPID(pid, false);
    }

    inline std::string GetProcessNameByPID(DWORD pid)
    {
        return ProcessUtilityHelper::GetProcessNameByPID(pid, true);
    }

    inline std::string WStringToUtf8(const std::wstring& wstr)
    {
        if (wstr.empty()) return {};
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size_needed, nullptr, nullptr);
        return result;
    }

    inline void FlipBit(bool& bit)
    {
        bit = !bit;
    }

    inline void ResizeWindow(HWND hwnd, int width, int height)
    {
        RECT rect = { 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPED, FALSE);
        SetWindowPos(hwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
    }

    inline std::vector<std::string> GetAudioOutputProcesses()
    {
        return ProcessUtilityHelper::GetAudioOutputProcesses();
    }
} 