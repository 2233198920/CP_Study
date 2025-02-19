#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::mutex mtx;

void usingLockGuard() {
    // 演示 lock_guard 使用，构造时加锁，析构时自动释放锁
    {
        std::lock_guard<std::mutex> lg(mtx);
        std::cout << "Using lock_guard: locked automatically and will unlock at scope end." << std::endl;
    }
    // 已自动解锁
}

void usingUniqueLock() {
    // 演示 unique_lock 使用，其灵活性: 可手动 unlock 和 re-lock
    std::unique_lock<std::mutex> ul(mtx);
    std::cout << "Using unique_lock: locked, now unlocking manually." << std::endl;
    ul.unlock();
    
    // 模拟一些无锁状态下的操作
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 重新加锁
    ul.lock();
    std::cout << "unique_lock re-locked, will unlock at scope end." << std::endl;
    // ul 在函数退出时会自动释放锁
}

int main() {
    std::thread t1(usingLockGuard);
    std::thread t2(usingUniqueLock);

    t1.join();
    t2.join();

    return 0;
}
