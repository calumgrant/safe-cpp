#pragma once

#define SAFE_ENABLED 1
#undef NDEBUG

#include <cassert>
#include <stdexcept>

template<typename Ex, typename Fn>
void assert_throws(Fn fn) {
  try {
    fn();
    assert(!"Should not get here");
  } catch (const Ex &) {
    // Expected
  }
  catch(std::exception &e) {
    assert(!"Unexpected exception");
  }
  catch(...) {
    assert(!"Unexpected exception");
  }
}
