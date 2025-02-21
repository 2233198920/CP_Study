
#include <iostream>

int main(){
    int a = 5;

    int &ref_a_left = a;
    int &&ref_a_right = std::move(a);
    ref_a_right = 6;
    std::cout << a << ","<< ref_a_left <<"," << ref_a_right<< std::endl;
    std::cout << &a << ","<< &ref_a_left <<"," << &ref_a_right<< std::endl;
    return 0;
}