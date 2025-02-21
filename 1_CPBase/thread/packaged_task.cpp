#include <iostream>
#include <future>
#include <thread>

// 一个简单的函数，用于演示
int compute(int a, int b) {
    return a + b;
}

int main() {
    // 创建一个packaged_task对象，封装compute函数
    std::packaged_task<int(int, int)> task(compute);

    // 获取与packaged_task关联的future对象
    std::future<int> result = task.get_future();

    // 创建一个线程，异步执行封装的任务
    std::thread t(std::move(task), 2, 3);

    // 在主线程中执行其他任务...
    std::cout << "Main thread is working...\n";
    // 等待异步任务完成并获取结果
    int value = result.get();
    std::cout << "Result from async task: " << value << std::endl;

    // 确保线程结束
    t.join();

    return 0;
}