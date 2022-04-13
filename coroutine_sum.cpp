#include <coroutine>
#include <vector>
#include <iostream>
#include "generator.hpp"
#include <numeric>

template <class T>
struct PrintAllocator : public std::allocator<T> {
  [[nodiscard]] constexpr T* allocate( std::size_t n ) {
    std::cout << "You allocated you stupid b**** " << n << "\n";
    return static_cast<std::allocator<T> *>(this)->allocate(n);
  }
}; 

struct Node {
  std::string name;
  const Node * child;
};


const Node leaf = {"Leaf", nullptr};
const Node child = {"Child", &leaf};
const Node root = {"Root", &child};

template <class T>
using vec_type = std::vector<T, PrintAllocator<T>>;

/*
===============================================================================
With Vectors
===============================================================================
*/
vec_type<const Node *> GetPreOrder(const Node & root) {
  vec_type<const Node *> result;
  const Node * current = root.child;
  while (current) {
    result.emplace_back(current);
    current = current->child;
  }
  return result;
}

/*
===============================================================================
With Callbacks
===============================================================================
*/
template <typename F>
void GetPreOrderCallBack(const Node & root, F f) {
  const Node * current = root.child;
  while (current) {
    f(current);
    current = current->child;
  }
}

/*
===============================================================================
With Coroutines
===============================================================================
*/
Generator<const Node *> GetPreOrderCoroutine(const Node & root) {
  const Node * current = root.child;
  while (current) {
    co_yield current;
    current = current->child;
  }
}

ReturnGenerator<int> Range(int start, const int end) {
  while (start < end) {
    co_yield start++;
  }
  co_return end;
}

/*
===============================================================================
Usage
===============================================================================
*/
int main() {
  const auto result = GetPreOrder(root);
  std::cout << "Result is: " << std::endl;
  for (const auto & n : result) {
    std::cout << n->name << std::endl;
  }
  std::cout << "\nCallback version\n";
  GetPreOrderCallBack(root, [] (const Node *n) {
    std::cout << n->name << std::endl;
  });

  std::cout << "\nCoroutine version\n";
  for (const Node * n : GetPreOrderCoroutine(root)) {
    std::cout << n->name << std::endl;
  }
  auto range = Range(5, 10);
  return std::accumulate(range.begin(), range.end(), 0);
}
