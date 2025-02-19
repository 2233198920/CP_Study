#include <iostream>

// 浅拷贝示例：仅复制指针地址，注意这里省略了解析构释放以避免重复释放
class ShallowCopy {
public:
    int* data;
    ShallowCopy(int value) { data = new int(value); }   //new:动态分配内存
    ShallowCopy(const ShallowCopy &other) { data = other.data; }
    void setValue(int val) { *data = val; }
    int getValue() const { return *data; }
};

// 深拷贝示例：为复制对象分配新的内存并复制数据内容
class DeepCopy {
public:
    int* data;
    DeepCopy(int value) { data = new int(value); }
    DeepCopy(const DeepCopy &other) { data = new int(*other.data); }
    ~DeepCopy() { delete data; }
    void setValue(int val) { *data = val; }
    int getValue() const { return *data; }
};

int main(){
    // 测试浅拷贝
    ShallowCopy shallowOrig(10);
    ShallowCopy shallowCopy = shallowOrig;
    shallowOrig.setValue(20);
    std::cout << "浅拷贝 - 原始值: " << shallowOrig.getValue() 
              << ", 复制值: " << shallowCopy.getValue() << std::endl;
              
    // 测试深拷贝
    DeepCopy deepOrig(10);
    DeepCopy deepCopy = deepOrig;
    deepOrig.setValue(20);
    std::cout << "深拷贝 - 原始值: " << deepOrig.getValue() 
              << ", 复制值: " << deepCopy.getValue() << std::endl;
              
    return 0;
}
