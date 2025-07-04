#pragma once
#include <memory>

#ifndef NENIY_REDBLACKTREE
#define NENIY_REDBLACKTREE

template <typename ValueType, typename Compare = std::less<ValueType>, typename Alloc = std::allocator<ValueType>>
class RedBlackTree {
 private:
  struct BaseNode;

  // base is used instead of nullptr
  BaseNode base; // parent == root, left == begin(), right == end() - 1

   struct BaseNode {
    BaseNode* parent;
    BaseNode* right;
    BaseNode* left;
  };

  struct Node : BaseNode {
    template <typename V>
    Node(BaseNode* parent, BaseNode* left, BaseNode* right, V&& value) : BaseNode{parent, left, right}, value(std::forward<V>(value)) {}

    std::size_t subtree_size = 1;
    ValueType value;
    bool is_red = true;
  };

  using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  using NodeAllocTraits = std::allocator_traits<NodeAlloc>;

  [[no_unique_address]] Compare compare;
  [[no_unique_address]] NodeAlloc alloc;

  template <typename V>
  Node* CreateNode(BaseNode* parent, BaseNode* left, BaseNode* right, V&& value) {
    Node* new_node = NodeAllocTraits::allocate(alloc, 1);
    NodeAllocTraits::construct(alloc, new_node, parent, left, right, std::forward<V>(value));
    return new_node;
  }

  void RemoveNode(Node* node) {
    if (node->parent != &base) {
      if (node->parent->left == node) {
        node->parent->left = &base;
      } else {
        node->parent->right = &base;
      }
    } else {
      base.parent = &base;
    }

    if (base.left == node) {
      base.left = node->parent;
    }
    if (base.right == node) {
      base.right = node->parent;
    }
    NodeAllocTraits::destroy(alloc, node);
    NodeAllocTraits::deallocate(alloc, node, 1);
  }

  static Node* Data(BaseNode* node) {
    return static_cast<Node*>(node);
  }

  template <bool IsConst>
  class Iterator {
   public:
    using value_type = ValueType;
    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = std::conditional_t<IsConst, const value_type*, value_type*>;
    using reference = std::conditional_t<IsConst, const value_type&, value_type>;
    using difference_type = std::ptrdiff_t;

    explicit Iterator(BaseNode* node, BaseNode* base) : node(node), base(base) {}

    template<bool OtherIsConst>
    requires(IsConst || !OtherIsConst)
    Iterator(Iterator<OtherIsConst> other) : node(other.node), base(other.base) {}

    bool operator==(Iterator other) const {
      return node == other.node;
    }

    bool operator!=(Iterator other) const {
      return node != other.node;
    }

    Iterator& operator++() {
      if (node->right != base) {
        node = node->right;
        while (node->left != base) {
          node = node->left;
        }
      } else if (base->right == node) {
        node = node->right;
      } else {
        BaseNode* parent = node->parent;
        while (parent->right == node) {
          node = parent;
          parent = node->parent;
        }
        node = parent;
      }
      return *this;
    }

    Iterator& operator--() {
      if (node == base) {
        node = base->right;
      } else if (node->left != base) {
        node = node->left;
        while (node->right != base) {
          node = node->right;
        }
      } else if (base->left == node) {
        node = node->left;
      } else {
        BaseNode* parent = node->parent;
        while (parent->left == node) {
          node = parent;
          parent = node->parent;
        }
        node = parent;
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator copy = *this;
      ++*this;
      return copy;
    }

    Iterator operator--(int) {
      Iterator copy = *this;
      --*this;
      return copy;
    }

    reference operator*() const {
      return Data(node)->value;
    }

    pointer operator->() const {
      return &**this;
    }

    template <typename AnyValueType, typename AnyCompare, typename AnyAlloc>
    friend class RedBlackTree;

   private:
    BaseNode* node;
    BaseNode* base;
  };

 public:
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  RedBlackTree() : base{&base, &base, &base} {}

  RedBlackTree(const Compare& compare, const Alloc& alloc) noexcept : base{&base, &base, &base}, compare(compare), alloc(alloc) {}

  RedBlackTree(const RedBlackTree& other) : RedBlackTree(other.compare, NodeAllocTraits::select_on_container_copy_construction(other.alloc)) {
    for (const ValueType& value : other) {
      insert(value);
    }
  }

  RedBlackTree& operator=(const RedBlackTree& other) {
    if (this == &other) {
      return *this;
    }
    clear();
    compare = other.compare;
    if constexpr (NodeAllocTraits::propagate_on_container_copy_assignment::value) {
      alloc = NodeAlloc(other.alloc);
    }
    for (const ValueType& value : other) {
      insert(value);
    }
    return *this;
  }

  RedBlackTree(RedBlackTree&& other) noexcept = delete;

  RedBlackTree& operator=(RedBlackTree&& other) = delete;

  ~RedBlackTree() { TraversalDelete(base.parent); }

  constexpr std::size_t size() const noexcept {
    if (base.parent == &base) {
      return 0;
    }
    return Data(base.parent)->subtree_size;
  }

  constexpr bool empty() const noexcept {
    return size() == 0;
  }

  iterator begin() { return iterator(base.left, &base); }

  iterator end() { return iterator(&base, &base); }

  const_iterator begin() const {
    return const_cast<RedBlackTree&>(*this).begin();
  }

  const_iterator end() const {
    return const_cast<RedBlackTree&>(*this).end();
  }

  const_iterator cbegin() const { return begin(); }

  const_iterator cend() const { return end(); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }

  reverse_iterator rend() { return reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const {
    return const_cast<RedBlackTree&>(*this).rbegin();
  }

  const_reverse_iterator rend() const {
    return const_cast<RedBlackTree&>(*this).rend();
  }

  const_reverse_iterator crbegin() const { return rbegin(); }

  const_reverse_iterator crend() const { return rend(); }

  template <typename V>
  std::pair<iterator, bool> insert(V&& value) {
    if (base.parent == &base) {
      base.parent = CreateNode(&base, &base, &base, std::forward<V>(value)); // Create a root
      base.left = base.parent;
      base.right = base.parent;
      Data(base.parent)->is_red = false;
      return {iterator(base.parent, &base), true};
    }
    return InsertImpl(Data(base.parent), std::forward<V>(value));
  }

  template <typename V> // count of deleted elements
  requires (!std::is_same_v<std::remove_cvref_t<V>, iterator>)
  std::size_t erase(V&& value) { return EraseImpl(Data(base.parent), std::forward<V>(value)); }

  iterator erase(const_iterator where) {
    Node* node = Data(where.node);
    iterator after_erased(node, where.base);
    ++after_erased;
    DeleteLogic(node);
    return after_erased;
  }

  void clear() {
    while (!empty()) {
      erase(begin());
    }
  }

  template <typename V>
  iterator find(V&& value) { return FindImpl(Data(base.parent), std::forward<V>(value)); }

  template <typename V>
  const_iterator find(V&& value) const { return const_cast<RedBlackTree&>(*this).find(std::forward<V>(value)); }

  template <typename V>
  iterator find_greater_than(V&& value) {
    return FindLessOrGreaterImpl(base.parent, std::forward<V>(value), &base, true);
  }

  template <typename V>
  const_iterator find_greater_than(V&& value) const {
    return const_cast<RedBlackTree&>(*this).find_greater_than(std::forward<V>(value));
  }

  template <typename V>
  iterator find_less_than(V&& value) {
    return FindLessOrGreaterImpl(base.parent, std::forward<V>(value), &base, false);
  }

  template <typename V>
  const_iterator find_less_than(V&& value) const {
    return const_cast<RedBlackTree&>(*this).find_less_than(std::forward<V>(value));
  }

  iterator statistic(std::size_t stat_num) {
    if (base.parent == &base || stat_num >= Data(base.parent)->subtree_size) {
      return end();
    }
    return StatisticImpl(base.parent, stat_num);
  }

  const_iterator statistic(std::size_t stat_num) const {
    return const_cast<RedBlackTree&>(*this).statistic(stat_num);
  }

 private:
  void TraversalDelete(BaseNode* node) {
    if (node == &base) {
      return;
    }
    TraversalDelete(node->left);
    TraversalDelete(node->right);
    RemoveNode(Data(node));
  }

  std::size_t SubtreeSize(BaseNode* node) const {
    if (node == &base) {
      return 0;
    }
    return Data(node)->subtree_size;
  }

  void SizeUpdate(BaseNode* node) { // recursively lifting with updating
    if (node == &base) {
      return;
    }
    Data(node)->subtree_size = SubtreeSize(node->left) + 1 + SubtreeSize(node->right);
    SizeUpdate(node->parent);
  }

  void RotateLeft(BaseNode* node) {  // From child to parent
    if (node == &base || node == base.parent) {
      return;
    }
    if (node->parent == base.parent) {
      base.parent = node;
    }
    node->parent->right = node->left;
    node->left = node->parent;
    node->parent = node->left->parent;
    if (node->parent != &base) {
      if (node->parent->left == node->left) {
        node->parent->left = node;
      } else {
        node->parent->right = node;
      }
    }
    node->left->parent  = node;
    if (node->left->right != &base) {
      node->left->right->parent = node->left;
    }
    Data(node->left)->subtree_size = SubtreeSize(node->left->left) + 1 + SubtreeSize(node->left->right);
    Data(node)->subtree_size = SubtreeSize(node->left) + 1 + SubtreeSize(node->right);
  }

  void RotateRight(BaseNode* node) {  // From child to parent
    if (node == &base || node == base.parent) {
      return;
    }
    if (node->parent == base.parent) {
      base.parent = node;
    }
    node->parent->left = node->right;
    node->right = node->parent;
    node->parent = node->right->parent;
    if (node->parent != &base) {
      if (node->parent->right == node->right) {
        node->parent->right = node;
      } else {
        node->parent->left = node;
      }
    }
    node->right->parent  = node;
    if (node->right->left != &base) {
      node->right->left->parent = node->right;
    }
    Data(node->right)->subtree_size = SubtreeSize(node->right->left) + 1 + SubtreeSize(node->right->right);
    Data(node)->subtree_size = SubtreeSize(node->left) + 1 + SubtreeSize(node->right);
  }

  void InsertRepair(Node* node) {
    if (node == &base) {
      return;
    }
    if (node->is_red && node == base.parent) {
      node->is_red = false;
    } else if (node->is_red && node->parent != &base &&
               Data(node->parent)->is_red) {  // Если требуется исправление (две
                                        // красные вершины)
      if (node->parent->parent != &base && node->parent->parent->left ==
          node->parent) {  // Если родитель слева от деда
        if (node->parent->parent->right != &base && Data(node->parent->parent->right)->is_red) {  // Случай 1 (дядя красный)
          Data(node->parent)->is_red = false;
          Data(node->parent->parent)->is_red = true;
          Data(node->parent->parent->right)->is_red = false;
          InsertRepair(Data(node->parent->parent));
        } else {  // Случай 2 (дядя чёрный)
          if (node->parent->left ==
              node) {  // Случай 2.1 (node тоже слева от родителя)
            RotateRight(node->parent);
            Data(node->parent)->is_red = false;
            Data(node->parent->right)->is_red = true;
          } else {  // Случай 2.2 (node справа от родителя)
            RotateLeft(node);
            RotateRight(node);
            node->is_red = false;
            Data(node->right)->is_red = true;
          }
        }
      } else if (node->parent->parent != &base) {  // Родитель справа от деда
        if (node->parent->parent->left != &base && Data(node->parent->parent->left)->is_red) {  // Случай 1 (дядя красный)
          Data(node->parent)->is_red = false;
          Data(node->parent->parent)->is_red = true;
          Data(node->parent->parent->left)->is_red = false;
          InsertRepair(Data(node->parent->parent));
        } else {  // Случай 2 (дядя чёрный)
          if (node->parent->right ==
              node) {  // Случай 2.1 (node тоже справа от родителя)
            RotateLeft(node->parent);
            Data(node->parent)->is_red = false;
            Data(node->parent->left)->is_red = true;
          } else {  // Случай 2.2 (node слева от родителя)
            RotateRight(node);
            RotateLeft(node);
            node->is_red = false;
            Data(node->left)->is_red = true;
          }
        }
      }
    }
  }

  template <typename V>
  std::pair<iterator, bool> InsertImpl(Node* node, V&& value) {
    if (compare(node->value, value)) {
      if (node->right == &base) {
        node->right = CreateNode(node, &base, &base, std::forward<V>(value));
        if (base.right == node) {
          base.right = node->right;
        }
        iterator inserted(node->right, &base);
        SizeUpdate(node->right);
        InsertRepair(Data(node->right));
        return {inserted, true};
      }
      return InsertImpl(Data(node->right), value);
    }
    if (compare(value, node->value)) {
      if (node->left == &base) {
        node->left = CreateNode(node, &base, &base, std::forward<V>(value));
        if (base.left == node) {
          base.left = node->left;
        }
        iterator inserted(node->left, &base);
        SizeUpdate(node->left);
        InsertRepair(Data(node->left));
        return {inserted, true};
      }
      return InsertImpl(Data(node->left), value);
    }
    return {iterator(node, &base), false};
  }

  template <typename V>
  iterator FindImpl(Node* node, V&& value) {
    if (node == &base) {
      return end();
    }
    if (compare(node->value, value)) {
      return FindImpl(Data(node->right), std::forward<V>(value));
    }
    if (compare(value, node->value)) {
      return FindImpl(Data(node->left), std::forward<V>(value));
    }
    return iterator(node, &base);
  }

  void CaseRedParent(Node* node) {
    if (node->parent->left != node) {
      // Случай 2.1.1.L (У левого ребёнка корня есть красный сын)
      if (node->parent->left->right != &base && Data(node->parent->left->right)->is_red) {
        RotateLeft(node->parent->left->right);
        RotateRight(node->parent->left);
        Data(node->parent)->is_red = false;
      } else if (node->parent->left->left != &base && Data(node->parent->left->left)->is_red) {
        RotateRight(node->parent->left);
      } else {  // Случай 2.1.2.L (у левого ребёнка корня нет красных сыновей)
        Data(node->parent)->is_red = false;
        Data(node->parent->left)->is_red = true;
      }
    } else {  // Случай 2.1.1.R (У правого ребёнка корня есть красный сын)
      if (node->parent->right->left != &base && Data(node->parent->right->left)->is_red) {
        RotateRight(node->parent->right->left);
        RotateLeft(node->parent->right);
        Data(node->parent)->is_red = false;
      } else if (node->parent->right->right != &base && Data(node->parent->right->right)->is_red) {
        RotateLeft(node->parent->right);
      } else {  // Случай 2.1.2.R (у правого ребёнка корня нет красных
                // сыновей)
        Data(node->parent)->is_red = false;
        Data(node->parent->right)->is_red = true;
      }
    }
  }

  void CaseBlackParent(Node* node) {
    if (node->parent->left != node) {  // Случай 2.2.L (Брат слева)
      if (node->parent->left != &base && Data(node->parent->left)->is_red) {  // Случай 2.2.L.1 (Брат красный)
        // Cлучай 2.2.L.1.1 (У сына брата есть красный сын)
        if (node->parent->left->right->left != &base && Data(node->parent->left->right->left)->is_red ||
            node->parent->left->right->right != &base && Data(node->parent->left->right->right)->is_red) {
          if (node->parent->left->right->right != &base && Data(node->parent->left->right->right)->is_red) {
            RotateLeft(node->parent->left->right->right);
            Data(node->parent->left->right)->is_red = false;
            Data(node->parent->left->right->left)->is_red = true;
          }
          RotateLeft(node->parent->left->right);
          RotateRight(node->parent->left);
          Data(node->parent->parent->left->right)->is_red = false;
        } else {  // Случай 2.2.L.1.2 (У сына брата нет красных сыновей)
          RotateRight(node->parent->left);
          Data(node->parent->left)->is_red = true;
          Data(node->parent->parent)->is_red = false;
        }
      } else {  // Случай 2.2.L.2 (Брат чёрный)
        // Случай 2.2.L.2.1 (У брата есть красные сыновья)
        if (node->parent->left->left != &base && Data(node->parent->left->left)->is_red) {
          RotateRight(node->parent->left);
          Data(node->parent->parent->left)->is_red = false;
        } else if (node->parent->left->right != &base && Data(node->parent->left->right)->is_red) {
          RotateLeft(node->parent->left->right);
          RotateRight(node->parent->left);
          Data(node->parent->parent)->is_red = false;
        } else {  // Случай 2.2.L.2.2 (У брата нет красных сыновей)
          Data(node->parent->left)->is_red = true;
          Case2(Data(node->parent));
        }
      }
    } else {  // Случай 2.2.R (Брат справа)
      if (node->parent->right != &base && Data(node->parent->right)->is_red) {  // Случай 2.2.R.1 (Брат красный)
        // Cлучай 2.2.R.1.1 (У сына брата есть красный сын)
        if (node->parent->right->left->left != &base && Data(node->parent->right->left->left)->is_red ||
            node->parent->right->left->right != &base && Data(node->parent->right->left->right)->is_red) {
          if (node->parent->right->left->left != &base && Data(node->parent->right->left->left)->is_red) {
            RotateRight(node->parent->right->left->left);
            Data(node->parent->right->left)->is_red = false;
            Data(node->parent->right->left->right)->is_red = true;
          }
          RotateRight(node->parent->right->left);
          RotateLeft(node->parent->right);
          Data(node->parent->parent->right->left)->is_red = false;
        } else {  // Случай 2.2.R.1.2 (У сына брата нет красных сыновей)
          RotateLeft(node->parent->right);
          Data(node->parent->right)->is_red = true;
          Data(node->parent->parent)->is_red = false;
        }
      } else {  // Случай 2.2.R.2 (Брат чёрный)
        // Случай 2.2.R.2.1 (У брата есть красные сыновья)
        if (node->parent->right->right != &base && Data(node->parent->right->right)->is_red) {
          RotateLeft(node->parent->right);
          Data(node->parent->parent->right)->is_red = false;
        } else if (node->parent->right->left != &base && Data(node->parent->right->left)->is_red) {
          RotateRight(node->parent->right->left);
          RotateLeft(node->parent->right);
          Data(node->parent->parent)->is_red = false;
        } else {  // Случай 2.2.R.2.2 (У брата нет красных сыновей)
          Data(node->parent->right)->is_red = true;
          Case2(Data(node->parent));
        }
      }
    }
  }

  void Case2(Node* node) {  // Починка чёрной глубины
    if (node->parent == &base) {
      return;
    }
    if (Data(node->parent)->is_red) {  // Случай 2.1 (родитель красный)
      CaseRedParent(node);
    } else {  // Cлучай 2.2 (родитель чёрный)
      CaseBlackParent(node);
    }
  }

  void DeleteLogic(Node* node) {
    if (node->left == &base && node->right == &base) {  // Оба сына пустые
      if (!node->is_red) {
        Case2(node);
      }
      BaseNode* parent = node->parent;
      RemoveNode(node);
      SizeUpdate(parent);
    } else if (node->left == &base ||
               node->right == &base) {  // Случай 3 (только один обычный сын)
      if (node->left != &base && Data(node->left)->is_red) {
        RotateRight(node->left);
        node->is_red = true;
        Data(node->parent)->is_red = false;
        DeleteLogic(node);
      } else {
        RotateLeft(node->right);
        node->is_red = true;
        Data(node->parent)->is_red = false;
        DeleteLogic(node);
      }
    } else {  // Оба сына обычные
      BaseNode* right_min = node->right;
      while (right_min->left != &base) {
        right_min = right_min->left;
      }
      std::swap(Data(right_min)->value, node->value);
      DeleteLogic(Data(right_min));
    }
  }

  template <typename V>
  std::size_t EraseImpl(Node* node, V&& value) {
    if (node == &base) {
      return 0;
    }
    if (compare(node->value, value)) {
      return EraseImpl(Data(node->right), std::forward<V>(value));
    }
    if (compare(value, node->value)) {
      return EraseImpl(Data(node->left), std::forward<V>(value));
    }
    
    DeleteLogic(node);
    return 1;
  }

  BaseNode* ChoiceLessOrGreater(BaseNode* prev_node, BaseNode* new_node, bool greater) const {
    if (prev_node == &base) {
      return new_node;
    }
    if (compare(Data(prev_node)->value, Data(new_node)->value)) {
      if (greater) {
        return prev_node;
      }
      return new_node;
    }
    if (greater) {
      return new_node;
    }
    return prev_node;
  }

  template <typename V>
  iterator FindLessOrGreaterImpl(BaseNode* node, V&& value, BaseNode* best_found, bool greater) {
    if (node == &base) {
      return iterator(best_found, &base);
    }
    if (compare(Data(node)->value, value)) {
      if (greater) {
        return FindLessOrGreaterImpl(node->right, std::forward<V>(value), best_found, greater);
      }
      return FindLessOrGreaterImpl(node->right, std::forward<V>(value), ChoiceLessOrGreater(best_found, node, greater), greater);
    }
    if (compare(value, Data(node)->value)) {
      if (greater) {
        return FindLessOrGreaterImpl(node->left, std::forward<V>(value), ChoiceLessOrGreater(best_found, node, greater), greater);
      }
      return FindLessOrGreaterImpl(node->left, std::forward<V>(value), best_found, greater);
    }
    if (greater) {
      return FindLessOrGreaterImpl(node->right, std::forward<V>(value), best_found, greater);
    }
    return FindLessOrGreaterImpl(node->left, std::forward<V>(value), best_found, greater);
  }

  iterator StatisticImpl(BaseNode* node, std::size_t stat_num) {
    std::size_t left_subtree = SubtreeSize(node->left);
    if (left_subtree < stat_num) { // statistic in right subtree
      return StatisticImpl(node->right, stat_num - left_subtree - 1);
    }
    if (left_subtree > stat_num) {
      return StatisticImpl(node->left, stat_num);
    }
    return iterator(node, &base);
  }
};

#endif // NENIY_REDBLACKTREE
