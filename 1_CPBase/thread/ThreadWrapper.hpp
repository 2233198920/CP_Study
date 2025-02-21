#ifndef THREAD_WRAPPER_HPP
#define THREAD_WRAPPER_HPP

#include <thread>
#include <utility>

namespace thread_utils {

class ThreadWrapper {
public:
    // 使用模板构造函数启动线程
    template<typename Function, typename... Args>
    explicit ThreadWrapper(Function&& f, Args&& ... args)
        : m_thread(std::forward<Function>(f), std::forward<Args>(args)...) {}

    // 禁止拷贝
    ThreadWrapper(const ThreadWrapper&) = delete;
    ThreadWrapper& operator=(const ThreadWrapper&) = delete;
    
    // 允许移动
    ThreadWrapper(ThreadWrapper&& other) noexcept
        : m_thread(std::move(other.m_thread)) {}
    ThreadWrapper& operator=(ThreadWrapper&& other) noexcept {
        if (this != &other) {
            if (m_thread.joinable())
                m_thread.join();
            m_thread = std::move(other.m_thread);
        }
        return *this;
    }
    
    ~ThreadWrapper(){
        if(m_thread.joinable()){
            m_thread.join();
        }
    }
    
    void join(){
        if(m_thread.joinable()){
            m_thread.join();
        }
    }
    
    void detach(){
        if(m_thread.joinable()){
            m_thread.detach();
        }
    }

private:
    std::thread m_thread;
};

} // namespace thread_utils

#endif // THREAD_WRAPPER_HPP
