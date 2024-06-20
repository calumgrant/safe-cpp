#pragma once
#include "fwd.hpp"
#include "lock.hpp"

namespace safe {

template <typename T, typename Mode> class exclusive {
public:
  using value_type = T;
  using lifetime_type = detail::lifetime<Mode>;

  // !! Not used?
  exclusive(ref<T, Mode> &&src)
      : value(*src), life(src.lifetime(), detail::move_tag()) {}

  exclusive(value_type &t, typename lifetime_type::reference life)
      : value(t), life(life) {}

  value_type &operator*() const { return value; }
  value_type *operator->() const { return &value; }

  template <typename U> exclusive &operator=(U &&other) {
    value = std::forward<U>(other);
    return *this;
  }

  operator value_type &() const { return value; }

private:
  value_type &value;
  detail::lock<exclusive_write, Mode> life;
};

// This is a mutable reference
template <typename T, typename Mode> class ref {
public:
  using value_type = T;
  using lifetime_type = detail::lifetime<Mode>;

  ref(value_type &value, typename lifetime_type::reference life)
      : value(value), life(life) {}

  template <typename U>
  ref(ref<U, Mode> &&src)
      : value(*src), life(src.lifetime(), detail::move_tag{}) {}

  template <typename U> ref(ref<const U, Mode> &&src) = delete;

  ref(ref &&src) : value(src.value), life(src.lifetime(), detail::move_tag{}) {}

  template <typename U>
  ref(const ref<U, Mode> &src) : value(src.value), life(src.lifetime()) {}

  ref(const ref &src) : value(src.value), life(src.reader.get_lifetime()) {}

  ref(safe::value<T, Mode> &src) : ref(src.write()) {}

  typename lifetime_type::reference lifetime() const {
    return life.lifetime();
  } // ??

  // Make private??
  ref<const T, Mode> read() const { return {value, reader.get_lifetime()}; }
  ref<T, Mode> write() const { return {value, reader}; }

  exclusive<T, Mode> operator->() { return {value, reader.get_lifetime()}; }

  exclusive<T, Mode> operator*() { return {value, reader.get_lifetime()}; }

  // Generally a bad idea
  // ptr<T, Mode> operator&() const { return {value, reader}; }

  ref &operator=(const ref &other) {
    value = other.value;
    return *this;
  }

  ref &operator=(ref &&other) {
    value = std::move(other.value);
    return *this;
  }

  template <typename U> ref &operator=(U &&src) {
    value = std::move(src);
    return *this;
  }

private:
  value_type &value;
  detail::lock<exclusive_write, Mode> life;
  mutable detail::lifetime<Mode> reader; // Track readers/writers of this writer
};

// This is an immutable reference
template <typename T, typename Mode> class ref<const T, Mode> {
public:
  using value_type = T;
  using lifetime_type = detail::lifetime<Mode>;

  ref(const value_type &value, typename lifetime_type::reference life)
      : value(value), life(life) {}

  ref(const ref &other) : value(other.value), life(other.lifetime()) {}

  ref(const ref<T, Mode> &other) : ref(other.read()) {}

  ref(ref<T, Mode> &&other)
      : value(*other), life(other.lifetime(), detail::move_tag{}) {}

  ref(const value<T, Mode> &src) : ref(src.read()) {}

  const value_type &operator*() const { return value; }
  const value_type *operator->() const { return &value; }

  // ptr<const T, Mode> operator&() const { return {value, lifetime()}; }

  operator const value_type &() const { return value; }

  typename lifetime_type::reference lifetime() const { return life.lifetime(); }

private:
  const value_type &value;
  detail::lock<shared_read, Mode> life;
};
} // namespace safe
