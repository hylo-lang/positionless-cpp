#pragma once

#include "positionless/partitioning.hpp"
#include "rapidcheck_wrapper.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

namespace testgen {

/// Generates `k` parts to fill up `n` elements, and returns the sizes of the first `k-1` parts.
inline std::vector<size_t> generate_partition_sizes(size_t n, size_t k) {
  if (k == 0)
    return {};
  if (k == 1)
    return {n};
  std::vector<size_t> r =
      *rc::gen::container<std::vector<size_t>>(k - 1, rc::gen::inRange<size_t>(0, n + 1));
  r.push_back(0);
  r.push_back(n);
  std::sort(r.begin(), r.end());
  std::adjacent_difference(r.begin(), r.end(), r.begin());
  r.erase(r.begin());
  r.pop_back();
  for (size_t s : r) {
    PRECONDITION(s <= n);
  }
  return r; // size k-1; the last part will be inferred
}

/// Generate an arbitrary split into parts for partitioning `p`.
///
/// Precondition: `p.parts_count() == 1`
template <typename It> auto generate_splits(positionless::partitioning<It>& p) {
  PRECONDITION(p.parts_count() == 1);
  const size_t n = static_cast<size_t>(std::distance(p.part(0).first, p.part(0).second));
  const size_t maxK = std::max<size_t>(1, n == 0 ? 4 : std::min<size_t>(n, 8));
  const size_t k = *rc::gen::inRange<size_t>(1, maxK + 1);

  if (k > 1) {
    auto sizes = generate_partition_sizes(n, k);
    for (size_t part_len : sizes) {
      p.add_part_begin(p.parts_count() - 1);
      p.grow_by(p.parts_count() - 2, part_len);
    }
  }
  PRECONDITION(p.parts_count() == k);
}

} // namespace testgen
