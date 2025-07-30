#pragma once

class Threading {
public:
    ~Threading()
    {
        Shutdown();
    }
public: 
    void Post(const std::function<void()>& task, std::function<void()> callback = nullptr);
    void Shutdown();
public:
    void Run();

    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::queue<std::function<void()>> m_tasks;
    std::atomic<bool> m_running{ true };
};

inline Threading* g_Threading; 