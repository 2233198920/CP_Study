#include <iostream>
#include <thread>
#include <atomic>

// 对比普通变量和原子变量
std::atomic<int> atomic_counter{0};  // 原子计数器
int normal_counter{0};               // 普通计数器

void increment_atomic(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        atomic_counter++;  // 原子操作，无需互斥量
    }
}

void increment_normal(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        normal_counter++;  // 非原子操作，会产生竞争条件
    }
}

int main() {
    const int num_threads = 4;
    const int iterations_per_thread = 100000;
    
    std::thread atomic_threads[num_threads];
    std::thread normal_threads[num_threads];

    // 创建使用原子变量的线程
    for (int i = 0; i < num_threads; ++i) {
        atomic_threads[i] = std::thread(increment_atomic, iterations_per_thread);
    }

    // 创建使用普通变量的线程
    for (int i = 0; i < num_threads; ++i) {
        normal_threads[i] = std::thread(increment_normal, iterations_per_thread);
    }

    // 等待所有线程完成
    for (int i = 0; i < num_threads; ++i) {
        atomic_threads[i].join();
        normal_threads[i].join();
    }

    std::cout << "Expected value: " << num_threads * iterations_per_thread << std::endl;
    std::cout << "Atomic counter value: " << atomic_counter << std::endl;
    std::cout << "Normal counter value: " << normal_counter << std::endl;

    return 0;
}
