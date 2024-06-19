#pragma once

namespace safe {
namespace detail {
template <typename Op, typename Mode> class lock;

struct move_tag {};

template <typename Op> class lock<Op, unchecked> {
public:
  lock(detail::lifetime<unchecked> &) {}
  lock(detail::lifetime<unchecked> &, move_tag) {}
  detail::lifetime<unchecked> &lifetime() const { return life; }

private:
  mutable detail::lifetime<unchecked> life;
};

template <typename Op> class lock<Op, checked> {
public:
  lock(detail::lifetime<checked> &life) : lifetime_ptr(life) {
    Op::acquire(life);
  }
  lock(detail::lifetime<checked> &life, move_tag) : lifetime_ptr(life) {
    Op::acquire_move(life);
  }
  lock(const lock &other) = delete;

  ~lock() { Op::release(lifetime_ptr.lifetime()); }

  detail::lifetime<checked> &lifetime() const {
    return lifetime_ptr.lifetime();
  }

private:
  lifetime_ref<checked> lifetime_ptr;
};

template <typename Op, typename Mode> class optional_lock;

template <typename Op> class optional_lock<Op, unchecked> {
public:
  optional_lock() = default;
  optional_lock(lifetime<unchecked> &) {}
  detail::lifetime<unchecked> &lifetime() const { return life; }

private:
  mutable detail::lifetime<unchecked> life;
};

template <typename Op> class optional_lock<Op, checked> {
public:
  optional_lock() : lifetime_ptr(nullptr) {}

  // Change to lifetime<checked>& !!
  optional_lock(lifetime<checked> &life) : lifetime_ptr(&life) {
    Op::acquire(life);
  }

  optional_lock(optional_lock &&other) : lifetime_ptr(other.lifetime_ptr) {
    other.lifetime_ptr = nullptr;
  }

  optional_lock(const optional_lock &other) : lifetime_ptr(other.lifetime_ptr) {
    if (lifetime_ptr)
      Op::acquire(*lifetime_ptr);
  }

  optional_lock &operator=(const optional_lock &other) {
    if (other.lifetime_ptr)
      Op::acquire(*other.lifetime_ptr);
    if (lifetime_ptr)
      Op::release(*lifetime_ptr);
    lifetime_ptr = other.lifetime_ptr;
    return *this;
  }

  optional_lock &operator=(optional_lock &&other) {
    if (lifetime_ptr)
      Op::release(*lifetime_ptr);
    lifetime_ptr = other.lifetime_ptr;
    other.lifetime_ptr = nullptr;
    return *this;
  }

  ~optional_lock() {
    if (lifetime_ptr)
      Op::release(*lifetime_ptr);
  }

  lifetime<checked> &lifetime() const {
    if (!lifetime_ptr)
      throw invalid_operation<Op>();
    return *lifetime_ptr;
  }

private:
  detail::lifetime<checked> *lifetime_ptr;
};

} // namespace detail

struct shared_read {
  static void acquire(detail::lifetime<checked> &life) {
    life.readers++;
    if (life.writers) {
      --life.readers;
      throw invalid_operation<shared_read>();
    }
  }

  static void acquire_move(detail::lifetime<checked> &life) { life.readers++; }

  static void release(detail::lifetime<checked> &life) { life.readers--; }

  static void acquire(detail::lifetime<unchecked>) {}
  static void release(detail::lifetime<unchecked>) {}
};

struct shared_use {};

struct exclusive_use {};

struct exclusive_write {
  static void acquire(detail::lifetime<checked> &life) {
    // Beware sequencing
    if (life.writers++ || life.readers) {
      life.writers--;
      throw invalid_operation<exclusive_write>();
    }
  }

  static void acquire_move(detail::lifetime<checked> &life) { life.writers++; }

  static void release(detail::lifetime<checked> &life) { --life.writers; }

  static void acquire(detail::lifetime<unchecked>) {}
  static void release(detail::lifetime<unchecked>) {}
};

} // namespace safe
