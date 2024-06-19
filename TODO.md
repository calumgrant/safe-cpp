
- Create references to self

- [ ] Disable range checks in release mode
- [ ] Check memory leaks
- [ ] Implement all missing methods

```c++

void assign_self(ptr<T> self, has_self<T> &obj)
{
  obj.self = self;
}

template<typename T>
class has_self
{
public:
    ptr<T> self;
};

class MyObject : public safe::has_self<MyObject>
{
  void call()
};
```

# Implementation

`value<T>` contains a "lifetime" record implemented using `defail::lifetime`. This records the status of the object:
- number of readers
- number of writers (0 or 1)
- weak count
- is the object live

In `unchecked` mode, the lifetime record is empty, and in `weak` mode, the lifetime object is stored on the heap and is only destroyed when all pointers to the lifetime go out of scope (tracked by the weak count). Weak mode is a little more expensive so is not the default.

A mutable ref throws an exception in its constructor if there are any existing readers or writers. Of course, we need to take threading into consideration in case multiple threads are attempting to acquire the ref at the same time. A ref is a bit like a "lock", and once acquired guarantees safe use of the object for the duration of the lock.

In order to borrow from a mutable ref, mutable refs carry their own lifetime. For example `value<int>().write().write()` actually has 3 lifetime objects: one in the value, a second in the first `write()` and a third in the final `write()`. The first `write()` borrows from the object, and the second `write()` borrows from the first `write()`.

Immutable refs don't need their own lifetime as they can just use the lifetime of the underlying value, since multiple readers are allowed.

Containers have an additional lifetime for their elements. Whenever you create an iterator, say using `begin()`, you borrow a read from the container, even if the container or iterator are non-const. This prevents operations on the container like erasing or moving elements for the duration of the iterator.

When you dereference an iterator, you borrow from the *element lifetime* of the container. Iterators contain a pointer to their container (in checked mode), and use this to verify all iterators before dereferencing or performing iterator arithmetic.

Pointers only keep a pointer to the value and its lifetime, and do not affect the lifetime other than the weak count. When you dereference a pointer to create a ref, the constructor of the ref performs the required to checks to ensure that the ref is valid.

# Differences to shared_ptr

Safe C++ appears to offer a similar functionality to `std::shared_ptr`, but they are in fact very different.

`shared_ptr` is about the safe *management* of object lifetimes, whereas Safe C++ is about safe *checking* of object lifetimes. Safe C++ leverages existing C++ object management techniques, and does not add additional overheads. `shared_ptr` requires that the values are allocated on the heap, whereas Safe C++ does not need any heap allocation.

Safe C++ checks can be turned off.



Invariants
- No const reference may exist beyond the lifetime of a reader or writer
- No non-const reference may live beyond the lifetime of a writer
- There is at most one writer
- There are no concurrent readers and writers
- Checks do not change the lifetime semantics


API usability
- safe::vector should be a drop-in replacement for std::vector
- Make unchecked containers more efficient


Invariants:
- An object has zero or more readers
- An object has at most one reader
- An object does not have readers and writers


- [ ] Reverse iterators



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


# Totally safe string

- Dangling references are impossible
- Buffer overflows are impossible
- Dangling iterators are impossible


