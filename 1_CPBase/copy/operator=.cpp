#include <iostream>
#include <cstring>

class MyString {
private:
    char* data;
    size_t length;

public:
    // 构造函数
    MyString(const char* str = "") {
        length = std::strlen(str);
        data = new char[length + 1];
        std::strcpy(data, str);
    }

    // 析构函数
    ~MyString() {
        delete[] data;
    }

    // 打印字符串
    void print() const {
        std::cout << data << std::endl;
    }

    

    // 重载运算符(深拷贝)
    MyString& operator=(const MyString& str) {
        // 检查自我赋值
        if (this == &str)
            return *this;

        // 释放原有的内存资源
        delete[] data;

        // 执行深拷贝，分配资源并复制内容
        length = str.length;
        data = new char[length + 1];
        std::strcpy(data, str.data);

        return *this;
    }
};

int main() {
    MyString str1("Hello");
    MyString str2 = str1; // 浅拷贝

    str1.print(); // 输出 Hello
    str2.print(); // 输出 Hello

    // 修改 str1
    str1[0] = 'J';
    str1.print();   //输出 Jello
    str2.print();   //没有重载会输出 Jello, 重载后输出 Hello

    return 0;
}