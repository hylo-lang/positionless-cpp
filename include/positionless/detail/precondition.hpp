#pragma once

#if defined(NDEBUG)

#include <exception>

/// Assert-like precondition check that calls `std::terminate` on failure.
#define PRECONDITION(expr)                                                                         \
  do {                                                                                             \
    if (!(expr))                                                                                   \
      std::terminate();                                                                            \
  } while (false)
#else

#include <cassert>
#define PRECONDITION(expr) assert(expr)

#endif