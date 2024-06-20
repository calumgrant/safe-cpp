# Safe C++ tutorial

Safe C++ is a set of classes and idioms that allows you to write C++ code
that is "safe", meaning that there will be no undefined behaviour.

See [tutorial.cpp](test/tutorial.cpp) for the full source code of this file.

## Overview

Safe C++ provides the classes

- `value<T>` - wraps a value of type `T`.
- `ref<T>` - a reference to `T`, with Rust-style borrowing.
- `container<C>` - wraps a container `C`.
- `ptr<T>` - a pointer to type `T`.
- Exception classes `invalid_read`, `invalid_write`, `expired_pointer`, `null_pointer`.

Use of these classes has no undefined behaviour. To make your code robust, wrap all containers in `container<>`, and replace all pointers and references with `ptr<T>` and `ref<T>` instead.

These classes only *check* the object lifetimes, but never *manage* or *change* object lifetimes. As a result, the checks can be disabled for release builds.

## Header and namespace

All types are in the `safe` namespace.

The main header file is `<safe/safe.hpp>`, but you can also include individual header files as well: `<safe/value.hpp>`, `<safe/ptr.hpp>`, `<safe/ref.hpp>` and `<safe/container.hpp>`.

There are also typedefs for `string` in `<safe/string.hpp>` and `vector<T>` in `<safe/vector.hpp>`.

## Values

First, let's define an object we want to make safe, `MyObject`.

```c++
#include <safe/safe.hpp>
#include <safe/string.hpp>

using namespace safe;

class MyObject {
public:
  // These are "safe" strings
  string name1, name2;
};
```

To create a safe value, wrap it in a `value<>` class:

```c++
    value<MyObject> o;
    value<int> i = 42;
```

Values are stored inline and do not allocate memory. You can access the underlying object using `*`. The double `*` is needed because first we acquire an exclusive lock on the object, and then we dereference it again to get the `MyObject&`. This whole thing would be a bit easier with language support but then it wouldn't be C++ any more.

```c++
    std::cout << **i << std::endl;
    i = 35;
```

You can access fields using `->`. Since `name1` is a safe string, we need to dereference it using `*` to get the `std::string&`.

```c++
    std::cout << **o->name1 << std::endl;
```

The purpose of `value<>` is so that we can create safe references to them.

## References

References come in two flavors: mutable and immutable.
Mutable references have type `ref<T>`, and are analogous to `T&`.
Immutable references have type `ref<const T>` and are analogous to `const T&`.

Like the programming language Rust, mutable references are exclusive,
meaning that it is an error to have more than one mutable reference to the
same value at any one time. Unlike Rust, this is checked at run-time, not
compile-time. However, these checks are very cheap, and can be disabled in
release builds.

### Mutable references

A mutable reference has the type `ref<T>`. A mutable reference allows the referenced value to be modified.

```c++
    ref<MyObject> w1 = o;
    ref<int> w2 = i;
```

With a mutable reference, you can modify the underlying object

```c++
    w1 = MyObject{"Charlie", "David"};
    w2 = 35;
```

You can access fields of a reference using `->`

```c++
    w1->name1 = "Eve";
```

Mutable references are exclusive, so attempts to create a second mutable reference to the same value will throw an `invalid_write`
exception.

```c++
    ref<MyObject> w1 = o;
    ref<MyObject> w2 = o; // throws invalid_write due to `w1`
```

The `value::write()` method also returns a mutable reference.

```c++
    auto w = o.write();
    w->name1 = "Annabel";
```

### Immutable references

An immutable reference has the type `ref<const T>`. An immutable reference does not allow the value to be modified.

```c++
    ref<const MyObject> r1 = o;
    ref<const int> r2 = i;
```

You can access the underlying object using `*`, and members using `->`

```c++
    std::cout << **r1->name1 << std::endl;
    std::cout << *r2 << std::endl;
```

Unlike mutable references, you can have many immutable references to the same value.

The `read()` method returns an immutable reference.

```c++
    auto r4 = o.read();
```

Mutable references are incompatible with immutable references, and
attempts to create a mutable reference whilst there are immutable
references will throw an exception.

```c++
    auto r = o.read();
    o.write();   // throws `invalid_write` since `r` is in scope.
```

Similarly, trying to create a immutable reference when there is already an active writer throws `invalid_read`:

```c++
    auto w = o.write();
    o.read();    // Throws `invalid_read` since `w` is in scope
```

Multiple readers are fine of course:

```c++
    auto r1 = o.read();
    auto r2 = o.read();     // Ok
```

### Borrowing

When a reference is created, it puts the underlying value into a "borrowed"
state, meaning that, like a library book, you cannot lend the same book
twice at the same time. This is designed to prevent race conditions and
errors caused by concurrent or reentrant access. Values can be "borrowed
for reading" or "borrowed for writing". Internally, this is implemented
using a counter. Like a library, if you have borrowed a book, you can lend
it to someone else.

```c++
    ref<MyObject> w1 = o;  // Borrow from `o`
    ref<MyObject> w2 = w1; // Borrow from `w1`
```

You can borrow immutable references from mutable references. You can have multiple readers at the same time.

```c++
      ref<const MyObject> r1{w2}; // Borrow from `w1`
      auto r2 = w2.read();
```

Whilst a mutable reference is borrowed, you cannot use it yourself.

```c++
    w2->name1 = "Alice"; // throws invalid_write because `w2` is borrowed by `r1` and `r2`
```

### Dangling references

If a value or reference gets destroyed in a "borrowed" state, then
the application will terminate. This is to prevent dangling references.

```c++
ref<const int> f1() {
    value<int> i;
    return i.read(); // std::terminate() called
};

ref<const int> f2(ref<int> w) {
    return w.read(); // std::terminate() called
};
```

The second example is invalid because we are returning a reference to `w`.

## Containers

You can wrap any container in order to prevent unsafe
operations. Standard containers are riddled with undefined behaviour.

### Creating containers

Containers are created using the `container<T>` class. Containers are values. There are a number of built-in containers which are simply typedefs for `container<T>`. A `string` is also a container.

```c++
#include <safe/string.hpp>
#include <safe/vector.hpp>

container<std::list<int>> list;
vector<int> vec = {1, 2, 3};
string str = "Foo";
```

### Accessing containers

For the most part, containers behave like their standard library counterparts.

```c++
  std::cout << "The list has " << vec.size() << " items" << std::endl;
```

References to containers have the usual semantics. `ref<T>` is a reference to a container, and is itself a container.

```c++
    auto r = vec.read();
    vec.write();  // throws invalid_write
```

### Accessing elements

Elements are accessed via the `ref<>` class. This makes the elements safe as well, even if they are not explicitly `value`.

```c++
    auto r = vec.read();
    ref<const int> first_item = r.front();
    auto last_item = r.back();
```

If the container supports random access, then you can use the subscript operator

```c++
    auto second_item = r[1];
```

The `[]` operator behaves like `at()`, and throws `std::out_of_range` if the
index is invalid, but only when checks are enabled.

```c++
    vec[-1];   // Throws std::out_of_range
```

Element references put the container in a "borrowed" state. (Technically, the container is in a reading state, whilst the element state is either reading or writing depending on the type of reference.) So you cannot create a mutable reference to the container whilst there are element references.

```c++
    auto r = vec.front();
    vec.read();     // Ok, multiple readers
    vec.write();    // Throws invalid_write
```

We can create mutable references to elements

```c++
    vec.front() = 9;
    auto w = vec.write();
    w.front() = 10;
    ref<int> first_item = w.front();
    first_item = 11;
```

But we cannot obtain a second mutable reference to an element, even if
it's a different element This prevents overlapping modification of
elements of the same container.

```c++
    auto w1 = vec.front();
    auto w2 = vec.back();  // Throws invalid_write
```

We can however have immutable references to different elements in the same container.

```c++
    auto r1 = vec.read().front();
    auto r2 = vec.read().back();    // Ok
```

### Iterators

Containers can be iterated, for example using a range-based for loop.

```c++
  for (ref<int> i : vec) {
    std::cout << "The value is " << *i << std::endl;
  }
```

You can iterate over references to the container

```c++
  for (ref<const int> i : vec.read()) {
    // ...
  }
```

Iterators borrow an immutable reference to the container
which allows multiple iterators to coexist (e.g. `begin()` and `end()`!)
and prevents modification/insertion of the container whilst iterating.

```c++
  for (auto i : vec) {
    vec.push_back(*i);  // Throws invalid_write
  }
```

Iterator arithmetic is always checked and must result in a valid iterator.

```c++
    vector<int>::iterator it1, it2 = vec.begin(), it3 = it2 + 2;

    // Invalid iterator arithmetic
    it3 + 10;      // Throws std::out_of_range
    it3[10];       // Throws std::out_of_range
    ++vec.end();   // Throws std::out_of_range

    // Invalid dereferences
    *it1;          // Throws std::out_of_range
    *vec.end();    // Throws std::out_of_range
```

## Pointers

Safe pointers are very similar to references, and come in
two flavors: mutable and immutable. Mutable pointers have the type `ptr<T>`, and are analogous to `T*`.
Immutable pointers have the type `ptr<const T>`, and are analogous to
`const T*`.

Unlike safe references,
 - safe pointers can be null
 - safe pointers can be reassigned
 - safe pointers don't borrow the underlying value
 - you cannot create a safe pointer from a reference, because a pointer to a reference isn't very useful

Unlike C pointers,
 - You cannot do pointer arithmetic (use iterators instead)
 - Safe pointers protect from null or uninitialised access
 - Safe pointers protect against illegal dereferences of null or destroyed objects 

Unlike `std::shared_ptr`,
 - Safe pointers are non-owning, and do not alter the lifetime of the object they point to
 - Safe pointers don't require the object to be heap-allocated.

### Creating pointers.

The default constructor creates a nullptr:

```c++
    ptr<int> p1;
```

Use the `&` operator to create a pointer from a value. Pointers don't borrow so you can have as many pointers as you like.

```c++
    ptr<int> p2 = &i;
    ptr<const int> p3 = &i;
```

You can assign pointers, or assign `nullptr` to a pointer.

```c++
    p1 = nullptr;
    p3 = p2;
```

### Dereferencing pointers

The `*` operator turns a pointer back into a `ref`,
and is needed to read or write the underlying value.
This borrows the value, so may throw exceptions.

```c++
    ptr<int> p = &i;
    ref<int> r = *p;
    *p;         // Throws invalid_read because i is already borrowed by r

    *p1;        // Throws null_pointer because p1 is nullptr
    p1 = &i;
    *p1 = 42;   // Assign the value of a pointer
```

You can access members of a pointer using `->`

```c++
  (&o)->name1 = "Agnes";
```

### Dangling pointers

A "dangling" pointer means that the object that a pointer is pointing to has gone out of scope and been deleted. In normal C++, this has undefined behaviour.

With Safe C++, the object keeps track of the number of references, and calls `std::terminate()` if an attempt is made to destroy an object that still has references or pointers to it.

```c++
ptr<int> fn1() {
    value<int> i;
    return &i;   // std::terminate() called
}
```

There are cases where we want to allow dangling pointers, for example in cyclical data structures.
In this case, we can use a weak lifetime, specified by the `weak` checks on `value`. (Weak checks are still disabled by default on release builds.)

```c++
ptr<int> fn2() {
    value<int, weak> i;
    return &i;   // Ok
}
```

Using `weak` allows pointers to dangle, but any attempt to dereference the pointer throws `expired_pointer`.

```c++
    ptr<int> p = fn2();  // Ok, but p is expired
    *p;                  // Throws expired_pointer
```
