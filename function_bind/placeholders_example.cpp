#include <functional>
#include <iostream>

void display(int a, int b, int c, int d) {
    std::cout << "a=" << a << ", b=" << b 
              << ", c=" << c << ", d=" << d << std::endl;
}

int main() {
    // 示例1：简单参数重排序
    auto bind1 = std::bind(display,
        std::placeholders::_2,  // 第2个参数放在第1个位置
        std::placeholders::_1,  // 第1个参数放在第2个位置
        std::placeholders::_3,  // 第3个参数放在第3个位置
        std::placeholders::_4   // 第4个参数放在第4个位置
    );
    bind1(10, 20, 30, 40);     // 输出: a=20, b=10, c=30, d=40

    // 示例2：固定部分参数
    auto bind2 = std::bind(display,
        100,                    // 固定第1个参数为100
        std::placeholders::_1,  // 使用调用时的第1个参数
        std::placeholders::_2,  // 使用调用时的第2个参数
        200                     // 固定第4个参数为200
    );
    bind2(10, 20);             // 输出: a=100, b=10, c=20, d=200

    return 0;
}
