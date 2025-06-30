#include <iostream>
#include <string>

class RedBlackTree {
 public:
  RedBlackTree() : root_(new Node(nullptr)) {}

  ~RedBlackTree() { TraversalDelete(root_); }

  void Insert(int value) {
    if (root_->unused) {
      root_->Use(value);
      root_->is_red = false;
    } else {
      InsertImpl(root_, value);
    }
  }

  std::string Find(int value) const { return FindImpl(root_, value); }

  void Erase(int value) { EraseImpl(root_, value); }

  std::string FindGreaterThan(int value) {
    return FindLessOrGreaterImpl(root_, value + 1, true, root_->value);
  }

  std::string FindLessThan(int value) {
    return FindLessOrGreaterImpl(root_, value - 1, false, root_->value);
  }

  std::string Statistic(int stat_num) {
    if (stat_num < 0 || stat_num >= root_->size) {
      return "none";
    }
    return StatisticImpl(root_, stat_num + 1);
  }

 private:
  struct Node {
    Node(Node* parent)
        : parent(parent),
          right(nullptr),
          left(nullptr),
          value(0),
          size(0),
          is_red(false),
          unused(true) {}

    void Use(int init_value) {
      right = new Node(this);
      left = new Node(this);
      value = init_value;
      size = 1;
      is_red = true;
      unused = false;
    }

    void Unuse() {
      right = nullptr;
      left = nullptr;
      value = 0;
      size = 0;
      is_red = false;
      unused = true;
    }

    Node* parent;
    Node* right;
    Node* left;
    int value;
    int size;
    bool is_red;
    bool unused;
  };

  void TraversalDelete(Node* node) {
    if (node->left != nullptr) {
      TraversalDelete(node->left);
    }
    if (node->right != nullptr) {
      TraversalDelete(node->right);
    }
    delete node;
  }

  void SizeUpdate(Node* node) {
    if (node == nullptr) {
      return;
    }
    node->size = node->left->size + node->right->size + 1;
    SizeUpdate(node->parent);
  }

  void RotateLeft(Node* node) {  // От текущего узла по ребру к родителю
    if (node->parent == root_) {
      root_ = node;
    }
    Node* tmp = node->left;
    if (tmp != nullptr) {
      tmp->parent = node->parent;
    }
    node->left = node->parent;
    if (node->parent->parent != nullptr) {
      if (node->parent->parent->left == node->parent) {
        node->parent->parent->left = node;
      } else {
        node->parent->parent->right = node;
      }
    }
    node->parent->right = tmp;
    node->parent = node->parent->parent;
    node->left->parent = node;
    node->left->size = node->left->left->size + node->left->right->size + 1;
    node->size = node->left->size + node->right->size + 1;
  }

  void RotateRight(Node* node) {  // От текущего узла по ребру к родителю
    if (node->parent == root_) {
      root_ = node;
    }
    Node* tmp = node->right;
    if (tmp != nullptr) {
      tmp->parent = node->parent;
    }
    node->right = node->parent;
    if (node->parent->parent != nullptr) {
      if (node->parent->parent->left == node->parent) {
        node->parent->parent->left = node;
      } else {
        node->parent->parent->right = node;
      }
    }
    node->parent->left = tmp;
    node->parent = node->parent->parent;
    node->right->parent = node;
    node->right->size = node->right->left->size + node->right->right->size + 1;
    node->size = node->left->size + node->right->size + 1;
  }

  void InsertRepair(Node* node) {
    if (node->is_red && node == root_) {
      node->is_red = false;
    } else if (node->is_red &&
               node->parent->is_red) {  // Если требуется исправление (две
                                        // красные вершины)
      if (node->parent->parent->left ==
          node->parent) {  // Если родитель слева от деда
        if (node->parent->parent->right->is_red) {  // Случай 1 (дядя красный)
          node->parent->is_red = false;
          node->parent->parent->is_red = true;
          node->parent->parent->right->is_red = false;
          InsertRepair(node->parent->parent);
        } else {  // Случай 2 (дядя чёрный)
          if (node->parent->left ==
              node) {  // Случай 2.1 (node тоже слева от родителя)
            RotateRight(node->parent);
            node->parent->is_red = false;
            node->parent->right->is_red = true;
          } else {  // Случай 2.2 (node справа от родителя)
            RotateLeft(node);
            RotateRight(node);
            node->is_red = false;
            node->right->is_red = true;
          }
        }
      } else {  // Родитель справа от деда
        if (node->parent->parent->left->is_red) {  // Случай 1 (дядя красный)
          node->parent->is_red = false;
          node->parent->parent->is_red = true;
          node->parent->parent->left->is_red = false;
          InsertRepair(node->parent->parent);
        } else {  // Случай 2 (дядя чёрный)
          if (node->parent->right ==
              node) {  // Случай 2.1 (node тоже справа от родителя)
            RotateLeft(node->parent);
            node->parent->is_red = false;
            node->parent->left->is_red = true;
          } else {  // Случай 2.2 (node слева от родителя)
            RotateRight(node);
            RotateLeft(node);
            node->is_red = false;
            node->left->is_red = true;
          }
        }
      }
    }
  }

  void InsertImpl(Node* node, int value) {
    if (node->value < value) {
      if (node->right->unused) {
        node->right->Use(value);
        SizeUpdate(node->right);
        InsertRepair(node->right);
      } else {
        InsertImpl(node->right, value);
      }
    } else if (node->value > value) {
      if (node->left->unused) {
        node->left->Use(value);
        SizeUpdate(node->left);
        InsertRepair(node->left);
      } else {
        InsertImpl(node->left, value);
      }
    }
  }

  std::string FindImpl(Node* node, int value) const {
    if (node->unused) {
      return "false";
    }
    if (node->value == value) {
      return "true";
    }
    if (node->value < value) {
      return FindImpl(node->right, value);
    }
    return FindImpl(node->left, value);
  }

  void CaseRedParent(Node* node) {
    if (node->parent->left != node) {
      // Случай 2.1.1.L (У левого ребёнка корня есть красный сын)
      if (node->parent->left->right->is_red) {
        RotateLeft(node->parent->left->right);
        RotateRight(node->parent->left);
        node->parent->is_red = false;
      } else if (node->parent->left->left->is_red) {
        RotateRight(node->parent->left);
      } else {  // Случай 2.1.2.L (у левого ребёнка корня нет красных сыновей)
        node->parent->is_red = false;
        node->parent->left->is_red = true;
      }
    } else {  // Случай 2.1.1.R (У правого ребёнка корня есть красный сын)
      if (node->parent->right->left->is_red) {
        RotateRight(node->parent->right->left);
        RotateLeft(node->parent->right);
        node->parent->is_red = false;
      } else if (node->parent->right->right->is_red) {
        RotateLeft(node->parent->right);
      } else {  // Случай 2.1.2.R (у правого ребёнка корня нет красных
                // сыновей)
        node->parent->is_red = false;
        node->parent->right->is_red = true;
      }
    }
  }

  void CaseBlackParent(Node* node) {
    if (node->parent->left != node) {  // Случай 2.2.L (Брат слева)
      if (node->parent->left->is_red) {  // Случай 2.2.L.1 (Брат красный)
        // Cлучай 2.2.L.1.1 (У сына брата есть красный сын)
        if (node->parent->left->right->left->is_red ||
            node->parent->left->right->right->is_red) {
          if (node->parent->left->right->right->is_red) {
            RotateLeft(node->parent->left->right->right);
            node->parent->left->right->is_red = false;
            node->parent->left->right->left->is_red = true;
          }
          RotateLeft(node->parent->left->right);
          RotateRight(node->parent->left);
          node->parent->parent->left->right->is_red = false;
        } else {  // Случай 2.2.L.1.2 (У сына брата нет красных сыновей)
          RotateRight(node->parent->left);
          node->parent->left->is_red = true;
          node->parent->parent->is_red = false;
        }
      } else {  // Случай 2.2.L.2 (Брат чёрный)
        // Случай 2.2.L.2.1 (У брата есть красные сыновья)
        if (node->parent->left->left->is_red) {
          RotateRight(node->parent->left);
          node->parent->parent->left->is_red = false;
        } else if (node->parent->left->right->is_red) {
          RotateLeft(node->parent->left->right);
          RotateRight(node->parent->left);
          node->parent->parent->is_red = false;
        } else {  // Случай 2.2.L.2.2 (У брата нет красных сыновей)
          node->parent->left->is_red = true;
          Case2(node->parent);
        }
      }
    } else {  // Случай 2.2.R (Брат справа)
      if (node->parent->right->is_red) {  // Случай 2.2.R.1 (Брат красный)
        // Cлучай 2.2.R.1.1 (У сына брата есть красный сын)
        if (node->parent->right->left->left->is_red ||
            node->parent->right->left->right->is_red) {
          if (node->parent->right->left->left->is_red) {
            RotateRight(node->parent->right->left->left);
            node->parent->right->left->is_red = false;
            node->parent->right->left->right->is_red = true;
          }
          RotateRight(node->parent->right->left);
          RotateLeft(node->parent->right);
          node->parent->parent->right->left->is_red = false;
        } else {  // Случай 2.2.R.1.2 (У сына брата нет красных сыновей)
          RotateLeft(node->parent->right);
          node->parent->right->is_red = true;
          node->parent->parent->is_red = false;
        }
      } else {  // Случай 2.2.R.2 (Брат чёрный)
        // Случай 2.2.R.2.1 (У брата есть красные сыновья)
        if (node->parent->right->right->is_red) {
          RotateLeft(node->parent->right);
          node->parent->parent->right->is_red = false;
        } else if (node->parent->right->left->is_red) {
          RotateRight(node->parent->right->left);
          RotateLeft(node->parent->right);
          node->parent->parent->is_red = false;
        } else {  // Случай 2.2.R.2.2 (У брата нет красных сыновей)
          node->parent->right->is_red = true;
          Case2(node->parent);
        }
      }
    }
  }

  void Case2(Node* node) {  // Починка чёрной глубины
    if (node->parent == nullptr) {
      return;
    }
    if (node->parent->is_red) {  // Случай 2.1 (родитель красный)
      CaseRedParent(node);
    } else {  // Cлучай 2.2 (родитель чёрный)
      CaseBlackParent(node);
    }
  }

  void DeleteLogic(Node* node) {
    if (node->left->unused && node->right->unused) {  // Оба сына пустые
      delete node->left;
      delete node->right;
      if (node->is_red) {  // Случай 1
        node->Unuse();
        SizeUpdate(node->parent);
      } else {  // Случай 2 (удаляемая вершина чёрная)
        node->Unuse();
        SizeUpdate(node->parent);
        Case2(node);
      }
    } else if (node->left->unused ||
               node->right->unused) {  // Случай 3 (только один обычный сын)
      if (node->left->is_red) {
        RotateRight(node->left);
        node->is_red = true;
        node->parent->is_red = false;
        DeleteLogic(node);
      } else {
        RotateLeft(node->right);
        node->is_red = true;
        node->parent->is_red = false;
        DeleteLogic(node);
      }
    } else {  // Оба сына обычные
      Node* left_min = node->right;
      while ((left_min->left != nullptr) && !left_min->left->unused) {
        left_min = left_min->left;
      }
      std::swap(left_min->value, node->value);
      DeleteLogic(left_min);
    }
  }

  void RootRoute(Node* node) {
    if (!node->left->unused && node->right->unused) {
      root_ = node->left;
    } else if (node->left->unused && !node->right->unused) {
      root_ = node->right;
    }
  }

  void EraseImpl(Node* node, int value) {
    if (node->unused) {
      return;
    }
    if (node->value == value) {
      if (node == root_) {
        RootRoute(node);
      }
      DeleteLogic(node);
    } else if (node->value < value) {
      EraseImpl(node->right, value);
    } else {  // node->value > value
      EraseImpl(node->left, value);
    }
  }

  std::string FindLessOrGreaterImpl(Node* node, int value, bool greater,
                                    int last_found) {
    if (node->unused) {
      if (greater) {
        return last_found >= value ? std::to_string(last_found) : "none";
      }
      return last_found <= value ? std::to_string(last_found) : "none";
    }
    if (node->value == value) {
      return std::to_string(value);
    }
    if (node->value < value) {
      if (greater) {
        return FindLessOrGreaterImpl(
            node->right, value, greater,
            node->value >= value ? node->value : last_found);
      }
      return FindLessOrGreaterImpl(
          node->right, value, greater,
          node->value <= value ? node->value : last_found);
    }
    if (greater) {
      return FindLessOrGreaterImpl(
          node->left, value, greater,
          node->value >= value ? node->value : last_found);
    }
    return FindLessOrGreaterImpl(
        node->left, value, greater,
        node->value <= value ? node->value : last_found);
  }

  std::string StatisticImpl(Node* node, int stat_num) {
    if (node->size - node->right->size == stat_num) {
      return std::to_string(node->value);
    }
    if (node->left->size < stat_num) {
      return StatisticImpl(node->right, stat_num - node->left->size - 1);
    }
    return StatisticImpl(node->left, stat_num);
  }

  Node* root_;
};
