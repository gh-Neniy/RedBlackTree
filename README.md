# RedBlackTree

Красно-чёрное дерево с поиском порядковой статистики. Это сбалансированное двоичное дерево поиска, которое обеспечивает **логарифмическое время** для операций вставки, удаления и поиска элемента, порядковой статистики, поиска наибольшего элемента строго меньшего данного и наименьшего строго большего данного.

## Основные операции

- **insert(value)** — Вставка нового элемента
- **erase(value)** — Удаление существующего элемента
- **find(value)** — Проверка наличия элемента
- **find_greater_than(value)** — Наименьший элемент, строго больший value
- **find_less_than(value)** - Наибольший элемент, строго меньший value
- **statistic(k)** — k-я порядковая статистика в 0-индексации

## Использование

```cpp
#include "RedBlackTree.h"
#include <iostream>

int main() {
  RedBlackTree<int> tree;
  tree.insert(2);
  tree.insert(5);
  tree.insert(11);
  tree.insert(17);

  std::cout << *tree.find(2) << '\n'; // 2
  std::cout << *tree.statistic(1) << '\n'; // 5
  tree.erase(11);

  std::cout << *tree.find_less_than(8) << '\n'; // 5
  std::cout << *tree.find_greater_than(8) << '\n'; // 17
}

```
