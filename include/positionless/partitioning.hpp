#pragma once

#include <cassert>
#include <concepts>
#include <iterator>
#include <utility>
#include <vector>

namespace positionless {

/// A separation of some collection into multiple contiguous parts.
///
/// A partitioning is constructed from a range defined by a pair of iterators.
/// The range must remain valid for the lifetime of the partitioning.
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
  ///
  /// - Invariant: `parts_count() >= 1`
  [[nodiscard]]
  size_t parts_count() const noexcept;

  /// Returns the iterators delimiting the part at index `part_index`.
  ///
  /// - Precondition: `part_index < parts_count()`
  [[nodiscard]]
  std::pair<Iterator, Iterator> part(size_t part_index) const noexcept;

  /// Returns `true` if the part at index `part_index` is empty.
  ///
  /// - Precondition: `part_index < parts_count()`
  [[nodiscard]]
  bool is_part_empty(size_t part_index) const noexcept;

  /// Increases the size of the part at index `part_index` by moving its end
  /// boundary forward by one element, and decreasing the size of the next part.
  ///
  /// - Precondition: `part_index + 1 < parts_count()`
  /// - Precondition: !is_part_empty(part_index + 1)
  void grow(size_t part_index);

  /// Increases the size of the part at index `part_index` by moving its end
  /// boundary forward by `n` elements, and decreasing the size of the next part.
  ///
  /// - Precondition: `part_index + 1 < parts_count()`
  /// - Precondition: size of part `part_index + 1` >= `n`
  /// - Complexity: O(n) for forward iterators, O(1) for random access iterators
  void grow_by(size_t part_index, size_t n);

  /// Adds a new empty part at the end of part `part_index`.
  ///
  /// - Precondition: `part_index < parts_count()`
  void add_part_end(size_t part_index);

  /// Adds a new empty part at the begin of part `part_index`.
  ///
  /// - Precondition: `part_index < parts_count()`
  void add_part_begin(size_t part_index);

  /// Adds `count` new empty parts at the end of part `part_index`.
  ///
  /// - Precondition: `part_index < parts_count()`
  void add_parts_end(size_t part_index, size_t count);

  /// Adds `count` new empty parts at the begin of part `part_index`.
  ///
  /// - Precondition: `part_index < parts_count()`
  void add_parts_begin(size_t part_index, size_t count);

  /// Removes the part at index `part_index`, growing the previous part to
  /// cover its range.
  ///
  /// - Precondition: `0 < part_index < parts_count()`
  void remove_part(size_t part_index);

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
inline std::pair<Iterator, Iterator>
partitioning<Iterator>::part(size_t part_index) const noexcept {
  assert(part_index < parts_count());
  return {boundaries_[part_index], boundaries_[part_index + 1]};
}

template <std::forward_iterator Iterator>
inline bool partitioning<Iterator>::is_part_empty(size_t part_index) const noexcept {
  assert(part_index < parts_count());
  auto [begin, end] = part(part_index);
  return begin == end;
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::grow(size_t part_index) {
  assert(part_index + 1 < parts_count());
  assert(!is_part_empty(part_index + 1));
  boundaries_[part_index + 1]++;
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::grow_by(size_t part_index, size_t n) {
  assert(part_index + 1 < parts_count());

  if constexpr (std::random_access_iterator<Iterator>) {
    // For random access iterators, we can check size and advance in O(1)
    auto [begin, end] = part(part_index + 1);
    assert(static_cast<size_t>(std::distance(begin, end)) >= n);
    boundaries_[part_index + 1] += n;
  } else {
    // For forward iterators, we need to check and advance step by step
    for (size_t i = 0; i < n; ++i) {
      assert(!is_part_empty(part_index + 1));
      boundaries_[part_index + 1]++;
    }
  }
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_part_end(size_t part_index) {
  assert(part_index < parts_count());
  boundaries_.insert(boundaries_.begin() + part_index + 1, boundaries_[part_index + 1]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_part_begin(size_t part_index) {
  assert(part_index < parts_count());
  boundaries_.insert(boundaries_.begin() + part_index, boundaries_[part_index]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_parts_end(size_t part_index, size_t count) {
  assert(part_index < parts_count());
  boundaries_.insert(boundaries_.begin() + part_index + 1, count, boundaries_[part_index + 1]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::add_parts_begin(size_t part_index, size_t count) {
  assert(part_index < parts_count());
  boundaries_.insert(boundaries_.begin() + part_index, count, boundaries_[part_index]);
}

template <std::forward_iterator Iterator>
inline void partitioning<Iterator>::remove_part(size_t part_index) {
  assert(part_index < parts_count());
  boundaries_.erase(boundaries_.begin() + part_index);
}

} // namespace positionless
