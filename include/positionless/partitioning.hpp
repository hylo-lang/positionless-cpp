#pragma once

#include "positionless/detail/precondition.hpp"

#include <concepts>
#include <iterator>
#include <utility>
#include <vector>

namespace positionless {

/// A separation of some collection into multiple contiguous parts.
///
/// A partitioning is constructed from a range defined by a pair of iterators.
/// The range must remain valid for the lifetime of the partitioning and the iterators given to
/// constructor most not be invalidated.
///
/// - Invariant: parts_count() >= 1
template <std::forward_iterator Iterator> class partitioning {
public:
  using iterator = Iterator;
  using value_type = std::iter_value_t<Iterator>;
  using difference_type = std::iter_difference_t<Iterator>;

  /// An instance covering the range [begin, end), having just one part.
  constexpr partitioning(Iterator begin, Iterator end);

  /// Returns the number of parts in the partitioning.
  [[nodiscard]]
  size_t parts_count() const noexcept;

  /// Returns the iterators delimiting the `i`th part.
  ///
  /// - Precondition: `i < parts_count()`
  [[nodiscard]]
  std::pair<Iterator, Iterator> part(size_t i) const noexcept;

  /// Returns `true` if the `i`th part is empty.
  [[nodiscard]]
  bool is_part_empty(size_t i) const noexcept;

  /// Returns the size of the `i`th part.
  ///
  /// Complexity: O(1) for random access iterators, O(n) otherwise.
  [[nodiscard]]
  size_t part_size(size_t i) const;

  /// Increases the size of the `i`th part by moving its end
  /// boundary forward by one element, and decreasing the size of the next part.
  ///
  /// - Precondition: `i + 1 < parts_count()`
  /// - Precondition: !is_part_empty(i + 1)
  void grow(size_t i);

  /// Increases the size of the `i`th part by moving its end
  /// boundary forward by `n` elements, and decreasing the size of the next part.
  ///
  /// - Precondition: `i + 1 < parts_count()`
  /// - Precondition: `part_size(i + 1) >= n`
  /// - Complexity: O(n) for forward iterators, O(1) for random access iterators
  void grow_by(size_t i, size_t n);

  /// Transfers all the elements of `i`th part to `i-1`th part, making the former empty.
  ///
  /// - Precondition: `0 < i < parts_count()`
  void transfer_to_prev(size_t i);

  /// Transfers all the elements of `i`th part to `i+1`th part, making the former empty.
  ///
  /// - Precondition: `i < parts_count() - 1`
  void transfer_to_next(size_t i);

  /// Adds a new empty part at the end of the `i`th part.
  void add_part_end(size_t i);

  /// Adds a new empty part at the beginning of the `i`th part.
  void add_part_begin(size_t i);

  /// Adds `count` new empty parts at the end of the `i`th part.
  void add_parts_end(size_t i, size_t count);

  /// Adds `count` new empty parts at the beginning of the `i`th part.
  void add_parts_begin(size_t i, size_t count);

  /// Removes the `i`th part, growing the previous part to cover its range.
  ///
  /// - Precondition: `0 < i < parts_count()`
  void remove_part(size_t i);

  /// Decreases the size of the `i`th part by moving its end boundary back by one element, and
  /// increasing the size of the next part.
  ///
  /// - Precondition: `i + 1 < parts_count()`
  /// - Precondition: !is_part_empty(i)
  void shrink(size_t i)
    requires std::bidirectional_iterator<Iterator>;

  /// Decreases the size of the `i`th part by moving its end
  /// boundary back by `n` elements, and increasing the size of the next part.
  ///
  /// - Precondition: `i + 1 < parts_count()`
  /// - Precondition: `part_size(i) >= n`
  /// - Complexity: O(n) for bidirectional iterators, O(1) for random access iterators
  void shrink_by(size_t i, size_t n)
    requires std::bidirectional_iterator<Iterator>;

private:
  /// The boundaries of each part in the partitioning.
  ///
  /// The first element is the begin iterator of the range, and the last
  /// element is the end iterator of the underlying range.
  std::vector<Iterator> boundaries_{};
};

// Inline definitions

template <std::forward_iterator Iterator>
inline constexpr partitioning<Iterator>::partitioning(Iterator begin, Iterator end) {
  boundaries_.reserve(10);
  boundaries_.emplace_back(std::move(begin));
  boundaries_.emplace_back(std::move(end));
}

template <std::forward_iterator Iterator>
inline size_t partitioning<Iterator>::parts_count() const noexcept {
  return boundaries_.size() - 1;
}

template <std::forward_iterator Iterator>
inline std::pair<Iterator, Iterator> partitioning<Iterator>::part(size_t i) const noexcept {
  PRECONDITION(i < parts_count());
  return {boundaries_[i], boundaries_[i + 1]};
}

template <std::forward_iterator Iterator>
inline bool partitioning<Iterator>::is_part_empty(size_t i) const noexcept {
  PRECONDITION(i < parts_count());
  auto [begin, end] = part(i);
  return begin == end;
}

template <std::forward_iterator Iterator>
inline size_t partitioning<Iterator>::part_size(size_t i) const {
  PRECONDITION(i < parts_count());
  return std::distance(boundaries_[i], boundaries_[i + 1]);
}

template <std::forward_iterator Iterator> inline void partitioning<Iterator>::grow(size_t i) {
  PRECONDITION(i + 1 < parts_count());
  PRECONDITION(!is_part_empty(i + 1));
  boundaries_[i + 1]++;
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::grow_by(size_t i, size_t n) {
  PRECONDITION(i + 1 < parts_count());

  if constexpr (std::random_access_iterator<Iterator>) {
    // For random access iterators, we can check size and advance in O(1)
    auto [begin, end] = part(i + 1);
    PRECONDITION(static_cast<size_t>(std::distance(begin, end)) >= n);
    boundaries_[i + 1] += n;
  } else {
    // For forward iterators, we need to check and advance step by step
    for (size_t k = 0; k < n; ++k) {
      PRECONDITION(!is_part_empty(i + 1));
      boundaries_[i + 1]++;
    }
  }
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::transfer_to_prev(size_t i) {
  PRECONDITION(0 < i);
  PRECONDITION(i < parts_count());
  // Transfer all elements to previous part by moving the boundary between them
  boundaries_[i] = boundaries_[i + 1];
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::transfer_to_next(size_t i) {
  PRECONDITION(i < parts_count() - 1);
  // Transfer all elements to next part by moving the boundary between them
  boundaries_[i + 1] = boundaries_[i];
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_part_end(size_t i) {
  PRECONDITION(i < parts_count());
  boundaries_.insert(boundaries_.begin() + i + 1, boundaries_[i + 1]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_part_begin(size_t i) {
  PRECONDITION(i < parts_count());
  boundaries_.insert(boundaries_.begin() + i, boundaries_[i]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_parts_end(size_t i, size_t count) {
  PRECONDITION(i < parts_count());
  boundaries_.insert(boundaries_.begin() + i + 1, count, boundaries_[i + 1]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_parts_begin(size_t i, size_t count) {
  PRECONDITION(i < parts_count());
  boundaries_.insert(boundaries_.begin() + i, count, boundaries_[i]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::remove_part(size_t i) {
  PRECONDITION(i < parts_count());
  boundaries_.erase(boundaries_.begin() + i);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::shrink(size_t i)
  requires std::bidirectional_iterator<Iterator>
{
  PRECONDITION(i + 1 < parts_count());
  PRECONDITION(!is_part_empty(i));
  boundaries_[i + 1]--;
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::shrink_by(size_t i, size_t n)
  requires std::bidirectional_iterator<Iterator>
{
  PRECONDITION(i + 1 < parts_count());

  if constexpr (std::random_access_iterator<Iterator>) {
    // For random access iterators, we can check size and advance in O(1)
    auto [begin, end] = part(i);
    PRECONDITION(static_cast<size_t>(std::distance(begin, end)) >= n);
    boundaries_[i + 1] -= n;
  } else {
    // For bidirectional iterators, we need to check and advance step by step
    for (size_t k = 0; k < n; ++k) {
      PRECONDITION(!is_part_empty(i));
      boundaries_[i + 1]--;
    }
  }
}

} // namespace positionless
