#pragma once

#include "positionless/detail/precondition.hpp"
#include "positionless/partitioning.hpp"

#include <algorithm>
#include <iterator>

namespace positionless {

/// Swaps the first element from part `i` with the first element from part `j`.
///
/// - Precondition: `i < p.parts_count()`
/// - Precondition: `j < p.parts_count()`
/// - Precondition: parts `i` and `j` are not empty
template <std::forward_iterator Iterator>
inline void swap_first(partitioning<Iterator>& p, size_t i, size_t j) {
  PRECONDITION(i < p.parts_count());
  PRECONDITION(j < p.parts_count());
  PRECONDITION(!p.is_part_empty(i));
  PRECONDITION(!p.is_part_empty(j));

  auto [begin_i, end_i] = p.part(i);
  auto [begin_j, end_j] = p.part(j);

  std::iter_swap(begin_i, begin_j);
}

} // namespace positionless
