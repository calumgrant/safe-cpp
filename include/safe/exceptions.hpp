#pragma once

#include <stdexcept>

namespace safe {
struct shared_read;
struct exclusive_write;

class exception : public std::runtime_error {
protected:
  exception(const char *msg) : std::runtime_error(msg) {}
};

template <typename Op> class invalid_operation;

template <> class invalid_operation<exclusive_write> : public exception {
public:
  invalid_operation() : exception("invalid write") {}
};

template <> class invalid_operation<shared_read> : public exception {
public:
  invalid_operation() : exception("invalid read") {}
};

using invalid_write = invalid_operation<exclusive_write>;
using invalid_read = invalid_operation<shared_read>;

class null_pointer : public exception {
public:
  null_pointer() : exception("null pointer") {}
};

} // namespace safe