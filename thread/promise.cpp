#include <iostream>
#include <future>
#include <thread>
#include <stdexcept>

// 模拟数据处理函数
void process_data(std::promise<std::string>&& promise, bool success) {
    try {
        std::cout << "Worker thread starting...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));  // 模拟耗时操作
        
        if (!success) {
            throw std::runtime_error("数据处理失败");
        }
        
        promise.set_value("处理完成");  // 设置结果
    }
    catch (...) {
        promise.set_exception(std::current_exception());  // 传递异常
    }
}

int main() {
    // 成功场景
    {
        std::cout << "=== 成功场景测试 ===\n";
        std::promise<std::string> promise;
        std::future<std::string> future = promise.get_future();
        
        std::thread worker(process_data, std::move(promise), true);
        
        std::cout << "主线程等待结果...\n";
        try {
            std::string result = future.get();  // 获取结果
            std::cout << "获得结果: " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "捕获异常: " << e.what() << std::endl;
        }
        
        worker.join();
    }

    // 失败场景
    {
        std::cout << "\n=== 失败场景测试 ===\n";
        std::promise<std::string> promise;
        std::future<std::string> future = promise.get_future();
        
        std::thread worker(process_data, std::move(promise), false);
        
        std::cout << "主线程等待结果...\n";
        try {
            std::string result = future.get();
            std::cout << "获得结果: " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "捕获异常: " << e.what() << std::endl;
        }
        
        worker.join();
    }

    return 0;
}
