#include <iostream>
#include <string>
#include <iomanip>
#include "SkipList.h"
using namespace std;
using namespace fm;

default_random_engine engine(random_device{}());
uniform_int_distribution<int> distribution(1, 1000);

void testInsert() {
  SkipList<int, string> skip_list;

  for (int i = 0; i < 100; ++i)
    // CMakeLists.txt设置了隐式类型转换警告，所以这里会进行告警，但不用理它
    skip_list.insert(make_pair(distribution(engine), "hello"));
  for (int i = 0; i < 100; ++i) {
    string tmp("world");
    skip_list.emplace(distribution(engine), std::move(tmp));
  }
  cout << "current skip list size: " << skip_list.size() << endl;
}

void testErase() {
  SkipList<int, string> skip_list;

  for (int i = 0; i < 100; ++i)
    skip_list.emplace(distribution(engine), "women");
  cout << "skip list size before erase: " << skip_list.size() << endl;
  for (int i = 300; i < 500; ++i) {
    auto iter = skip_list.find(i);
    if (iter != skip_list.end())
      skip_list.erase(iter);
  }
  cout << "skip list size after erase: " << skip_list.size() << endl;
}

void testMove() {
  SkipList<int, vector<int>> skip_list;

  skip_list[32] = vector<int>{2, 3, 4, 5};
  skip_list[35] = vector<int>{23, 432, 543, 2};
  cout << "skip list size before moving: " << skip_list.size() << endl;
  decltype(skip_list) skip_list_move(std::move(skip_list));
  cout << "old skip list size after moving: " << skip_list.size() << endl;
  cout << "new skip list size after moving: " << skip_list_move.size() << endl;
};

void doubleLine() {
  cout << "=================================" << endl;
}

int main() {
  // 使用gTest进行单元测试会更好
  testInsert();
  doubleLine();
  testErase();
  doubleLine();
  testMove();
  return 0;
}