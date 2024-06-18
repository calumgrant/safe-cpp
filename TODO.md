How to print a string?

Consider if we want to use inheritance for backwards-compatibility?

Usability vs safety.
- We want a drop-in replacement.

- Create references to self

- [ ] Disable range checks in release mode

class MyObject : public safe::self
{
  void call()
};


safe::value<T>
safe::ref<T>
safe::ref<const T>
safe::ptr<T>
safe::container<T>

- [ ] Mode = shared
  shared_debug or shared_release

# Data model




Safe objects are wrapped in a `value<T>` wrapper which is designed to protect the object from undefined behaviour. A `object` creates and manages a single instance of `T`.

To access a `object`, you need to create a `ref<T>` or `ref<const T>`, corresponding to `T&` and `const T&`. You can also use a `ptr<T>` and `ptr<const T>`, which can be null.

To actually use an object, you create a `use<T>` or `use<const T>`.  You can also have a `back_ref<T>` and `back_ref<const T>`, and `back_ptr<T>` or `back_ptr<const T>`. Unlike `std::weak_ptr`, the checks of `back_ref` can be disabled, and there is no API to test whether the underlying object is valid or not.

`iterator<It>` wraps an iterator, and prevents modification of the container by also holding a `ref<const C>`, and can return a `ref<value_type>` (which can be const or non-const).

  object<T> ->* ref<T> ->* ref<const T> -> use<const T> -> T
            ->* ref<T> -> use<T> -> T

## Safety rules (and how they are implemented):

`ref<T>` and `ptr<T>` are mutually exclusive with all other references to the same object - there can only be one instance of a `ref<T>`.  

Borrowing: You can borrow a `ref<T>` and `ref<const T>` from a `ref<T> X`, whereupon `X` enters a `borrowed` state and can no longer be used or borrowed again. But you *can* borrow from a borrowed object.

When a ref goes out of scope, it releases the object it borrowed.

## Containers and iterators

`iterator<It>` holds a `ref<const C>` to the container, and returns a `ref<T>` or `ref<const T>` when the iterator is dereferenced.

Iterator safety: All iterator arithmetic and operations are validated on the container `C`. All pointer arithmetic (on `ptr<T>` is validated against a container??? HOW???

## Strings



Invariants
- No const reference may exist beyond the lifetime of a reader or writer
- No non-const reference may live beyond the lifetime of a writer
- There is at most one writer
- There are no concurrent readers and writers
- Checks do not change the lifetime semantics

# How this is enforced

Objects have a "state" which is none/reading(n)/writing/destroyed. Objects can only be accessed via a "reader" (`ref`) or a writer (`mut`).

But, `mut` actually creates an `exclusive_mut<>` which does not allow sharing, and only this provides `->`.

Danger is something like `m->foo()` which MUST be in an exclusive region. We need to actually make it non-borrowable

This seems extremely annoying!!

Throw an exception

Borrowing:
A writer has its own lifetime object, and readers or writers can "borrow" access from the writer. In the borrowed state, a writer can no longer access the object.
?? Can this be hacked?

State machine for an object. Implement as `state`.
  1. None
  2. Reading (n readers)
  3. Writing (1 writer)
  4. Destroyed

For a mut, it also has a "state"
object<T> -> mut<T> -> non_borrowable_mut<T> -> T&
                    -> ref<T> -> const T&
                              -> ref<T> ...
                    -> mut<T> ...
          -> ref<T> -> ...



container<C> -> mut<cw<C>> -> ref<cw<C>>
                           -> mut<element<C>> ->
             -> ref<cw<T>> ->
             -> ref<const cw<T>> ->  
```
What about iterators?



mut<X> m;
m->update(

Implementation.
- [ ] Should there simply be `atomic<int> *` as the thing to borrow??

An object, or a 

- If we have a mut ref, then we can also "borrow" from this
    

API usability
- There are too many barriers/incompatibilities with std::vector etc.
- safe::vector should be a drop-in replacement for std::vector
- Problem is that keeping 2 writes alive isn't good.
- Ensure that benchmark is easy to use.
  `*` and `[]` return ref-types on the `vector_impl`
    - But we'll need an alternative to get proper refs?

- Make unchecked containers more efficient

Dereference: 

Invariants:
- An object has zero or more readers
- An object has at most one reader
- An object does not have readers and writers

This crashes:
ref<int> r2 = r2;  // WHAT THE FUCK

```
safe::vector<int> items = ...

safe::ref<int> i = items[1];
safe::ref<int> j = *items.begin();
safe::mut<int> k = items.begin()[1];
```

Rules:
- You cannot get a mut ref to the container if there are any refs to the elements
    - But you can get a mut ref to elements there is a read ref to the container
- There is at most one mut ref to the container or its contents
    - You cannot have mut refs to 2 different elements
- You can have a mut ref to the elements and read refs to the container
- Iterators create read refs to the container
    - Including non-const iterators
- When we destroy the container (or mut ref it), there must be no other readers or writers to the container or its elements

How it works:
- Iterators are "readers" of the object, this prevents "erase" and the like
- Even non-const iterators are "readers"
- When you dereference an iterator, you create a writer on the "contents"
  - When we destroy a container, we also check for contents writers.
- When we have a contents-ref, we also have a reader-ref.
  - We cannot acquire a m

- Rename "lifetime" to "access..." and "lock" to "access...".


```
class container_impl
{
    Container container;
    lifetime object_access;
    lifetime contents_access;
};
```


optional_ref<T, Operation, Mode>
{
};

optional_lifetime...
 -> optional_read
 -> optional<Op>
- Assignable...


Nullable iterators
- can lock an object?
- a nullable ref
- iterators and locks are assignable??
- !! Cannot increment a "null" iterator

Weak objects!
Check memory leaks



nullable_ref<Op,Mode> container;
nullable_container_ref<>
Can we use the null object pattern? E.g. a static container
static object<Container,Mode>



Containers:
A `container` holds a specific container

`object<container<std::vector>>` marshalls read or write access to the container.

BUT, `container` actually needs a lifetime for `begin()/end()`.

Making iterators safe by returning borrowed references to the item+vector

Difference between:

- safe::vector<std::string> and
- std::vector<safe::string> and
- safe::vector<safe::string>

- Can a safe container guard its contents? e.g.

safe::ref<std::string> r1 = vec[12];
safe::ref<std::string> r2 = *vec.begin();

Problem is multiple writers for the iterator.

for(auto r : vec)
{
}

Problem is multiple writers. if a iterator is  a reader, how can it be a writer?
- As an exception, we can have multiple writers for iterators.

```
auto it  = vec.begin(), end = vec.end();
while(it != end)
{
    mut m = *It;
}

object -> reader
object -> writer -> shared_write (chained)
                 -> exclusive_write (chained)
                 -> shared_read (chained)
```

iterator: This has the "shared_write" mode that allows multiple writers, 
const_iterator: This has the "shared_reader"



- Implement ptr as we'll need this for iterators
- Copy constructor for `mut` if we mean the same thing?
- `c_str()` to return `ptr<char>`

How to get references to containers?
Null iterators.
- Implement a "mutable container" which is a mut<>.

- Safe exceptions??

How to distinguish

```c++
safe::object<Foo> foo;

auto w = foo.write();

foo->modify();  // Want to prevent this because there's a writer
w->modify();    // Ok because it refers to the same writer
```

```c++
safe::string s1;
s1.write().resize(10);  // Does not work!
s1.resize(10);   // ?? 
```

- How to implement this pattern in general?
- recursive_writer()?


- default constructor for an iterator.

Iterator safety:
- Check inc and dec for std::list etc
- Arbitrary pointer arithmetic
- Reverse iterators
- Can we have a ref to a safe::string ??

# Bulletproof C++

What's unsafe:
1. References and pointers
  a. Dangling reference/pointer
  b. Use after free/delete
  c. Concurrent access
  d. Array out of bounds
2. Iterators
  a. Iterator arithmetic
  b. Use of invalidated iterator

Solution:
1. Replace all raw references and pointers with `safe::ref<T>` and `safe::ptr<T>`
2. Replace all iterators with `safe::iterator<It>`.



What are the guarantees?

1. Don't return "raw" references (including iterators, pointers, or classes that contain iterators and pointers (e.g. string_view)).
2. 

What could go wrong?
- return reference
- uninitialized iterators
- null iterator?
- 

How to prevent refences

x.write([](mut<X> m) {
    // Nothing preventing access to x at this point?
});

- [ ] Look at `std::uncaught_exception()`

# Totally safe string

- Dangling references are impossible
- Buffer overflows are impossible
- Dangling iterators are impossible

Dangling references are the big problem

```
safe::string str1;
safe::string & str2 = str1;   // !! No

safe::string::iterator i1 = str1.begin();

safe::string::ref r1 = str1;
safe::string::mut m1 = str1;
safe::string::ptr p1 = str1;


safe::ref<std::string> s1;   // ???
```

Rules:
- Don't store pointers and refs in fields
- 



template <typename C, typename It, typename Op, typename Mode> class iterator_implXX {
  // Iterator category etc.
public:
  using value_type = typename C::value_type;

  // static object<C> empty_container;

  // iterator_impl() : it(empty_container.unsafe_read().begin()), checks(empty.unsafe_read()), {}

  iterator_implXX(It it, iterator_checks<C, Mode> checks, detail::lifetime<Mode>::ptr lifetime)
      : it(it), checks(checks), lock(lifetime) {}

  bool operator!=(const iterator_implXX &other) const { return it != other.it; }

  iterator_implXX &operator++() {
    // checks.check_inc(it);
    ++it;
    return *this;
  }

  // Could return a mut or a ref!!
  // !! No, actually we want to return a special reference type
  ref<value_type, Mode> operator*() {
    // checks.check_deref(it);
    return {*it, lock.ref()};
  }
  // typename It::reference operator*() const { return *it; }

private:
  It it;
  // Pointer to container??
  iterator_checks<C, Mode> checks;
  detail::lock<Op, Mode> lock;
};

// DELETEME ===============================================
template <typename C, typename Mode = mode> class container0 {
public:
  using value_type = typename C::value_type;
  using difference_type = typename C::difference_type;
  using size_type = typename C::size_type;
  // !! etc

  template <typename... Args>
  container0(Args &&...args) : value(std::forward<Args>(args)...) {}

  struct iterator {
  public:
    iterator(typename C::iterator it, detail::iterator_checks<C, Mode> c,
             typename detail::lifetime<Mode>::ptr l)
        : it(it), checks(c), lock(l) {}
    bool operator!=(const iterator &other) const { return it != other.it; }

    iterator &operator++() {
      checks.check_inc(it);
      ++it;
      return *this;
    }

    iterator operator++(int) {
      checks.check_inc(it);
      auto old = *this;
      it++;
      return old;
    }

    iterator &operator--() {
      checks.check_dec(it);
      ++it;
      return *this;
    }

    iterator operator--(int) {
      checks.check_dec(it);
      auto old = *this;
      it++;
      return old;
    }

    iterator operator+(difference_type delta) const {
      // Only supported for random access iterators
      // Checked on deref
      return {it + delta, checks, lock.ref()};
    }

    iterator operator-(difference_type delta) const {
      // Only supported for random access iterators
      return {it - delta, checks, lock.ref()};
    }

    // TODO: +=, -=

    typename C::value_type &operator*() {
      checks.check_deref(it);
      return *it;
    }

  private:
    typename C::iterator it;
    detail::iterator_checks<C, Mode> checks;
    detail::lock<shared_read, Mode> lock;
  };

  struct const_iterator {
  public:
    const_iterator(typename C::const_iterator it,
                   typename detail::lifetime<Mode>::ptr l)
        : it(it), lock(l) {}

  private:
    typename C::const_iterator it;
    detail::lock<shared_read, Mode> lock;
  };

  // using iterator = safe::iterator<typename C::iterator, write, Mode>;
  // using const_iterator = safe::iterator<typename C::const_iterator, read,
  // Mode>;

  iterator begin() {
    return {value.unsafe_read().begin(), value.unsafe_read(), value.lifetime()};
  }
  const_iterator begin() const { return cbegin(); }
  const_iterator cbegin() const {
    return {value->begin(), *value, value.lifetime()};
  }

  iterator end() {
    return {value.unsafe_read().end(), value.unsafe_read(), value.lifetime()};
  }
  const_iterator end() const { return cend(); }
  const_iterator cend() const {
    return {value->end(), this->value, value.lifetime()};
  }

  using ref = safe::ref<container0<C, Mode>, Mode>;
  using mut = safe::mut<container0<C, Mode>, Mode>;

  ref read() const { return {*this, value.lifetime()}; }

  // What about operations on the mut? Double-write??
  mut write() { return {*this, value.lifetime()}; }

  size_type size() const { return value->size(); }

  // Operations

  // Operator -> and operator *

  // !! No don't support this directly
  void push_back(const value_type &item) { value.write()->push_back(item); }
  void resize(size_type new_size) { value.write()->resize(new_size); }

protected:
  object<C, Mode> value;
};

// Unsafe iterator arithmetic
