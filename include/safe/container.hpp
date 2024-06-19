#pragma once

#include "value.hpp"

namespace safe {

namespace detail {
template <typename Container, typename Mode,
          typename IteratorCategory =
              typename Container::const_iterator::iterator_category>
struct iterator_checks {
  static void check_deref(const Container &container,
                   const typename Container::const_iterator &it)  {}
  static void check_inc(const Container &container,
                 const typename Container::const_iterator &it)  {}
  static void check_dec(const Container &container,
                 const typename Container::const_iterator &it)  {}
};

template <typename Container, typename IteratorCategory>
struct iterator_checks<Container, unchecked, IteratorCategory> {
  static void check_deref(const Container &container,
                   const typename Container::const_iterator &it)  {}
  static void check_inc(const Container &container,
                 const typename Container::const_iterator &it)  {}
  static void check_dec(const Container &container,
                 const typename Container::const_iterator &it)  {}
};

template <typename Container, typename IteratorCategory>
struct iterator_checks<Container, checked, IteratorCategory> {
  static void check_deref(const Container &container,
                   const typename Container::const_iterator &it)  {}
  static void check_inc(const Container &container,
                 const typename Container::const_iterator &it)  {
    if (it == container.end())
      throw std::out_of_range("out of range");
  }
  static void check_dec(const Container &container,
                 const typename Container::const_iterator &it)  {
    if (it == container.begin())
      throw std::out_of_range("out of range");
  }
};

template <typename Container>
struct iterator_checks<Container, checked, std::random_access_iterator_tag> {
  static void check_deref(const Container &container, const typename Container::const_iterator &it)  {
    if (it < container.begin() || it >= container.end())
      throw std::out_of_range("out of range");
  }

  static void check_inc(const Container & container, const typename Container::const_iterator &it)  {
    if (it == container.end())
      throw std::out_of_range("out of range");
  }

  static void check_range(const Container &container,
                          const typename Container::const_iterator &it) {
    if (it < container.begin() || it > container.end())
      throw std::out_of_range("out of range");
  }
};

template <typename C, typename Mode> class container_impl {
public:
  using size_type = typename C::size_type;
  using container_type = C;
  using value_type = typename C::value_type;
  using lifetime_type = detail::lifetime<Mode>;
  using checks = iterator_checks<C, Mode>;

  template <typename It, typename ContainerRef, typename ValueRef>
  class iterator_impl {
  public:
    iterator_impl() : it{}, container{} {}

    iterator_impl(It it, ContainerRef container, lifetime_type &lifetime)
        : it(it), container(container), container_lock(lifetime) {}

    // ValueRef read() const {

    ValueRef operator*() { /* Race conditions?? */
      if (!container)
        throw std::out_of_range("uninitialized iterator");
      checks::check_deref(container->container, it);
      return {*it, container->element_access};
    }

    ValueRef operator[](int i) {
      if (!container)
        throw std::out_of_range("uninitialized iterator");
      checks::check_deref(container->container, it + i);
      return {it[i], container->element_access};
    }

    bool operator!=(const iterator_impl &other) const { return it != other.it; }

    iterator_impl &operator++() {
      if (!container)
        throw std::out_of_range("uninitialized iterator");
      checks::check_inc(container->container, it);
      ++it;
      return *this;
    }

    iterator_impl &operator--() {
      if (!container)
        throw std::out_of_range("uninitialized iterator");
      checks::check_dec(container->container, it);
      --it;
      return *this;
    }

    iterator_impl operator+(int n) {
      if (!container)
        throw std::out_of_range("uninitialized iterator");
      checks::check_range(container->container, it + n);
      return {it + n, container, container_lock.lifetime()};
    }

    iterator_impl operator-(int n) {
      if (!container)
        throw std::out_of_range("uninitialized iterator");
      checks::check_range(container->container, it - n);
      return {it - n, container, container_lock.lifetime()};
    }

  private:
    It it;
    ContainerRef container;
    detail::optional_lock<shared_read, Mode> container_lock;
  };

public:
  using iterator = iterator_impl<typename C::iterator, container_impl *,
                                 safe::ref<value_type, Mode>>;
  using const_iterator =
      iterator_impl<typename C::const_iterator, const container_impl *,
                    safe::ref<const value_type, Mode>>;

  // Accessors
  iterator begin() { return {container.begin(), this, container_access}; }
  iterator end() { return {container.end(), this, container_access}; }
  const_iterator begin() const {
    return {container.begin(), this, container_access};
  }
  const_iterator end() const {
    return {container.end(), this, container_access};
  }

  // Operations

  // Only possible in a write context
  void push_back(const value_type &value) { container.push_back(value); }

  template <typename... Args> void emplace_back(Args &&...args) {
    container.emplace_back(std::forward<Args>(args)...);
  }

  template <typename... Args>
  container_impl(Args &&...args) : container(std::forward<Args&&>(args)...) {}

  container_impl(const container_impl &other) : container(other.container) {}

  container_impl(container_impl &other) : container(other.container) {}

  container_impl(container_impl &&other) : container(std::move(other.container)) {}

  container_impl(std::initializer_list<value_type> il) : container(il) {}

  auto &lifetime() const { return container_access; }
  auto &element_lifetime() const { return element_access; }
  size_type size() const { return container.size(); }
  void resize(size_type s) { return container.resize(s); }
  void clear() { container.clear(); }

  safe::ref<value_type, Mode> operator[](size_type i) {
    if (i >= container.size())
      throw std::out_of_range("out of range");
    return {container[i], element_access};
  }

  safe::ref<const value_type, Mode> operator[](size_type i) const {
    if (i >= container.size())
      throw std::out_of_range("out of range");
    return {container[i], element_access};
  }

  safe::ref<value_type, Mode> at(size_type i) {
    if (i >= container.size())
      throw std::out_of_range("out of range");
    return {container[i], element_access};
  }

  safe::ref<const value_type, Mode> at(size_type i) const {
    if (i >= container.size())
      throw std::out_of_range("out of range");
    return {container[i], element_access};
  }

  ref<value_type, Mode> front() const { 
    if(container.size() == 0)
      throw std::out_of_range("empty container");
    return {container.front(), element_access};
  }

  ref<value_type, Mode> back() const { 
    if(container.size() == 0)
      throw std::out_of_range("empty container");
    return {container.back(), element_access};
  }


  // !!!!!!!
  // private:
public:
  C container;
  mutable detail::lifetime<Mode> container_access, element_access;
};
} // namespace detail

// Immutable reference to a container
template <typename C, typename Mode> class ref<const container<C, Mode>, Mode> {
public:
  using impl_type = detail::container_impl<C, Mode>;
  using container_type = container<C, Mode>;
  using lifetime_type = detail::lifetime<Mode>;
  using value_type = typename C::value_type;
  using size_type = typename C::size_type;

  using iterator = typename container_type::const_iterator;
  using const_iterator = iterator;

  ref(const container_type &c) : ref(c.read()) {}
  // value(c.value), life(c.value.lifetime()) {}

  ref(const impl_type &value, lifetime_type &life)
      : value(value), life(life) {}

  ref(const ref &other) : value(other.value), life(other.life.lifetime()) {}

  ref(ref<container_type, Mode> &&other)
      : value(other.value), life(other.lifetime(), detail::move_tag{}) {}

  //const container_type &operator*() const { return value; }

  //const container_type *operator->() const { return &value; }

  // operator const container_type &() const { return operator*(); }

  ref<const value_type, Mode> front() const { 
    if(value.container.size() == 0)
      throw std::out_of_range("empty container");
    return {value.container.front(), value.element_lifetime()};
  }

  ref<const value_type, Mode> back() const { 
    if(value.container.size() == 0)
      throw std::out_of_range("empty container");
    return {value.container.back(), value.element_lifetime()};
  }

  iterator begin() const { return value.begin(); }
  iterator end() const { return value.end(); }

  const value_type &operator[](size_type i) const { return value.at(i); }
  ref<const value_type, Mode> at(size_type i) const { return {value.container.at(i), value.element_lifetime()}; }

  size_type size() const { return value.size(); }

private:
  const impl_type &value;
  detail::lock<shared_read, Mode> life;
};

// This is the mutable reference
template <typename C, typename Mode> class ref<container<C, Mode>, Mode> {
public:
  using container_type = detail::container_impl<C, Mode>;
  using value_type = typename C::value_type;
  using lifetime_type = detail::lifetime<Mode>;

  ref(container<C,Mode> &c, lifetime_type &life) : value(c.value), life(life) {}

  ref(container_type &value, lifetime_type &life)
      : value(value), life(life) {}
  ref(container<C, Mode> &src) : ref(src.write()) {}
  ref(const ref &other) : value(other.value), life(other.reader) {}
  // mut(object<value_type,Mode>&obj) : mut(obj.unsafe_read(), obj.lifetime())
  // {} mut(shared_object<value_type,Mode>&);

  // Make private??
  // ?? excl()
  ref<const container<C>, Mode> read() const { return {value, reader}; }
private:
  exclusive<container_type, Mode> write() const { return {value, reader}; }
public:

  detail::lifetime<Mode> &lifetime() const { return life.lifetime(); }

  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;
  using size_type = typename container_type::size_type;

  iterator begin() { return {value.container.begin(), &value, reader}; }
  iterator end() { return {value.container.end(), &value, reader}; }
  size_type size() { return value.size(); }

  ref<value_type, Mode> operator[](size_type i) const { return {value.container.at(i), value.element_lifetime()}; }
  ref<value_type, Mode> at(size_type i) const { return {value.container.at(i), value.element_lifetime()}; }
  ref<value_type, Mode> front() const { 
    if(value.container.size() == 0)
      throw std::out_of_range("empty container");
    return {value.container.front(), value.element_lifetime()};
  }

  ref<value_type, Mode> back() const { 
    if(value.container.size() == 0)
      throw std::out_of_range("empty container");
    return {value.container.back(), value.element_lifetime()};
  }


  size_type size() const { return read().size(); }
  void resize(size_type s) { write()->resize(s); }

  template <typename... Args> void emplace_back(Args &&...args) {
    write()->emplace_back(std::forward<Args>(args)...);
  }

  void push_back(const value_type &v) { write()->push_back(v); }

  void clear() { write()->clear(); }

private:
  friend class ref<const container<C, Mode>, Mode>;
  container_type &value;
  detail::lock<exclusive_write, Mode> life;
  mutable detail::lifetime<Mode> reader; // Track readers/writers of this writer
};

template <typename C, typename Mode> class container {
public:
  using container_type = detail::container_impl<C, Mode>; // ??
  using value_type = typename C::value_type;

  template <typename... Args>
  container(Args &&...args) : value(std::forward<Args&&>(args)...) {}

  container(std::initializer_list<value_type> il) : value(il) {}

  container(const container<C,Mode> &other) : value(other.value) {}

  container(container<C,Mode> &other) : value(other.value) {}

  container(container<C,Mode> &&other) : value(std::move(other.value)) {}

  container & operator=(const container<C,Mode> &other) {
    value = other.value;
    return *this;
  }

  container & operator=(container<C,Mode> &&other) {
    value.container = std::move(other.value.container);
    return *this;
  }

  container & operator=(container<C,Mode> &other) {
    value.container = other.value.container;
    return *this;
  }

  template <typename Args>
  container & operator=(Args &&args) {
    value.container = std::forward<Args&&>(args);
    return *this;
  }

  ref<const container, Mode> read() const {
    ref<const container, Mode> tmp_read = {value, value.element_lifetime()};
    return {value, value.lifetime()};
  }

  // !! Problem: We'll want to check there are no references to the elements as
  // well as the object
  ref<container, Mode> write() {
    ref<container, Mode> tmp_write = {value, value.element_lifetime()};
    return {value, value.lifetime()};
  }

  // !! I think this is a terrible idea !!
  // exclusive<const C, Mode> operator->() const { return {value.container, value.lifetime()}; }

  // !! Only for strings!
  exclusive<const std::string, Mode> operator*() const { return {value.container, value.lifetime()}; }

  // This is not iterable??
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;
  using size_type = typename C::size_type;

  // Note: All of these accessors and operations must
  // go via read() or write() to validate access

  ref<value_type, Mode> operator[](size_type i) { return write().at(i); }

  ref<const value_type, Mode> operator[](size_type i) const {
    return read().at(i);
  }

  ref<value_type, Mode> front() { return write().front(); }
  ref<const value_type, Mode> front() const { return read().front(); }

  ref<value_type, Mode> back() { return write().back(); }
  ref<const value_type, Mode> back() const { return read().back(); }


  iterator begin() { return {value.container.begin(), &value, value.lifetime()}; }
  iterator end() { return {value.container.end(), &value, value.lifetime()}; }

  const_iterator begin() const { return {value.container.begin(), &value, value.lifetime()}; }
  const_iterator end() const { return {value.container.end(), &value, value.lifetime()}; }

  ref<value_type, Mode> at(size_type i) { return write().at(i); }

  ref<const value_type, Mode> at(size_type i) const {
    return {value.at(i), element_lifetime()};  // !! Does this prevent writing??
  }

  size_type size() const { return read().size(); }

  void resize(size_type new_size) { write().resize(new_size); }

  void clear() { write().clear(); }

  // Operations
  void push_back(const value_type &v) { write().push_back(v); }

  template <typename... Args> void emplace_back(Args &&...args) {
    write().emplace_back(std::forward<Args>(args)...);
  }

  detail::lifetime<Mode> & element_lifetime() const { return value.element_lifetime(); }

private:
  container_type value;
};

// Auto-convert value<std::vector<C>> to a container_value??

// Safe char pointer?
} // namespace safe
