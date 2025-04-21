#include "LockedQueue.h"

#include <thread>
#include <iostream>

class Check1 {
public:
    bool Process(int val) {
        return val%2 == 0;
    }
};

class Check2 {
public:
    bool Process(int val) {
        return val%2 == 1;
    }
};

int main() {

    LockedQueue<int> queue;

    std::thread pd1([&]() {
        queue.add(1);
        queue.add(2);
        queue.add(3);
        queue.add(4);
    });

    std::thread pd2([&]() {
        queue.add(5);
        queue.add(6);
        queue.add(7);
        queue.add(8);
    });

    std::thread cs1([&]() {
        int ele;
        Check1 check;
        while(queue.next(ele, check)) {
            std::cout << std::this_thread::get_id() << " : pop " << ele << std::endl;
        }
    });
    std::thread cs2([&]() {
        int ele;
        Check2 check;
        while(queue.next(ele, check)) {
            std::cout << std::this_thread::get_id() << " : pop " << ele << std::endl;
        }
    });

    pd1.join();
    pd2.join();
    cs1.join();
    cs2.join();
    
    return 0;
}