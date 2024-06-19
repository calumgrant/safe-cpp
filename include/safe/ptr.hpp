#pragma once
#include "fwd.hpp"

namespace safe
{
template <typename T, typename Mode> class ptr {
public:
  using value_type = T;
  using lifetime_type = detail::lifetime<Mode>;

  ref<T, Mode> operator*() const {
    if (!value)
      throw null_pointer();
    return {*value, life.lifetime()};
  }

  ptr() : value{} {};
  ptr(nullptr_t) : ptr() {}
  ptr(value_type &value, lifetime_type &life) : value(&value), life(life) {}

  operator bool() const { return value; }

  ref<T, Mode> operator->() { return **this; }

private:
  T *value;
  detail::optional_lifetime_ptr<Mode> life;
};

}
