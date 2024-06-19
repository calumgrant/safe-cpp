#pragma once

#include "exceptions.hpp"
#include <atomic>

namespace safe {

namespace detail {

template <typename Mode> struct lifetime;

template <> struct lifetime<unchecked> {

  void terminate_if_live() const {}
  void check_no_readers() const {}

  lifetime &get_lifetime() { return *this; }
};

template <> struct lifetime<checked> {
  using int_type = std::atomic<int>;
  int_type weak_count = 1, // Number of references to this record
      is_live = 1,         // Number of strong references
      readers = 0,         // Number of active readers
      writers = 0;         // Number of active writers

  void terminate_if_live() const {
    if (readers || writers)
      std::terminate();
  }

  void check_no_readers() const {
    if (readers) {
      throw invalid_operation<exclusive_write>();
    }
  }

  ~lifetime() {
    terminate_if_live();
    if (weak_count > 1)
      std::terminate();
  }

  lifetime<checked> &get_lifetime() { return *this; }
};

template <typename Mode> class lifetime_ref;

template <> class lifetime_ref<checked> {
public:
  lifetime_ref() : life(new detail::lifetime<checked>) {}
  lifetime_ref(lifetime<checked> &life) : life(&life) { life.weak_count++; }
  ~lifetime_ref() {
    if (!--life->weak_count) {
      delete life;
    }
  }

  detail::lifetime<checked> &lifetime() const { return *life; }

protected:
  lifetime_ref(const lifetime_ref &other) = delete;
  lifetime_ref &operator=(const lifetime_ref &other) = delete;
  detail::lifetime<checked> *life;
};

template <> struct lifetime<checked_weak> {
  lifetime() {}
  ~lifetime() { get_lifetime().is_live = false; }

  lifetime_ref<checked> life;

  lifetime<checked> &get_lifetime() { return life.lifetime(); }
};

template <> class lifetime_ref<unchecked> {
public:
  lifetime_ref() {}
  detail::lifetime<unchecked> &lifetime() const { return life; }

private:
  mutable detail::lifetime<unchecked> life;
};

template <typename Mode> class optional_lifetime_ptr;

template <> class optional_lifetime_ptr<unchecked> {
public:
  optional_lifetime_ptr() = default;
  optional_lifetime_ptr(lifetime<unchecked> &life) {}

  bool is_live() const { return true; }
  detail::lifetime<unchecked> &lifetime() const { return life; }

private:
  mutable detail::lifetime<unchecked> life;
};

template <> class optional_lifetime_ptr<checked> {
public:
  optional_lifetime_ptr() : life(nullptr) {}
  optional_lifetime_ptr(lifetime<checked> &life) : life(&life) {
    life.weak_count++;
  }
  optional_lifetime_ptr(const optional_lifetime_ptr &other) : life(other.life) {
    if (life)
      life->weak_count++;
  }
  optional_lifetime_ptr(optional_lifetime_ptr &&other) : life(other.life) {
    other.life = nullptr;
  }

  optional_lifetime_ptr &operator=(const optional_lifetime_ptr &other) {
    if (other.life)
      other.life->weak_count++;
    if (life && !--life->weak_count) {
      delete life;
    }
    life = other.life;
    return *this;
  }

  optional_lifetime_ptr &operator=(optional_lifetime_ptr &&other) {
    // edge case: self-assignment !! ??
    if (life && !--life->weak_count) {
      delete life;
    }
    life = other.life;
    other.life = nullptr;
    return *this;
  }

  ~optional_lifetime_ptr() {
    if (life && !--life->weak_count) {
      // life->terminate_if_live();
      delete life;
    }
  }

  bool is_live() const { return life && life->is_live; }
  detail::lifetime<checked> &lifetime() const {
    if (!life)
      throw null_pointer();
    return *life;
  }

private:
  detail::lifetime<checked> *life;
};
} // namespace detail
} // namespace safe
