#pragma once

#include "positionless/partitioning.hpp"
#include "rapidcheck_wrapper.hpp"

#include <algorithm>
#include <numeric>
#include <forward_list>

// A `std::forward_list` with a partitioning over its elements.
template <typename T> struct forward_list_partitioning {
  using container_t = std::forward_list<T>;
  using iterator_t = typename container_t::iterator;

  /// The underlying data.
  container_t data_;
  /// The partitioning over the data.
  positionless::partitioning<iterator_t> partitioning_;

  /// An instance with `k` parts over the elements of `d`.
  explicit forward_list_partitioning(container_t d, size_t k = 1)
      : data_(std::move(d)), partitioning_(data_.begin(), data_.end()) {
    if (k > 1) {
      partitioning_.add_parts_begin(0, k - 1);
    }
  }
};

namespace rc {

/// RapidCheck generator for forward_list_partitioning<T>
template <typename T> struct Arbitrary<forward_list_partitioning<T>> {
  static Gen<forward_list_partitioning<T>> arbitrary() {
    return gen::exec([]() {
      const auto n = *gen::inRange<size_t>(0, 64);
      auto data = *gen::container<std::vector<T>>(n, gen::arbitrary<T>());

      const auto maxK = std::max<size_t>(1, n == 0 ? 4 : std::min<size_t>(n, 8));
      const auto k = *gen::inRange<size_t>(1, maxK + 1);

      // Initially have only one part covering all data.
      forward_list_partitioning<T> r(typename forward_list_partitioning<T>::container_t(data.begin(), data.end()), 1);

      if (k == 1) {
        return r;
      }

      // Generate k+1 cut points in [0, n], include 0 and n, sort.
      std::vector<size_t> sizes =
          *gen::container<std::vector<size_t>>(k - 1, gen::inRange<size_t>(0, n + 1));
      sizes.push_back(0);
      sizes.push_back(n);
      std::sort(sizes.begin(), sizes.end());

      // Transform in-place to adjacent differences;
      // erase the first element (0), and the last element (to n).
      std::adjacent_difference(sizes.begin(), sizes.end(), sizes.begin());
      sizes.erase(sizes.begin());
      sizes.pop_back();

      // Add the parts, with the generated sizes.
      for (size_t part_len : sizes) {
        r.partitioning_.add_part_begin(r.partitioning_.parts_count() - 1);
        r.partitioning_.grow_by(r.partitioning_.parts_count() - 2, part_len);
      }

      return r;
    });
  }
};

} // namespace rc
