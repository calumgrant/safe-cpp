#include <iostream>

template <typename Ex = std::exception, typename Fn> void throws(Fn f) {
  try {
    f();
  } catch (const Ex &e) {
    return;
  }
  std::cerr << "Expected exception not thrown" << std::endl;
  exit(1);
}

#define THROWS(X) throws([&]() { X; });
#define TERMINATES(X)
