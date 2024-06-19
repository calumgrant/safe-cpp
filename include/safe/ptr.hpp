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
    if (!life.is_live())
      throw expired_pointer();
    return {*value, life.lifetime()};
  }

  ptr() : value{} {};
  ptr(nullptr_t) : ptr() {}
  ptr(value_type &value, typename lifetime_type::reference life)
      : value(&value), life(life) {}

  template <typename U>
  ptr(const ptr<U, Mode> &other) : value(other.value), life(other.life) {}

  operator bool() const { return value; }

  ref<T, Mode> operator->() { return **this; }

private:
  template <typename U, typename M> friend class ptr;
  T *value;
  detail::optional_lifetime_ptr<Mode> life;
};

}
