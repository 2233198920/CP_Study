#include <iostream>
#include "calc_add.h"
#include "calc_multiply.h"

int main() {
    int a = 5, b = 3;
    
    std::cout << "加法运算: " << a << " + " << b << " = " << add(a, b) << std::endl;
    std::cout << "乘法运算: " << a << " × " << b << " = " << multiply(a, b) << std::endl;
    
    return 0;
}
