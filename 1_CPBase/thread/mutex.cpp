#include <iostream>
#include <thread>
#include <mutex>


std::mutex mtx;       // 全局互斥量
int count = 0;        // 全局变量

void count_10k() {
    for(int i = 0; i < 10000; ++i){
    //使用try_lock()
    // if(mtx.try_lock()){
    //     ++count;
    //     mtx.unlock();
    // }

    //使用lock()
    {
        // std::lock_guard<std::mutex> lock(mtx);
        // std::unique_lock<std::mutex> lock(mtx);
        mtx.lock();
        ++count;
        mtx.unlock();
    }
    }
}

int main() {
    std::thread threads[10];
    for(int i = 0; i < 10; ++i)
        threads[i] = std::thread(count_10k);
    
    for(auto& th : threads) th.join();
    std::cout << count << std::endl;

    return 0;
}
