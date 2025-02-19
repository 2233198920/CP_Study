# 线程池详细代码解析

## 头文件说明
```cpp
#include <iostream>     // 用于输入输出
#include <vector>       // 存储线程的容器
#include <queue>        // 任务队列
#include <thread>       // 线程相关
#include <mutex>        // 互斥量
#include <condition_variable>  // 条件变量
#include <functional>   // std::function和std::bind
#include <future>       // std::future和std::packaged_task
```

## 构造函数详解
```cpp
ThreadPool(size_t threads) : stop(false) {
    for(size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] { ... });
    }
}
```
- `stop(false)`: 初始化stop标志为false
- `workers.emplace_back()`: 在线程vector末尾构造新线程
- `[this]`: lambda表达式捕获this指针，使lambda内部可以访问类成员

### 工作线程函数详解
```cpp
while(true) {    // 无限循环，持续处理任务
    std::function<void()> task;    // 定义任务函数对象
    {   // 作用域块，控制锁的生命周期
        std::unique_lock<std::mutex> lock(this->queue_mutex);   // RAII方式加锁
        this->condition.wait(lock, [this] {    // 等待条件满足
            return this->stop || !this->tasks.empty();
        });
        
        if(this->stop && this->tasks.empty()) {    // 停止条件判断
            return;
        }
        
        task = std::move(this->tasks.front());    // 移动语义获取任务
        this->tasks.pop();    // 移除队首任务
    }
    task();    // 执行任务
}
```

## 任务提交函数(enqueue)详解
```cpp
template<class F, class... Args>
auto enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::result_of<F(Args...)>::type>
```
- `template<class F, class... Args>`: 可变参数模板
- `F&& f`: 万能引用，接收任务函数
- `Args&&... args`: 可变参数包，接收函数参数
- `std::result_of<F(Args...)>::type`: 推导函数返回值类型

### 任务打包过程
```cpp
using return_type = typename std::result_of<F(Args...)>::type;
auto task = std::make_shared<std::packaged_task<return_type()>>(
    std::bind(std::forward<F>(f), std::forward<Args>(args)...)
);
```
- `std::make_shared`: 创建共享指针管理任务
- `std::packaged_task`: 包装任务以获取future
- `std::bind`: 绑定函数和参数
- `std::forward`: 完美转发保持参数类型

### 任务入队
```cpp
std::future<return_type> res = task->get_future();    // 获取future对象
{
    std::unique_lock<std::mutex> lock(queue_mutex);    // 加锁保护队列
    if(stop) throw std::runtime_error("enqueue on stopped ThreadPool");
    tasks.emplace([task](){ (*task)(); });    // 将任务添加到队列
}
condition.notify_one();    // 通知一个等待中的线程
return res;    // 返回future
```

## 析构函数详解
```cpp
~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;    // 设置停止标志
    }
    condition.notify_all();    // 通知所有等待的线程
    for(std::thread &worker: workers) {
        worker.join();    // 等待所有线程完成
    }
}
```

## 关键数据成员说明
```cpp
private:
    std::vector<std::thread> workers;    // 工作线程容器
    std::queue<std::function<void()>> tasks;    // 任务队列
    std::mutex queue_mutex;    // 队列互斥锁
    std::condition_variable condition;    // 条件变量
    bool stop;    // 停止标志
```

## 使用示例解析
```cpp
int main() {
    ThreadPool pool(4);    // 创建4个线程的线程池
    
    // 提交lambda表达式任务
    auto result1 = pool.enqueue([](int x) { return x * x; }, 10);
    auto result2 = pool.enqueue([](int x, int y) { return x + y; }, 2, 3);
    
    // 通过future获取结果
    std::cout << "result1: " << result1.get() << std::endl;    // 输出100
    std::cout << "result2: " << result2.get() << std::endl;    // 输出5
}
```

## 重要概念解释

### RAII锁管理
- `std::unique_lock<std::mutex>` 使用RAII技术管理锁的生命周期
- 作用域结束时自动解锁，避免忘记解锁导致死锁

### 条件变量同步
- `condition.wait()` 自动释放锁并等待
- `condition.notify_one()` 唤醒一个等待的线程
- `condition.notify_all()` 唤醒所有等待的线程

### 完美转发
- `std::forward` 保持参数的左值/右值特性
- 配合通用引用实现高效的参数传递

### 智能指针管理
- `std::make_shared` 异步任务的内存管理
- 确保任务对象在所有线程完成前不会被销毁
