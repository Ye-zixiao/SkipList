#include <iostream>
#include <string>
#include <iomanip>
#include "SkipList.h"
using namespace std;
using namespace fm;

void testSkipList() {
  default_random_engine engine(random_device{}());
  uniform_int_distribution<int> dist(1, 10000);
  SkipList<int, string> skip_list;
  string str("hello world");

  for (int i = 0; i < 50000; ++i)
    skip_list.emplace(dist(engine), str);
  for (int i = 0; i < 50000; ++i) {
    string tmp("hello asian");
    skip_list.insert(make_pair(dist(engine), std::move(tmp)));
  }
  cout << "current skip list size: " << skip_list.size() << endl;

  for (int i = 200; i < 350; ++i)
    skip_list.erase(i);
  for (int i = 300; i < 400; ++i) {
    if (skip_list.contains(i))
      cout << "key " << i << " exits" << endl;
    else
      cout << "Key " << i << " is not exited" << endl;
  }
  cout << "current skip list size: " << skip_list.size() << endl;
  auto iter_of_20 = skip_list.find(20);
  if (iter_of_20 != skip_list.end()) {
    for (auto iter = skip_list.begin(); iter != iter_of_20; ++iter)
      cout << "key: " << setw(2) << iter->key() << ", value: "
           << iter->value() << endl;
  }
}

void testAnother() {
  SkipList<int, string> skip_list;
  skip_list[32] = "hello world";
  cout << "\nskip list size: " << skip_list.size() << endl;
  cout << boolalpha << skip_list.contains(32) << endl;
  auto iter = skip_list.find(32);
  if (iter != skip_list.end())
    cout << iter->key() << endl;
  for (const auto &elem:skip_list)
    cout << elem.key() << ' ' << elem.value() << endl;

  SkipList<int, string> skip_list1(std::move(skip_list));
  cout << "old skip list size: " << skip_list.size() << endl;
  cout << "new skip list size: " << skip_list1.size() << endl;
}

int main() {
  // 使用gTest来进行单元测试会更好
  testSkipList();
  testAnother();
  return 0;
}