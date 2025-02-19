#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> data_queue;
bool done = false;

// 生产者函数
void producer() {
    for(int i = 0; i < 10; ++i) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(i);
            std::cout << "生产者生产: " << i << std::endl;
        }
        cv.notify_one();  // 通知一个等待的消费者
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 生产结束，通知消费者
    {
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
    }
    cv.notify_one();
}

// 消费者函数
void consumer() {
    while(true) {
        std::unique_lock<std::mutex> lock(mtx);
        // 等待条件变量，防止虚假唤醒
        cv.wait(lock, []{ return !data_queue.empty() || done; });
        
        if(data_queue.empty() && done) {
            std::cout << "消费者结束" << std::endl;
            return;
        }
        
        int value = data_queue.front();   // 取出队列头元素
        data_queue.pop();
        std::cout << "消费者消费: " << value << std::endl;
    }
}

int main() {
    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);
    
    producer_thread.join();
    consumer_thread.join();
    
    return 0;
}
