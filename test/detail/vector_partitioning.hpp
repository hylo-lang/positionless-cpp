#pragma once

#include "partitioning_generators.hpp"
#include "positionless/partitioning.hpp"
#include "rapidcheck_wrapper.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

// A `std::vector` with a partitioning over its elements.
template <typename T> struct vector_partitioning {
  using container_t = std::vector<T>;
  using iterator_t = typename container_t::iterator;

  /// The underlying data.
  container_t data_;
  /// The partitioning over the data.
  positionless::partitioning<iterator_t> partitioning_;

  /// An instance with `k` parts over the elements of `d`.
  explicit vector_partitioning(container_t d, size_t k = 1)
      : data_(std::move(d)), partitioning_(data_.begin(), data_.end()) {
    if (k > 1) {
      partitioning_.add_parts_begin(0, k - 1);
    }
  }
};

namespace rc {

/// RapidCheck generator for vector_partitioning<T>
template <typename T> struct Arbitrary<vector_partitioning<T>> {
  static Gen<vector_partitioning<T>> arbitrary() {
    return gen::exec([]() {
      const auto n = *gen::inRange<size_t>(0, 64);
      auto data = *gen::container<std::vector<T>>(n, gen::arbitrary<T>());
      vector_partitioning<T> r(std::move(data));
      testgen::generate_splits(r.partitioning_);
      return r;
    });
  }
};

} // namespace rc
