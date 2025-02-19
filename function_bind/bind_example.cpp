#include <functional>
#include <iostream>

class Multiplier {
public:
    double multiply(double x, double y) {
        return x * y;
    }
    
    int multiplyAndAdd(int a, int b, int c) {
        return a * b + c;
    }
};

void print(int a, int b, int c) {
    std::cout << "a=" << a << ", b=" << b << ", c=" << c << std::endl;
}

int main() {
    Multiplier m;
    
    // 1. 绑定类成员函数
    auto bind1 = std::bind(&Multiplier::multiply, &m, std::placeholders::_1, 2);
    std::cout << "Bind with fixed parameter: " << bind1(4) << std::endl;  // 4 * 2
    
    // 2. 参数重排序
    auto bind2 = std::bind(&Multiplier::multiplyAndAdd, &m,
                          std::placeholders::_2,  // a
                          std::placeholders::_1,  // b
                          std::placeholders::_3); // c
    std::cout << "Parameter reordering: " << bind2(3, 2, 1) << std::endl;  // 2 * 3 + 1
    
    // 3. 绑定普通函数并固定部分参数
    auto bind3 = std::bind(print, 42, std::placeholders::_1, 0);
    bind3(10);  // 输出: a=42, b=10, c=0
    
    return 0;
}
