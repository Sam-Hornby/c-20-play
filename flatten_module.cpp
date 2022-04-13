#include "generator.hpp"
#include <vector>
#include <iostream>
#include <ranges>


const std::vector<std::vector<int>> Module = {
    {1,2 ,3 },
    {10, 20 ,30},
    {100, 200, 300},
    {1000, 2000, 3000},
};

/*
===============================================================================
Convenience for looking at all instructions
===============================================================================
*/
Generator<int> flatten_module(const std::vector<std::vector<int>> & Module) {
  for (const auto & comp : Module) {
    for (const auto & instr : comp) {
      co_yield instr;
    }
  }
}
/*
===============================================================================
Pipeline of operations
===============================================================================
*/
Generator<int> RemoveOddNumbers(Generator<int> module) {
  for (const auto & inst : module) {
    if (inst % 2) {
      continue;
    }
    co_yield inst;
  }
}

Generator<int> RemoveLargeNumbers(Generator<int> module) {
  for (const auto & inst : module) {
    if (inst > 1000) {
      continue;
    }
    co_yield inst;
  }
}

Generator<int> Add4000(Generator<int> module) {
  for (const auto & inst : module) {
    co_yield inst;
  }
  co_yield 4000;
}

template <typename Any, typename... Args>
inline auto operator| (Any&& obj, std::invocable<Any> auto && func){
  return func(std::forward<Any>(obj));
}

Generator<int> piping_pipeline(const std::vector<std::vector<int>> & Module) {
  return flatten_module(Module) 
         | RemoveOddNumbers
         | RemoveLargeNumbers
         | Add4000;
}

int main() {
  for (const auto & instr : piping_pipeline(Module)) {
    std::cout << instr << ", ";
  }
  std::cout << std::endl;
}
