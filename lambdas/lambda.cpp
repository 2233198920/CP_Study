#include <iostream>

//lambda:[捕获列表](参数列表) -> 返回类型 { 函数体 }


int main(){
int x = 10, y = 20;
auto lambda = [x, &y]() {
    std::cout << x << ", " << y << std::endl; // x 是值捕获，y 是引用捕获
    // x++;                      // 编译错误：x 是只读的
    y++;                         // 修改外部变量 y
};

auto modify = [x]()mutable{    // 使用 mutable 修饰 lambda，使得值捕获被修改
    x++;
    std::cout << x << std::endl;
};

lambda(); // 输出: 10, 20
std::cout << y << std::endl; // 输出: 21（y 被修改）
return 0;
}