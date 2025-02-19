# std::function 和 std::bind 使用示例

## std::function 示例

本示例展示了 C++ 中 std::function 的三种常见使用方式。

### 三种函数存储方式

1. 存储普通函数
```cpp
std::function<int(int, int)> f1 = normalFunc;
```

2. 存储成员函数
```cpp
std::function<int(int, int)> f2 = std::bind(&Calculator::memberFunc, &calc, 
                                           std::placeholders::_1, 
                                           std::placeholders::_2);
```

3. 存储 lambda 表达式
```cpp
std::function<int(int, int)> f3 = [](int a, int b) { return a - b; };
```

## std::bind 示例

std::bind 提供了以下功能：

1. 将可调用对象和参数绑定成一个新的可调用对象
2. 支持参数重排序
3. 支持固定部分参数值

### 基本用法
```cpp
// 绑定成员函数并固定部分参数
// 为什么需要传两个参数：成员函数总是需要一个对象实例来调用
auto bind1 = std::bind(&Multiplier::multiply, &m, std::placeholders::_1, 2);

// 参数重排序
auto bind2 = std::bind(&Multiplier::multiplyAndAdd, &m,
                      std::placeholders::_2,
                      std::placeholders::_1,
                      std::placeholders::_3);

// 绑定普通函数
auto bind3 = std::bind(print, 42, std::placeholders::_1, 0);
```

### 占位符说明
- std::placeholders::_1 表示第一个参数
- std::placeholders::_2 表示第二个参数
- 以此类推...

### 注意事项：
1. 占位符的编号必须连续，从 _1 开始
2. 占位符的数量决定了绑定后函数需要的参数个数
3. 固定值参数在绑定时就确定，调用时不能改变


