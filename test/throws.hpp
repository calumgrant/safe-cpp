#include <cassert>

template <typename Ex = std::exception, typename Fn> void throws(Fn f) {
  try {
    f();
  } catch (const Ex &e) {
    return;
  }
  assert(!"Expected exception not thrown");
}

#define THROWS(X) throws([&]() { X; });
#define TERMINATES(X)
