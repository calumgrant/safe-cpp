# Safe C++

Completely safe C++

This library defines a set of classes that can be used to write 100% safe C++ code.

It catches the following errors:
- Dangling references
- Invalid/null/uninitialized/dangling pointers
- Invalid iterators
- Unsafe pointers/iterators
- Array out of bounds/buffer overflows
- Multithreading errors

The checks can be turned off, and then the code will be as fast and unsafe as regular unsafe code.

Use cases:
- C++ codebases where you want it to be robust but also interface to other C++ code.
- Where you have a lot of legacy C++ code and don't want to switch languages.
- For critical applications where security and availability are important.
- Making parts of a C++ codebase robust, whilst the performant parts can still run without checks.
- Ensuring code is robust in a debug build, then turning checks off in the release.
- Failing early and loudly, preventing insidious errors

The guarantee: If you don't use language-level pointers and references, don't throw exceptions in destructors, and if you don't use the standard containers, then there will be no undefined behaviour.

The first person who can prove me wrong will get $100. By the way, I do expect to lose this wager at some point due to a bug somewhere, but hopefully such bugs will not be fundamental. I also expect to need to add more caveats to the guarantee.

By "safe", we mean no undefined behaviour. Code can still fail, but will fail in a specified and sometimes recoverable manner.

## Quick examples

Protecting against dangling references:

```c++
safe::ref<string> f1()
{
    safe::string result = "fred";
    return result;          // std::terminate() called
}
```

Protecting against null pointers:

```c++
void f2()
{
    safe::ptr<string> str;
    *str = "Hello";         // safe::null_ptr thrown
}
```

Protecting against multithreading errors:

```c++
safe::string str;

void thread_fn()
{
    str.write() = "Fred";       // safe::invalid_write thrown
}
```

Protecting against invalid iterators:

```c++
safe::vector<int> vec;
*vec.begin();                   // std::out_of_range thrown

for(auto i : vec)
    vec.write().push_back(i);   // safe::invalid_write thrown

auto it = vec.begin();
vec.resize(100);
*it = 10;                       // safe::out_of_range thrown
```

Protecting against buffer overflows:

```c++
safe::vector<int> vec2;
std::copy(vec.begin(), vec.end(), vec2.begin());    // std::out_of_range thrown
```

Protecting against other invalid container operations:

```c++
vec2[100] = 10;     // std::out_of_range thrown
vec2.front();       // std::out_of_range thrown
vec2.back();        // std::out_of_range thrown
```

# Safe objects

The container `safe::value<T>` holds an object of type T. To access T itself, you must call `*`, `read()` or `write()` to get a `ref<T>` or a `mut<T>` which is a mutable or immutable reference.

```c++
#include "safe/object.hpp"

class Foo
{
public:
    void resize(int new_size);
    int size() const;
};

int main()
{
    safe::object<Foo> foo;
    foo.read()->size();
    foo.write()->resize(10);
}
```

`read()` and `write()` are mutually exclusive, much like the Rust programming language, so

```c++
    safe::object<Foo> foo;
    auto foo_ref = foo.read();
    auto foo_mut = foo.write();     // Throws safe::invalid_write
```

Unlike Rust, `read`/`write` are not checked at compile time.

Multiple readers are permitted, but there can only be a single writer.

# Disabling runtime checks



# Safe containers

## Strings

`safe::string` is a string type with safe iterators.

Protection against invalid indexes:

```c++
safe::string str;
str[0] = 'a';    // Throws std::out_of_range
```

Protection against invalid iterators:

```c++
*str.end()=0;  // Throws std::out_of_range
std::copy(vec.begin(), vec.end(), str.begin());    // Throws std::out_of_range
```

Use `std::range` to safely represent a range of characters (as opposed to `std::string_view`).

`c_str()` returns a `safe::ptr<char>` ??

For practical reasons, `safe::string` has `c_str()` and `string_view()` functions.
These
- Must not be returned from a function
- Must not be stored in a field.
- Must not be stored beyond the lifetime of the object.

## Vectors

## Other containers

## Safe pointers

# Converting programs to use safe C++

Step 1: Ensure that all fields and return values don't contain references/iterators/pointers (RIPs).
a. Redesign your code to avoid returning RIPs. Prefer to return by value instead (smart pointers are fine).
b. For cases where these are unavoidable, replace these with safe RIPs (safe::ref<>, safe::iterator<> and safe::ptr<>). You will need to also change the type of the referee to be safe::object.

Step 2: Change all argument types to be value-types or safe RIPs.

Step 2: Ensure that all containers and strings use safe:: types

# Reference

## Exceptions

## Thread safety

## `safe::object<T, Mode>`

This stores an instance of type `T`.

Types:

- `value_type`

Methods:

- `object(...)` constructor - constructs the value
- `read()` - returns a `ref<T>`
- `write()` - returns a `mut<T>`
- `~object()` - terminates the program if there are active references

Operators:
- `*`
- `=` assignment

Exceptions:

Thread safety:

## `safe::shared_object<T>`

A shared object is the same as `safe::object`, but it allows for more complex lifetime management. In particular, it permits `weak_ref` and `weak_mut`.

Note that this does *NOT* have the same behaviour as `std::shared_ptr`, because references only check the lifetime of an object, but do not keep objects alive. This allows runtime checks to be disabled without changing the behaviour of the program. If your intention is for the reference to manage the lifetime of an object, use `std::shared_ptr`.

## `safe::ref<T, Mode>`

If `T` is `const`, then this is an immutable/readonly reference, otherwise this creates a mutable/read-write reference. The underlying reference cannot be null.

Typedefs:
- `using value_type = T;`

Operators:
- `value_type * operator->()`
- `writer<T> operator*()`

Constructors:
- `ref(object<T>)`
- `ref(shared_object<T>)`
- `ref(mut<T>)`
- `ref(const weak_ref<T>&)`
- `ref(const ref<T>&)`

## `safe::iterator<It, Mode>`

## `safe::ptr<T, Mode>`

## `safe::weak_ptr<T, Mode>`

## `safe::weak_ref<T, Mode>`

The same as `safe::ref`, except that exceptions are thrown on access, rather than on the destruction of the underlying object. This is more permissive and handles situations like cyclical data structures.

## `safe::string`

Alias for `safe::object<std::string>`.

## `safe::vector<T>`

Alias for `safe::object<std::vector<T>>`.


