#include <iostream>
#include <future>
#include <thread>
#include <chrono>

// 一个耗时的计算函数
int heavy_computation(int x) {
    std::cout << "Heavy computation starts in thread: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3)); // 模拟耗时操作
    return x * x;
}

int main() {
    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;

    // 使用 std::async 创建异步任务
    std::future<int> result = std::async(std::launch::async, heavy_computation, 10);

    // 主线程可以执行其他任务
    std::cout << "Main thread is working...\n";

    // 等待异步任务完成并获取结果
    int value = result.get();
    std::cout << "Result from async task: " << value << std::endl;

    return 0;
}