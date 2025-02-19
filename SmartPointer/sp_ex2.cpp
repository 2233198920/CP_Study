#include <iostream>
#include <memory>
#include <string>

// Sample 类用于展示对象生命周期，方便理解智能指针管理内存的方式
class Sample {
public:
    std::string name;
    Sample(const std::string &n) : name(n) {
        std::cout << "Sample对象 " << name << " 被构造" << std::endl;
    }
    ~Sample() {
        std::cout << "Sample对象 " << name << " 被析构" << std::endl;
    }
    void print() {
        std::cout << "Sample: " << name << std::endl;
    }
};

int main() {
    std::cout << "== unique_ptr 特性示例 ==" << std::endl;
    {
        // unique_ptr 拥有独占所有权，不允许拷贝，可通过 std::move 转移所有权
        std::unique_ptr<Sample> uptr1 = std::make_unique<Sample>("UniqueSample");
        uptr1->print();
        // 转移 uptr1 的所有权给 uptr2 后，uptr1 变为空
        std::unique_ptr<Sample> uptr2 = std::move(uptr1);
        if (!uptr1)
            std::cout << "uptr1 不再拥有对象" << std::endl;
        uptr2->print();
    } // 离开作用域，uptr2 自动析构，释放对象内存

    std::cout << "\n== shared_ptr 特性示例 ==" << std::endl;
    {
        // shared_ptr 允许多个指针共享同一对象，采用引用计数管理内存
        std::shared_ptr<Sample> sptr1 = std::make_shared<Sample>("SharedSample");
        std::cout << "引用计数: " << sptr1.use_count() << std::endl; // 初始为1
        {
            // 拷贝构造增加引用计数
            std::shared_ptr<Sample> sptr2 = sptr1;
            std::cout << "引用计数: " << sptr1.use_count() << std::endl; // 增加到2
            sptr2->print();
        } // sptr2 离开作用域，引用计数减1
        std::cout << "引用计数: " << sptr1.use_count() << std::endl; // 又降回1
        sptr1->print();
    } // sptr1 离开作用域，引用计数归零，对象被析构

    std::cout << "\n== weak_ptr 特性示例 ==" << std::endl;
    {
        // 创建 shared_ptr 对象
        std::shared_ptr<Sample> sharedSample = std::make_shared<Sample>("WeakDemoSample");
        // weak_ptr 观察对象，不增加引用计数
        std::weak_ptr<Sample> wptr = sharedSample;
        std::cout << "引用计数 (通过 sharedSample): " << sharedSample.use_count() << std::endl; // 应为1

        // 使用 lock() 尝试获取 shared_ptr
        if (auto lockedPtr = wptr.lock()) {
            std::cout << "通过 weak_ptr 获得 shared_ptr，调用 print():" << std::endl;
            lockedPtr->print();
        } else {
            std::cout << "无法获得对象，可能已被析构" << std::endl;
        }
        // 重置 shared_ptr，释放对象
        sharedSample.reset();
        if (wptr.expired())
            std::cout << "weak_ptr 指向的对象已被析构" << std::endl;
    }
    return 0;
}
