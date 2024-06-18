
#include "test.hpp"
#include <iostream>

// Includes for the safe library

// This is support for basic values
#include <safe/value.hpp>

// This is support for containers

// Safe strings
#include <safe/string.hpp>

// Safe vectors
#include <safe/vector.hpp>

#include <list>

int main() {
  // Namespace - all symbols are in the `safe` namespace
  using namespace safe;

  // 1) Objects
  // An object with a default constructor
  value<int> a;

  // You can explicitly specify the checks ("checked" or "unchecked")
  // By default, it is "checked" for debug builds and "unchecked" for release
  // builds
  value<int, checked> a2;
  value<int, unchecked> a3;

  // An value with a value constructor
  value<int> b1 = 42;
  value<int, unchecked> b2{42};

  // To assign the value, use the = operator
  a = 10;
  // To access the value, you need to dereference the value
  // The first * obtains a reference, the second * dereferences the reference
  // this is so that the reference remains live for the duration of the value
  // access
  std::cout << **a << std::endl;

  // 2) References
  // References come in two flavors: read-only and write
  // Read-only references have the type ref<T>

  {
    // Constructing a readonly reference from an value
    ref<const int> r1 = a;

    // The read() method returns a reference
    auto r2 = a.read();

    // You can have as many read-only references as you like
    auto r3 = r2;

    // Avoid C++ references as these aren't safe
    const int &r4 = *a.read(); // Incorrect - native references are not safe

    // You can access a read-only reference using the * and -> operators
    int copy = *r1;
  }

  // mutable references have type mut<T>
  // Of course, these allow you to modify the value
  {
    // Constructing a mutable reference from an value
    ref<int> m1 = a;
  }

  {
    // The write() method returns a reference
    auto m2 = a.write();
  }

  {
    ref<int> m1 = a;

    // Unlike read-only references, you cannot have more than one mutable
    // reference
    assert_throws<invalid_write>([&] { a.write(); });

    // Any attempt to obtain a read reference will throw an exception
    assert_throws<invalid_read>([&] { a.read(); });

    // You can also borrow a read reference from a mutable reference
    {
      auto r1 = m1.read();
      ref<const int> r2 = m1;
    }

    // You can access the underlying value using the * and -> operators
    *m1 = 10;
  }

  **a = 10;
  *a.write() = 10;

  // Unlike shared_ptr, references do not manage value lifetimes. Instead, they
  // *check* value lifetimes, and these checks can be turned off without
  // changing program behaviour.

  {
    // If the underlying value gets destroyed, then its destructor will
    // terminate the program
    auto f = []() {
      value<int> a;
      return a.read(); // Bang! std::terminate is called
    };
  }

  {
    value<int> a;
    ref<int> m = a;
    ref<const int> r = m;
    // Not allowed as there's already a reader on m.
    assert_throws<invalid_write>([&] { ref<int> m2 = m; });
  }

  // 3) Containers
  // It's possible to wrap any container with safe::container
  // This detects dangling references and invalid iterators.

  // Here is a safe list.
  container<std::list<int>> list;

  // Here is a safe vector.
  // safe::vector<T> is just a typedef for safe::container<std::vector<T>>
  safe::vector<int> vec;

  {
    // You can create references using the read() and write() methods
    auto r = vec.read();
    ref<const vector<int>> r1 = vec;

    // Naturally, reading and writing are mutually exclusive
    assert_throws<invalid_write>([&] { vec.write(); });
  }

  // You can only have one writer at a time.
  {
    auto w = vec.write();
    assert_throws<invalid_write>([&] { vec.write(); });
  }

  // References to elements
  // References to elemets are guarded by the lifetime of the container
  // This means that if you destroy a container with live
  // references to any element, then the program will terminate.

  {
    // Create a container
    vector<int> vec;

    // Add an element
    vec.push_back(42);

    // Create a reference to the element
    ref<int> r = vec[0];

    *r = 10;

    // We cannot get a mutable reference to another element
    // whilst the initial reference is live.
    // This is just in case the two references are to the same element,
    // which would violate our invariants
    assert_throws<invalid_write>([&] { vec[0]; });

    // We cannot perform writing operations on a container if there
    // are other references (readers or writers)
    assert_throws<invalid_write>([&] { vec.push_back(5); });
  }

  {
    // If you destroy a container with live references, then the program will
    // terminate

    auto f = [&]() -> ref<int> {
      safe::vector<int> vec;
      vec.push_back(42);
      return vec[0]; // Terminate called here
    };
  }

  // TODO: Initialize a vector with a list of elements
  {
    // safe::vector<int> vec = {1,2,3};
  }

  // Obtaining multiple readers
  {
    safe::vector<int> vec; //. = {1,2,3};
    vec.push_back(1);

    // vec = {1,2,3};
    {
      ref<int> r1 = vec[0]; // !! How to get a ref??
    }
    ref<const int> m1 = vec.read().at(0);

    // We cannot get a writer to the container or an element whilst there
    // are any references to the writer or element

    assert_throws<invalid_write>([&] { vec.write(); });
  }

  // Obtaining a reference to an invalid member throws an exception

  {
    vector<int> vec;
    assert_throws<std::out_of_range>([&] { vec[0]; });
    assert_throws<std::out_of_range>([&] { vec.at(0); });
    assert_throws<std::out_of_range>([&] { vec.read().at(0); });
    assert_throws<std::out_of_range>([&] { vec.write().at(0); });
  }

  // Iterator safety

  // Uninitialized iterators are invalid
  {
    safe::vector<int>::iterator it;
    assert_throws<std::out_of_range>([&] { *it; });
  }

  {
    // Iterating a container
    safe::vector<int> collection{1, 2, 3};
    {
      auto w = collection.write();
      auto i = w.begin(); // !! Make sure we read this from the element
    }
    for (auto i : collection.write()) {
      *i = 10;
    }
    assert(*collection[0] == 10);
  }

  // 4) Strings

  // 5) Multithreading

  // Vectors of things

  {
    std::vector<safe::value<int>> vec;
    vec.emplace_back(1);
    vec.push_back(2);
    assert(***vec[1] == 2); // Yuck!
    auto writer = vec[1].write();
  }

  // Safe iterators
  safe::vector<int> ints;

  // Safe strings

  safe::string str = "Hello, world!";

  int i1 = *str[0];

  // Error here:
  {
    auto x = str.read();
    x.begin();
  }

  for (auto c : str.read()) {
    std::cout << *c;
  }

  std::cout << std::endl;

  for (auto c : str.write()) {
    std::cout << *c;
  }

  std::cout << std::endl;
  for (auto c : str) {
    std::cout << *c;
  }
  std::cout << std::endl;

  // Not tested yet

  try {
    safe::string::iterator it1;
    *it1; // Error - iterator is not initialized
    assert(!"Should not get here");
  } catch (std::out_of_range &) {
    // Expected
  }

  // Test an invalid iterator
  safe::string str2;
  try {
    *str2.begin();
    assert(!"Should not get here");
  } catch (const std::out_of_range &) {
    // Expected
  }

  container<std::string, unchecked> str3;
  // Unchecked read ok (but dodgy)
  auto x = *str3.begin();

  container<std::list<int>, checked> list1;
  try {
    ++list1.begin(); // Error - cannot go past the end
    assert(!"Should not get here");
  } catch (const std::out_of_range &) {
    // Expected
  }

  try {
    --list1.begin(); // Error - cannot go past the end
    assert(!"Should not get here");
  } catch (const std::out_of_range &) {
    // Expected
  }

  // Attempt to write to a reading container
  safe::vector<int> items;
  items.push_back(1);
  try {
    for (auto i : items)
      items.push_back(**i);
    assert(!"Should not get here");
  } catch (const safe::invalid_write &) {
    // Expected
  }

  // Pointers

  {
    safe::value<int> a;
    ptr<int> p1;
    assert(!p1);

    p1 = &a;
    assert(p1);
    **p1 = 42;
  }

  // References to a safe container
  {
    safe::string str1;
    str1.read().size();
    str1.resize(5);
    str1.write().resize(10); // !! Problem - resize() attempts to write to
    // the container as well.

    // !!
    // safe::string::iterator i;
  }

  // Can you resize a container when an element reference exists?
  {
    safe::vector<int> vec;
    vec.push_back(1);
    auto r = vec[0];
    try {
      vec.resize(2);
      assert(!"Should not get here");
    } catch (invalid_write &) {
    }
  }

  // Chaining
  {
    value<int> a;
    auto b = a.write();
    auto c = b.write(); // Ok as it's chained
  }

  // c_str
  {
    // !! TODO
    safe::string str = "Hello, world!";
    // auto p = str.c_str();
    // std::cout << p.get() << std::endl;
  }

  // More values
  {
    string str = "abc";
    str.size();
    str.read().size();
    str.write().resize(2);
    assert(str.read().size() == 2);

    for (auto ch : str.read()) {
      std::cout << *ch;
    }

    string str2{str};
    str2 = str;
  }

  // Functions
  {
    string str = "Hello, world!";
    auto fn = [](ref<const string> r) { std::cout << r.size() << std::endl; };
    fn(str);
    fn(str.read());
    fn(str.write());
    fn(str.write().read());
  }

  return 0;
}