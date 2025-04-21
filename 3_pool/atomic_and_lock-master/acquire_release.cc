#include <atomic>
#include <thread>
#include <assert.h>
#include <iostream>

std::atomic<bool> x,y;
std::atomic<int> z;

void write_x_then_y()
{
    x.store(true,std::memory_order_relaxed);  // 1 
    y.store(true,std::memory_order_release);  // 2   y = true x= true
}

void read_y_then_x()
{
    while(!y.load(std::memory_order_acquire));  // 3 自旋，等待y被设置为true
    if(x.load(std::memory_order_relaxed))  // 4
        ++z;
}
// z 能确保一定能读到1

int main()
{
    x=false;
    y=false;
    z=0;
    std::thread a(write_x_then_y);
    std::thread b(read_y_then_x);
    a.join();
    b.join();
    std::cout << z.load(std::memory_order_relaxed) << std::endl;
    return 0;
}