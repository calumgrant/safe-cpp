// Welcome is the Safe C++ tutorial.
//
// Safe C++ is a set of classes and idioms that allows you to write C++ code
// that is "safe", meaning that there will be no undefined behaviour.

#include "safe/string.hpp"
#include "safe/value.hpp"
#include "safe/vector.hpp"

#include <iostream>
#include <list>

template <typename Ex = std::exception, typename Fn> void throws(Fn f) {
  try {
    f();
  } catch (const Ex &e) {
    return;
  }
  std::cerr << "Expected exception not thrown" << std::endl;
  exit(1);
}

#define THROWS(X) throws([&]() { X; });
#define TERMINATES(X)

using namespace safe;

class MyObject {
public:
  // These are "safe" strings
  string name1, name2;
};

int main() {

  // 1. Values
  // Safe values are stored in a `value` class
  // A value is stored on the stack, and provides a safe wrapper around an
  // object.

  // 1.1. Creating safe values

  // Create a stack-allocated safe value o.
  value<MyObject> o = MyObject{"Alice", "Bob"};

  // Create a stack-allocated integer
  value<int> i = 42;

  // 1.2. Accessing safe values

  // You can access the underlying object using `*`
  std::cout << **i << std::endl;
  i = 35;

  // You can access fields using ->
  // Since name1 is a safe string, we need to dereference it using *.
  std::cout << **o->name1 << std::endl;

  // 2. References
  // References come in two flavors: mutable and immutable.
  // Mutable references have type `ref<T>`, and are analogous to `T&`.
  // Immutable references have type `ref<const T>` and are analogous to `const
  // T&`.

  // Like the programming language Rust, mutable references are exclusive,
  // meaning that it is an error to have more than one mutable reference to the
  // same value at any one time. Unlike Rust, this is checked at run-time, not
  // compile-time. However, these checks are very cheap, and can be disabled in
  // release builds.

  // 2.1 Mutable references

  {
    // A mutable reference has the type ref<T>.
    // A mutable reference allows the referenced value to be modified.
    // It is analagous to `MyObject&w`, and must be initialized.
    ref<MyObject> w1{o};
    ref<int> w2{i};

    // You can modify the underlying object
    w2 = 35;
    w1 = MyObject{"Charlie", "David"};

    // You can access fields of a reference using ->
    w1->name1 = "Eve";

    // Mutable references are exclusive, so attempts to
    // create a second mutable reference to the same value will throw
    // exceptions.
    THROWS(ref<MyObject> w4{o})
  }

  {
    // The write() methods on an object also returns a mutable reference.
    auto w = o.write();
    w->name1 = "Annabel";
  }

  // 2.2 Immutable references

  {
    // An immutable reference has the type ref<const T>.
    // An immutable reference does not allow the value to be modified.
    // It is analagous to const MyObject&w, and must be initialized with a
    // value.
    ref<const MyObject> r1{o};
    ref<const int> r2{i};

    // You can access fields of a reference using ->
    std::cout << **r1->name1 << std::endl;
    std::cout << *r2 << std::endl;

    // Unlike mutable references, you can have many immutable references to the
    // same value.
    ref<const MyObject> r3{o};

    // The read() method returns an immutable reference.
    auto r4 = o.read();

    // Mutable references are incompatible with immutable references, and
    // attempts to create a mutable reference whilst there are immutable
    // references will throw exceptions.
    THROWS(o.write());
  }

  // 2.3. Borrowing

  // When a reference is created, it puts the underlying value into a "borrowed"
  // state, meaning that, like a library book, you cannot lend the same book
  // twice at the same time. This is designed to prevent race conditions and
  // errors caused by concurrent or reentrant access. Values can be "borrowed
  // for reading" or "borrowed for writing". Internally, this is implemented
  // using a counter. Like a library, if you have borrowed a book, you can lend
  // it to someone else.

  {
    ref<MyObject> w1{o};  // Borrow from o
    ref<MyObject> w2{w1}; // Borrow from w1

    {
      // You can borrow immutable references from mutable references
      ref<const MyObject> r1{w2}; // Borrow from w1

      // Whilst a mutable reference is "borrowed", you cannot use it yourself.

      THROWS(o.write());           // o is borrowed by w1
      THROWS(w2->name1 = "Alice"); // w2 is borrowed by r2
    }

    // Now that we returned r1, we can use w2 again
    w2->name1 = "Alice";
  }

  // 2.4 Dangling references

  // If a value or reference gets destroyed in a "borrowed" state, then
  // the application will terminate. This is to prevent dangling references.

  {
    auto fn1 = []() -> ref<const int> {
      value<int> i;
      return i.read(); // std::terminate() called
    };

    auto fn2 = [](ref<int> w) -> ref<const int> {
      return w.read(); // std::terminate() called
    };
    TERMINATES(fn1());
    TERMINATES(fn2(i));
  }

  // 3. Containers

  // You can wrap any container in order to prevent unsafe
  // operations. Standard containers are riddled with undefined behaviour.

  // 3.1 Creating containers

  // Containers are created using the `container<T>` class.
  // Containers are values.

  // Create an empty list
  container<std::list<int>> list;

  // There are a number of built-in containers which are
  // simply typedefs for container<T>.
  // A string is also a container.
  vector<int> vec = {1, 2, 3};
  string str = "Foo";

  // 3.2 Accessing containers

  // For the most part, containers behave like their standard library
  // counterparts.

  std::cout << "The list has " << vec.size() << " items" << std::endl;

  // References to containers have the usual semantics.

  {
    // ref<T> is a reference to a container, and is itself a container.
    auto r = vec.read();

    // You cannot have a mutable ref at the same time as an immutable ref
    THROWS(vec.write());
  }

  // 3.3 References to elements

  // Unlike the standard containers, elements are accessed via the ref<> class
  // This adds an additional level of safety.

  {
    auto r = vec.read();
    ref<const int> first_item = r.front();
    auto last_item = r.back();

    // If the container supports random access, then you can use the [] operator
    auto second_item = r[1];

    // The [] operator behaves like at(), and throws std::out_of_range if the
    // index is invalid
    THROWS(vec[-1]);

    // Of course, you cannot create a mutable reference to the vector whilst it
    // is in a borrowed state
    THROWS(vec.write());
    THROWS(vec.front());

    // But you can request immutable reference:
    vec.read().front();
  }

  {
    // We can also create mutable references to elements
    vec.front() = 9;
    auto w = vec.write();
    w.front() = 10;
    ref<int> first_item = w.front();
    first_item = 11;

    // But we cannot obtain a second mutable reference to an element, even if
    // it's a different element This prevents overlapping modification of
    // elements of the same container
    THROWS(w.back() = 12);
  }

  // 3.4 Iterators

  // Containers can be iterated, for example using a range-based for loop.
  for (ref<int> i : vec) {
    std::cout << "The value is " << *i << std::endl;
  }

  // You can of course iterate over references to the container
  for (ref<const int> i : vec.read()) {
    // ...
  }

  // Iterators borrow an immutable reference to the container
  // which allows multiple iterators to coexist (e.g. begin() and end()!)
  // and prevents modification/insertion of the container whilst iterating.
  for (auto i : vec) {
    THROWS(vec.push_back(*i));
  }

  // Safe iterators prevent invalid dereferences, for example
  THROWS(*vec.end());

  // Safe iterators detect invalid iterator arithmetic.
  THROWS(++vec.end());

  // 4. Pointers

  // Pointers are very similar to references, and come in
  // two flavors: mutable and immutable.
  // Mutable pointers have the type `ptr<T>`, and are analogous to `T*`.
  // Immutable pointers have the type `ptr<const T>`, and are analogous to
  // `const T*`.
  //
  // Unlike references:
  // - pointers can be null
  // - pointers can be reassigned.
  // Unlike C pointers,
  // - You cannot do pointer arithmetic (use iterators instead)
  // - Pointers are initialized to null
  // - Null pointers throw exceptions when dereferenced
  // - Dangling pointers terminate the program

  // 4.1. Creating pointers.

  // This creates a nullptr:
  ptr<int> p1;

  // Use the & operator to create a pointer from a value or reference
  { auto p2 = &i; }
  { ptr<const int> p3 = &i.read(); }
  // Reassign a pointer to null
  p1 = nullptr;

  //  p3 = p2;  // Assign a mutable to an immutable pointer

  // 4.2 Dereferencing pointers

  // The * operator turns a pointer back into a ref,
  // and is needed to access the underlying value.

  {
    ptr<int> p = &i;
    ref<int> r = *p;
  }

  THROWS(*p1); // p1 is nullptr
  p1 = &i;
  *p1 = 42;

  // You can access members of a pointer using ->
  (&o)->name1 = "Agnes";

  // Dangling pointers
  {
    // Pointers put their value into a borrowed state, so dangling pointers will
    // cause the program to terminate

    auto fn1 = []() {
      value<int> i;
      return &i; // std::terminate() called
    };

    TERMINATES(fn1());
  }
}
