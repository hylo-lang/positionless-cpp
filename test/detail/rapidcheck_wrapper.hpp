#pragma once

#include <doctest/doctest.h>
#include <exception>
#include <rapidcheck.h>
#include <rapidcheck/gen/Arbitrary.h>

/// Defines a test case that uses RapidCheck to check a property.
#define TEST_PROPERTY(name, body)                                                                  \
  TEST_CASE(name) { CHECK(rc::check(name, body)); }
