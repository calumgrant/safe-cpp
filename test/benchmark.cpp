#include "safe/value.hpp"
#include "safe/vector.hpp"
#include <chrono>
#include <functional>
#include <iostream>

using namespace safe;

void create(int size, std::vector<int> &vec) {
  for (auto i = 0; i < size; i++) {
    vec.emplace_back(i);
  }
}

template <typename Mode>
void create(int size, ref<vector<int, Mode>, Mode> vec) {
  for (auto i = 0; i < size; i++) {
    vec.emplace_back(i);
  }
}

template <typename Mode> void use(ref<const vector<int, Mode>, Mode> vec) {
  int sum = 0;
  int size = vec.size();

  // TODO: Don't create a read ref each time.

  for (auto i = 0; i < size; i++) {
    for (auto j = 0; j < size; j++) {
      sum += *vec.at(i) * *vec.at(j);
    }
  }
  std::cout << "Sum=" << sum << " ";
}

template <typename Mode> void run(int size) {
  auto t1 = std::chrono::high_resolution_clock::now();
  safe::vector<int, Mode> vec;

  create<Mode>(size, vec.write());
  use<Mode>(vec.read());
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout
      << "Time="
      << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
      << "ms" << std::endl;
}

void normal_run(int size) {
  std::vector<int> vec;
  for (auto i = 0; i < size; i++) {
    vec.emplace_back(i);
  }

  int sum = 0;

  // TODO: Don't create a read ref each time.

  for (auto i = 0; i < size; i++) {
    for (auto j = 0; j < size; j++) {
      sum += vec[i] * vec[j];
    }
  }
  std::cout << "Sum=" << sum << " ";
}

void benchmark(const std::string &name, std::function<void()> fn) {
  auto t1 = std::chrono::high_resolution_clock::now();
  fn();
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout
      << name << " Time="
      << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
      << "ms" << std::endl;
}

int main() {
  const int size = 10000;
  std::cout << "Checked time: " << std::endl;
  run<safe::checked>(size);
  std::cout << "Unchecked time: " << std::endl;
  run<safe::unchecked>(size);
  benchmark("Native", [&]() { normal_run(size); });
}
