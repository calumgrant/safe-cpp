#pragma once
#include "fwd.hpp"
#include <optional>

namespace safe
{
template <typename T, typename Mode> class ptr {
public:
  using value_type = T;

  ref<T, Mode> operator*() const {
    if (!value)
      throw null_pointer();
    return *value;
  }

  ptr() = default;
  ptr(nullptr_t) {}

  ptr(ref<T, Mode> &&r) : value(std::move(r)) {}
  ptr(const ref<T, Mode> &r) : value(r) {}

  ptr & operator=(const ptr &other) {
    if(this != &other)
    {
      value.reset();
      value = other.value;
    }
    return *this;
  }

  ptr & operator=(ptr &&other) {
    if(this != &other)
    {
      value.reset();
      value = std::move(other.value);
    }
    return *this;
  }


  operator bool() const { return value.has_value(); }

  // exclusive<T, Mode> operator->() { return **this; }

private:
  std::optional<ref<T, Mode>> value;
};

}
