//
// Created by Ye-zixiao on 2021/8/4.
//

#ifndef SKIPLIST__SKIPLIST_H_
#define SKIPLIST__SKIPLIST_H_

#include <random>
#include <memory>
#include <cassert>

namespace fm {

template<typename Key, typename Value>
struct SkipNode {
  std::pair<const Key, Value> key_value;

  template<typename K, typename V>
  SkipNode(K &&key, V &&value, size_t height) :
      key_value(std::make_pair(std::forward<K>(key), std::forward<V>(value))),
      next_(new SkipNode *[height]{}) {}

  // movable is not important
  SkipNode(const SkipNode &) = delete;
  SkipNode &operator=(const SkipNode &) = delete;
  ~SkipNode() = default;

  void setNext(size_t height, SkipNode *next) {
    assert(height >= 0);
    next_[height] = next;
  }

  SkipNode *getNext(size_t height) const {
    assert(height >= 0);
    return next_[height];
  }

  const Key &key() const { return key_value.first; }
  const Value &value() const { return key_value.second; }
  Value &value() { return key_value.second; }

 private:
  std::unique_ptr<SkipNode *[]> next_;
};

template<typename Node>
class SkipListIterator {
 public:
  SkipListIterator() : node_(nullptr) {}
  explicit SkipListIterator(Node *node) : node_(node) {}

  SkipListIterator &operator++() {
    assert(node_);
    node_ = node_->getNext(0);
    return *this;
  }

  SkipListIterator operator++(int) {
    assert(node_);
    auto tmp = SkipListIterator(node_);
    node_ = node_->getNext(0);
    return tmp;
  }

  bool operator==(const SkipListIterator &rhs) const { return node_ == rhs.node_; }
  bool operator!=(const SkipListIterator &rhs) const { return node_ != rhs.node_; }
  Node *operator->() const { return node_; }
  Node &operator*() const { return *node_; }
  operator bool() const { return node_; }

 private:
  Node *node_;
};

template<typename Key, typename Value,
    typename Comparator=std::less<Key>,
    typename Allocator=std::allocator<SkipNode<Key, Value>>>
class SkipList {
 public:
  using SizeType = size_t;
  using KeyType = Key;
  using MappedType = Value;
  using NodeType = SkipNode<KeyType, MappedType>;
  using Iterator = SkipListIterator<NodeType>;
  using AllocType = Allocator;

 public:
  SkipList() :
      comparator_(Comparator()),
      engine_(std::random_device{}()),
      head_(createNode(Key{}, Value{}, kMaxHeight)),
      current_height_(0),
      size_(0) {}

  ~SkipList() {
    auto *curr = head_;
    while (curr) {
      auto *tmp = curr;
      curr = curr->getNext(0);
      destroyNode(tmp);
    }
  }

  SkipList(SkipList &&rhs) noexcept:
      comparator_(Comparator{}),
      engine_(std::random_device{}()),
      head_(rhs.head_),
      current_height_(rhs.current_height_),
      size_(rhs.size_) {
    rhs.head_ = nullptr;
    rhs.current_height_ = 0;
    rhs.size_ = 0;
  }

  SkipList &operator=(SkipList &&rhs) noexcept {
    if (this != &rhs) {
      SkipList tmp(std::move(rhs));
      this->swap(tmp);
    }
    return *this;
  }

 private:
  static constexpr SizeType kMaxHeight = 16;

  static Allocator skip_node_allocator_;

  Comparator comparator_;
  std::default_random_engine engine_;
  SkipNode<Key, Value> *head_;
  SizeType current_height_;
  SizeType size_;

 public:
  template<typename K, typename V>
  std::pair<Iterator, bool> emplace(K &&key, V &&value) {
    // 查找相应的各级前驱指针
    NodeType *prev[kMaxHeight]{};
    NodeType *node = findLessThan(key, prev);
    assert(node);

    // 若该键值已经存在，则失败返回
    if (node->getNext(0)) {
      const KeyType &next_key = node->getNext(0)->key();
      if (!comparator_(next_key, key) && !comparator_(key, next_key))
        return std::make_pair(Iterator(), false);
    }

    // 创建新的节点
    SizeType new_height = getRandomHeight();
    NodeType *new_node = createNode(std::forward<K>(key),
                                    std::forward<V>(value), new_height);
    // 更新相应的指针，进行连接
    for (SizeType height = 0; height < std::min(current_height_, new_height); ++height) {
      new_node->setNext(height, prev[height]->getNext(height));
      prev[height]->setNext(height, new_node);
    }
    if (new_height > current_height_) {
      for (SizeType height = current_height_; height < new_height; ++height)
        prev[height]->setNext(height, new_node);
      current_height_ = new_height;
    }
    ++size_;
    return std::make_pair(Iterator(new_node), true);
  }

  template<typename K, typename V>
  std::pair<Iterator, bool> insert(std::pair<K, V> &&key_value) {
    return emplace(std::forward<K>(key_value.first),
                   std::forward<V>(key_value.second));
  }

  bool erase(const KeyType &key) {
    // 查找各级前驱指针
    NodeType *prev[kMaxHeight]{};
    NodeType *node = findLessThan(key, prev);
    assert(node);

    // 若指定键key并不存在，则失败返回
    NodeType *curr = node->getNext(0);
    if (!curr || comparator_(key, curr->key()) || comparator_(curr->key(), key))
      return false;

    // 重新调整待删节点各级前驱节点的next指针，并递减跳表大小
    for (SizeType height = 0; height < kMaxHeight; ++height) {
      if (prev[height]->getNext(height) != curr) break;
      prev[height]->setNext(height, curr->getNext(height));
    }
    destroyNode(curr);
    --size_;

    // 调整当前最大高度
    while (current_height_ > 0 && !head_->getNext(current_height_ - 1))
      --current_height_;
    return true;
  }

  bool erase(Iterator iter) {
    return erase(iter->key());
  }

  MappedType &operator[](const KeyType &key) {
    auto iter = find(key);
    if (iter == end())
      // 若找不到该元素，则进行插入
      iter = this->template emplace(key, MappedType{}).first;
    return iter->value();
  }

  Iterator find(const KeyType &key) const {
    NodeType *prev_node = findLessThan(key, nullptr);
    auto *node = prev_node->getNext(0);
    if (node && !comparator_(key, node->key()) && !comparator_(node->key(), key))
      return Iterator(node);
    return Iterator();
  }

  void swap(SkipList &rhs) noexcept {
    if (this != &rhs) {
      using std::swap;
      swap(rhs.current_height_, this->current_height_);
      swap(rhs.head_, this->head_);
      swap(rhs.size_, this->size_);
    }
  }

  bool contains(const KeyType &key) const { return find(key) != end(); }
  SizeType counts(const KeyType &key) const { return contains(key) ? 1 : 0; }
  Iterator begin() const { return Iterator(head_->getNext(0)); }
  Iterator end() const { return Iterator(); }
  SizeType size() const { return size_; }
  bool empty() const { return size_ == 0; }

 private:
  NodeType *findLessThan(const KeyType &key, NodeType *prev_arr[kMaxHeight]) const {
    NodeType *node = head_;
    for (int height = kMaxHeight - 1; height >= 0; --height) {
      while (node->getNext(height) && comparator_(node->getNext(height)->key(), key))
        node = node->getNext(height);
      if (prev_arr) prev_arr[height] = node;
    }
    return node;
  }

  SizeType getRandomHeight() {
    constexpr SizeType kBranching = 4;
    SizeType height = 1;
    while (height < kMaxHeight && (engine_() % kBranching) == 0)
      ++height;
    return height;
  }

  template<typename K, typename V>
  static NodeType *createNode(K &&key, V &&value, SizeType height) {
    NodeType *new_node = skip_node_allocator_.allocate(1);
    skip_node_allocator_.construct(new_node, std::forward<K>(key), std::forward<V>(value), height);
    return new_node;
  }

  static void destroyNode(NodeType *node) {
    skip_node_allocator_.destroy(node);
    skip_node_allocator_.deallocate(node, 1);
  }
};

template<typename Key, typename Value, typename Comparator, typename Allocator>
Allocator SkipList<Key, Value, Comparator, Allocator>::skip_node_allocator_;

} // namespace fm

#endif //SKIPLIST__SKIPLIST_H_
