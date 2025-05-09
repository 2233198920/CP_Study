# C++ Lambda 函数详细说明

## 1. 介绍
Lambda 表达式是 C++11 引入的一种轻量级的匿名函数，常用于编写内联函数，提高代码可读性和简洁性。它们支持捕获外部变量，并可以直接在定义处实现操作逻辑。

## 2. 基本语法
基本语法格式如下：
```
auto [函数名] = [capture](parameters) -> return_type {
    // 函数体
};
```
- **capture**：捕获列表，用于指定哪些外部变量可以在 lambda 内部使用。
- **parameters**：参数列表。
- **return_type**：返回值类型（可选，通常由编译器自动推导）。
- **函数体**：Lambda 表达式的执行代码。

## 3. 捕获外部变量
捕获列表支持以下几种方式：
- `[ ]`：不捕获任何变量。
- `[a, b]`：按值捕获 a、b。
- `[&a, &b]`：按引用捕获 a、b。
- `[=]`：默认按值捕获所有变量。
- `[&]`：默认按引用捕获所有变量。

