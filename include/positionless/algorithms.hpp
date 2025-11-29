#pragma once

#include "positionless/partitioning.hpp"

#include <algorithm>
#include <iterator>

namespace positionless {

/// Swaps the first element from part `i` with the first element from part `j`.
///
/// - Precondition: `i < parts_count()`
/// - Precondition: `j < parts_count()`
/// - Precondition: parts `i` and `j` are not empty
template <std::forward_iterator Iterator>
inline void swap_first(partitioning<Iterator>& p, size_t i, size_t j) {
  assert(i < p.parts_count());
  assert(j < p.parts_count());
  assert(!p.is_part_empty(i));
  assert(!p.is_part_empty(j));

  auto [begin_i, end_i] = p.part(i);
  auto [begin_j, end_j] = p.part(j);

  std::iter_swap(begin_i, begin_j);
}

} // namespace positionless
