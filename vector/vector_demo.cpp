#include <iostream>
#include <vector>
using namespace std;

void printVector(const vector<int>& vec) {
    for (int x : vec) {
        cout << x << " ";
    }
    cout << endl;
}

int main() {
    // 创建和初始化vector
    vector<int> numbers = {1, 2, 3, 4, 5};
    cout << "初始vector: ";
    printVector(numbers);

    // 添加元素
    numbers.push_back(6);
    cout << "push_back后: ";
    printVector(numbers);

    // 在指定位置插入
    numbers.insert(numbers.begin() + 2, 10);
    cout << "insert后: ";
    printVector(numbers);

    // 删除元素
    numbers.pop_back();
    cout << "pop_back后: ";
    printVector(numbers);

    // 使用迭代器删除
    numbers.erase(numbers.begin() + 1);
    cout << "erase后: ";
    printVector(numbers);

    // 获取vector信息
    cout << "大小: " << numbers.size() << endl;
    cout << "容量: " << numbers.capacity() << endl;
    cout << "首元素: " << numbers.front() << endl;
    cout << "末元素: " << numbers.back() << endl;

    return 0;
}
