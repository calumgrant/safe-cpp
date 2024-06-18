#pragma once

#include <stdexcept>
#include <utility>

#include "fwd.hpp"
#include "lifetime.hpp"

#include "ref.hpp"
#include "ptr.hpp"


namespace safe {


template <typename T, typename Mode> class value {
public:
  using value_type = T;
  using lifetime_type = detail::lifetime<Mode>;

  template <typename U> value(const value<U, Mode> &src) : value(*src.read()) {}

  template <typename U>
  value(value<U, Mode> &&src) : value(std::move(*src.read())) {}

  template <typename... Args>
  value(Args &&...args) : _value(std::forward<Args>(args)...) {}

  value(T&&src) : _value(std::move(src)) {}

  ref<const T, Mode> read() const { return {_value, life}; }
  ref<T, Mode> write() { return {_value, life}; }

  ref<const T, Mode> operator*() const { return read(); }
  ref<const T, Mode> operator->() const { return read(); }

  ref<T, Mode> operator*() { return write(); }
  ref<T, Mode> operator->() { return write(); }

  ptr<T, Mode> operator&() { return write(); }
  ptr<const T, Mode> operator&() const { return read(); }

  // value_type &unsafe_read() { return _value; }
  // const value_type &unsafe_read() const { return _value; }

  template <typename U> value &operator=(U &&v) {
    *write() = std::forward<U>(v);
    return *this;
  }

  template <typename U> value &operator=(const value<U> &v) {
    *write() = *v.read();
    return *this;
  }

  template <typename U> value &operator=(value<U> &v) {
    *write() = *v.read();
    return *this;
  }

  template <typename U> value &operator=(value<U> &&src) {
    *write() = std::move(src.value);
    return *this;
  }

  lifetime_type &lifetime() const { return life; }

protected:
  value_type _value;
  mutable lifetime_type life;
};

} // namespace safe
