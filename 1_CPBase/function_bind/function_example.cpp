#include <functional>
#include <iostream>

// 普通函数
int normalFunc(int a, int b) {
    return a + b;
}

class Calculator {
public:

    // 构造函数
    Calculator(std::string name) : name_(name) {}
    // Calculator(std::string name) {
    //     name_ = name;    // 这种方式效率较低
    // }

    int memberFunc(int a, int b) {
        return a * b;
    }

private:
    std::string name_;

};

int main() {
    // 1. 存储普通函数
    std::function<int(int, int)> f1 = normalFunc;
    std::cout << "Normal function: " << f1(5, 3) << std::endl;

    // 2. 存储成员函数
    Calculator calc("calc");
    std::function<int(Calculator&, int, int)> f2 = &Calculator::memberFunc;
    std::cout << "Member function: " << f2(calc,5, 3) << std::endl;

    // 3. 存储 lambda 表达式
    std::function<int(int, int)> f3 = [](int a, int b) { return a - b; };
    std::cout << "Lambda function: " << f3(5, 3) << std::endl;

    return 0;
}
