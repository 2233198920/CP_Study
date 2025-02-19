#include <iostream>
#include <memory>
#include <string>

// Sample 类用于测试对象的构造和析构，帮助理解智能指针管理对象生命周期。
// 详细解释：构造函数中输出表示对象被创建，析构函数中输出表示对象被销毁。
class Sample {
public:
    std::string name;
    Sample(const std::string &n) : name(n) {
        std::cout << "构造 Sample: " << name << std::endl;
        // 解释：当创建 Sample 对象时，输出其名字，方便观察对象何时被创建。
    }
    ~Sample() {
        std::cout << "析构 Sample: " << name << std::endl;
        // 解释：当 Sample 对象生命周期结束时，析构函数会输出消息，方便观察对象销毁时刻。
    }
    void show() {
        std::cout << "Sample: " << name << std::endl;
        // 解释：简单方法用于显示 Sample 对象的信息。
    }
};

// Node 类用于演示循环引用问题，利用 weak_ptr 来避免循环引用导致内存泄露。
// 详细解释：
// 1. shared_ptr 用于管理对象生命周期，会自动记录引用计数；
// 2. weak_ptr 不增加引用计数，主要用于检查对象是否还存在。
class Node {
public:
    std::string value;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev; // 使用 weak_ptr 避免与 next 构成循环引用

    Node(const std::string &v) : value(v) {
        std::cout << "构造 Node: " << value << std::endl;
        // 解释：创建 Node 时输出消息，易于调试和理解对象间的关系。
    }
    ~Node() {
        std::cout << "析构 Node: " << value << std::endl;
        // 解释：销毁 Node 对象时输出消息，帮助验证是否存在内存泄露。
    }
};

int main() {
    std::cout << "=== unique_ptr 示例 ===" << std::endl;
    {
        // 创建 unique_ptr: unique_ptr 独占对象所有权，不能拷贝，只能转移
        std::unique_ptr<Sample> uPtr1 = std::make_unique<Sample>("Unique1");
        uPtr1->show(); // 调用 show() 展示对象信息

        // 转移所有权：使用 std::move 将 uPtr1 的所有权转移给 uPtr2，uPtr1 变为空
        std::unique_ptr<Sample> uPtr2 = std::move(uPtr1);
        if (!uPtr1) {
            std::cout << "uPtr1 已释放所有权" << std::endl;
            // 解释：检查 uPtr1 是否为空，以验证所有权已成功转移
        }
        uPtr2->show();
    } // 离开作用域，uPtr2 被销毁，对象内存被释放

    std::cout << "\n=== shared_ptr 与 weak_ptr 示例 ===" << std::endl;
    // 创建 shared_ptr: 允许多个 shared_ptr 指向同一对象，通过引用计数管理内存
    //std::shared_ptr<Sample> sPtr1 = std::make_shared<Sample>("Shared1");
    shared_ptr<Samplt> sPtr1(new Sample("Shared1"));
    {
        std::shared_ptr<Sample> sPtr2 = sPtr1; // 引用计数加1
        std::cout << "引用计数: " << sPtr1.use_count() << std::endl;
        // 解释：use_count() 显示当前对象被多少个 shared_ptr 管理
        sPtr2->show();
    } // 离开作用域，sPtr2 销毁，引用计数减1
    std::cout << "引用计数: " << sPtr1.use_count() << std::endl;

    // weak_ptr 示例: 不增加引用计数，仅用于观察对象是否存在
    std::weak_ptr<Sample> wPtr = sPtr1;
    if (auto temp = wPtr.lock()) {
        // lock() 返回一个 shared_ptr，若对象存在则有效，否则返回空指针
        std::cout << "weak_ptr 成功获得 shared_ptr: ";
        temp->show();
    }
    sPtr1.reset(); // 释放最后一个 shared_ptr，触发对象析构
    if (wPtr.expired()) {
        // expired() 检查 weak_ptr 指向的对象是否已被销毁
        std::cout << "weak_ptr 指向的对象已析构" << std::endl;
    }

    std::cout << "\n=== 避免循环引用示例 ===" << std::endl;
    // 创建两个 Node 对象，通过 shared_ptr 建立联系
    std::shared_ptr<Node> node1 = std::make_shared<Node>("Node1");
    std::shared_ptr<Node> node2 = std::make_shared<Node>("Node2");
    node1->next = node2; // node1 指向 node2，增加 node2 的引用计数
    node2->prev = node1; // node2 使用 weak_ptr 指向 node1，不增加引用计数，避免循环引用

    // 解释：如果两个对象互相持有 shared_ptr，可能导致引用计数无法归零，而使用 weak_ptr 可规避此问题

    return 0;
}
