#include "Threading.hpp"

void Threading::Post(const std::function<void()>& task, std::function<void()> callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_running)
    {
        return; 
    }

    m_tasks.emplace(std::move(task));
    bool was_idle = !m_thread.joinable();

    if (was_idle)
    {
        m_thread = std::thread(&Threading::Run, this);
#ifdef _WIN32
        SetThreadPriority(m_thread.native_handle(), THREAD_PRIORITY_IDLE);
#endif
    }

    m_cv.wait(lock, [this]
    {
        return m_tasks.empty() || !m_running;
    });
}

void Threading::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
        while (!m_tasks.empty())
        {
            m_tasks.pop();
        }
    }
    m_cv.notify_one();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void Threading::Run()
{
    std::function<void()> task;
    std::function<void()> callback;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_running || m_tasks.empty())
        {
            m_cv.notify_one(); 
            return;
        }

        task = std::move(m_tasks.front());
        callback = m_tasks.front();
        m_tasks.pop();
    }

    task();
    if (callback)
    {
        callback();
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cv.notify_one();
    }
}