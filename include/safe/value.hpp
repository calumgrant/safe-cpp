#pragma once

#include <stdexcept>
#include <utility>

#include "fwd.hpp"
#include "lifetime.hpp"

#include "ref.hpp"
#include "ptr.hpp"


namespace safe {

template <typename T> struct mode_type {
  using type = T;
};

template <> struct mode_type<weak> {
  using type = mode;
};

template <typename T, typename Mode> class value {
public:
  using value_type = T;
  using lifetime_type = detail::lifetime<Mode>;
  using mode = typename mode_type<Mode>::type;

  template <typename U> value(const value<U, Mode> &src) : value(*src.read()) {}

  template <typename U>
  value(value<U, mode> &&src) : value(std::move(*src.read())) {}

  template <typename... Args>
  value(Args &&...args) : _value(std::forward<Args>(args)...) {}

  value(T&&src) : _value(std::move(src)) {}

  ref<const T, mode> read() const { return {_value, life.get_lifetime()}; }
  ref<T, mode> write() { return {_value, life.get_lifetime()}; }

  ref<const T, mode> operator*() const { return read(); }
  ref<const T, mode> operator->() const { return read(); }

  ref<T, mode> operator*() { return write(); }
  ref<T, mode> operator->() { return write(); }

  ptr<T, mode> operator&() { return {_value, life.get_lifetime()}; }
  ptr<const T, mode> operator&() const {
    return {_value, life.lifetime()};
    ;
  }

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
