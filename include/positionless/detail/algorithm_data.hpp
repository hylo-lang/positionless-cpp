#pragma once

#include "positionless/detail/algorithm_data_fwd.hpp"
#include "positionless/detail/precondition.hpp"
#include "positionless/partitioning.hpp"

#include <vector>

#define POSITIONLESS_DEBUG_PRINT false

#if POSITIONLESS_DEBUG_PRINT
#include <iostream>
#endif

namespace positionless::detail {

/// The algorithm data shared between the iterators.
///
/// Invariants: the number of parts is one less than the number of iterators we have valid.
template <std::forward_iterator BaseIterator>
class algorithm_data {
public:
  /// The base iterator type.
  using base_iterator = BaseIterator;

  /// An instance of the algorithm data that covers the range [begin, end).
  ///
  /// Creates two iterators corresponding to `begin` and `end`, but never exposes them directly.
  explicit algorithm_data(base_iterator begin, base_iterator end);

  /// Creates a new iterator pointing to the begin of the range, and returns its index.
  size_t create_begin_iterator();

  /// Creates a new iterator pointing to the end of the range, and returns its index.
  size_t create_end_iterator();

  /// Returns the base iterator corresponding to `iterator_index`.
  ///
  /// - Precondition: iterator `iterator_index` is valid.
  base_iterator base(size_t iterator_index) const;

  /// Creates a copy of the base iterator at `iterator_index`, and return the index of the new
  /// iterator.
  ///
  /// - Precondition: iterator `iterator_index` is valid.
  size_t copy_iterator(size_t iterator_index);

  /// Marks the iterator at `iterator_index` as destroyed / not valid.
  ///
  /// - Precondition: iterator `iterator_index` is valid.
  void destroy_iterator(size_t iterator_index);

  /// Increments the iterator at `iterator_index` to point to the next element.
  ///
  /// - Precondition: iterator `iterator_index` is valid.
  /// - Precondition: `base(iterator_index)` != `base(end_index())`.
  void increment(size_t iterator_index);

  /// Increments the iterator at `iterator_index` by `n` elements.
  ///
  /// - Precondition: iterator `iterator_index` is valid.
  /// - Precondition: `base(iterator_index) + n` <= `base(end_index())`.
  void increment_by(size_t iterator_index, size_t n)
    requires std::random_access_iterator<BaseIterator>;

  /// Decrements the iterator at `iterator_index` to point to the next element.
  ///
  /// - Precondition: iterator `iterator_index` is valid.
  /// - Precondition: `base(iterator_index)` != `base(end_index())`.
  void decrement(size_t iterator_index);

  /// Decrements the iterator at `iterator_index` by `n` elements.
  ///
  /// - Precondition: iterator `iterator_index` is valid.
  /// - Precondition: `base(iterator_index) + n` <= `base(end_index())`.
  void decrement_by(size_t iterator_index, size_t n)
    requires std::random_access_iterator<BaseIterator>;

private:
  /// The partitioning applied to the input slice.
  partitioning<BaseIterator> partitioning_;

  /// Mapping from iterator index to part index.
  /// We reserve here a tombstone value to indicate destroyed iterators.
  /// This can contain values that are equal to `partitioning_.parts_count()` for iterators that
  /// point to the element past the end of the slice.
  std::vector<size_t> parts_mapping_;

#if POSITIONLESS_DEBUG_PRINT
  /// Prints the current state of the data for debugging purposes.
  void print_debug() const;
#endif
};

/// The value we use to indicate a destroyed iterator.
static constexpr size_t tombstone_part = static_cast<size_t>(-1);

template <std::forward_iterator BaseIterator>
inline algorithm_data<BaseIterator>::algorithm_data(BaseIterator begin, BaseIterator end)
    : partitioning_(begin, end), parts_mapping_{0, 1} {
#if POSITIONLESS_DEBUG_PRINT
  std::cout << "Initialized algorithm_data\n";
  print_debug();
#endif
}

template <std::forward_iterator BaseIterator>
inline size_t algorithm_data<BaseIterator>::create_begin_iterator() {
  size_t r = copy_iterator(0);
#if POSITIONLESS_DEBUG_PRINT
  std::cout << "created begin iterator: " << r << "\n";
  print_debug();
#endif
  return r;
}

template <std::forward_iterator BaseIterator>
inline size_t algorithm_data<BaseIterator>::create_end_iterator() {
  size_t r = copy_iterator(1);
#if POSITIONLESS_DEBUG_PRINT
  std::cout << "created end iterator: " << r << "\n";
  print_debug();
#endif
  return r;
}

template <std::forward_iterator BaseIterator>
inline typename algorithm_data<BaseIterator>::base_iterator
algorithm_data<BaseIterator>::base(size_t iterator_index) const {
  PRECONDITION(iterator_index < parts_mapping_.size());
  const size_t part = parts_mapping_[iterator_index];
  PRECONDITION(part != tombstone_part);
  if (part == partitioning_.parts_count()) {
    return partitioning_.part(partitioning_.parts_count() - 1).second;
  } else {
    return partitioning_.part(part).first;
  }
}

template <std::forward_iterator BaseIterator>
inline size_t algorithm_data<BaseIterator>::copy_iterator(size_t iterator_index) {
  PRECONDITION(iterator_index < parts_mapping_.size());
  const size_t part = parts_mapping_[iterator_index];
  PRECONDITION(part != tombstone_part);
  // Add an empty part before the part corresponding to `iterator_index`.
  partitioning_.add_part_begin(part);
  // Shift all the mappings that corresponds to parts >= `part` by 1.
  std::transform(
      parts_mapping_.cbegin(),
      parts_mapping_.cend(),
      parts_mapping_.begin(),
      [part](size_t p) { return p >= part && p != tombstone_part ? p + 1 : p; }
  );

  // Try to find a tombstone to reuse.
  size_t r;
  auto it = std::find(parts_mapping_.begin(), parts_mapping_.end(), tombstone_part);
  if (it != parts_mapping_.end()) {
    *it = part;
    r = static_cast<size_t>(std::distance(parts_mapping_.begin(), it));
  } else {
    // Add a new mapping for the new iterator.
    parts_mapping_.push_back(part);
    // Return the index of the new iterator.
    r = parts_mapping_.size() - 1;
  }
#if POSITIONLESS_DEBUG_PRINT
  std::cout << "copied iterator " << iterator_index << " to " << r << "\n";
  print_debug();
#endif
  return r;
}

template <std::forward_iterator BaseIterator>
inline void algorithm_data<BaseIterator>::destroy_iterator(size_t iterator_index) {
  PRECONDITION(iterator_index < parts_mapping_.size());
  parts_mapping_[iterator_index] = tombstone_part;
#if POSITIONLESS_DEBUG_PRINT
  std::cout << "destroyed iterator: " << iterator_index << "\n";
  print_debug();
#endif
}

template <std::forward_iterator BaseIterator>
inline void algorithm_data<BaseIterator>::increment(size_t iterator_index) {
  PRECONDITION(iterator_index < parts_mapping_.size());
  const size_t part = parts_mapping_[iterator_index];
  PRECONDITION(part != tombstone_part);
  PRECONDITION(partitioning_.part(part).first != base(1));

  if (!partitioning_.is_part_empty(part)) {
    // Simple case: we just grow the previous part, which will move the iterator.
    partitioning_.grow(part - 1);
  } else {
    // We need to shift some parts, to make space for growing.
    size_t next_non_empty_part = part + 1;
    while (next_non_empty_part < partitioning_.parts_count() &&
           partitioning_.is_part_empty(next_non_empty_part)) {
      next_non_empty_part++;
    }
    PRECONDITION(next_non_empty_part < partitioning_.parts_count());

    // Swap iterators that map to `part` and `next_non_empty_part-1`.
    auto it = std::find(parts_mapping_.begin(), parts_mapping_.end(), next_non_empty_part);
    PRECONDITION(it != parts_mapping_.end());
    std::swap(*it, parts_mapping_[iterator_index]);

    // Now grow the right part.
    partitioning_.grow(next_non_empty_part - 1);
  }

#if POSITIONLESS_DEBUG_PRINT
  std::cout << "incremented iterator: " << iterator_index << "\n";
  print_debug();
#endif
}

template <std::forward_iterator BaseIterator>
inline void algorithm_data<BaseIterator>::increment_by(size_t iterator_index, size_t n)
  requires std::random_access_iterator<BaseIterator>
{
  // TODO: improve this
  for (size_t i = 0; i < n; ++i) {
    increment(iterator_index);
  }
}

template <std::forward_iterator BaseIterator>
inline void algorithm_data<BaseIterator>::decrement(size_t iterator_index) {
  PRECONDITION(iterator_index < parts_mapping_.size());
  const size_t part = parts_mapping_[iterator_index];
  PRECONDITION(part != tombstone_part);
  PRECONDITION(part > 0);
  PRECONDITION(partitioning_.part(part).first != base(0));

  if (part > 0 && !partitioning_.is_part_empty(part - 1)) {
    // Simple case: we just shrink the previous part, which will move the iterator backward.
    partitioning_.shrink(part - 1);
  } else {
    // We need to find a previous non-empty part to shrink.
    size_t prev_non_empty_part = part - 1;
    while (prev_non_empty_part > 0 && partitioning_.is_part_empty(prev_non_empty_part)) {
      prev_non_empty_part--;
    }
    PRECONDITION(!partitioning_.is_part_empty(prev_non_empty_part));

    // Swap iterators that map to `part` and `prev_non_empty_part+1`.
    auto it = std::find(parts_mapping_.begin(), parts_mapping_.end(), prev_non_empty_part + 1);
    PRECONDITION(it != parts_mapping_.end());
    std::swap(*it, parts_mapping_[iterator_index]);

    // Now shrink the non-empty part.
    partitioning_.shrink(prev_non_empty_part);
  }

#if POSITIONLESS_DEBUG_PRINT
  std::cout << "decremented iterator: " << iterator_index << "\n";
  print_debug();
#endif
}

template <std::forward_iterator BaseIterator>
inline void algorithm_data<BaseIterator>::decrement_by(size_t iterator_index, size_t n)
  requires std::random_access_iterator<BaseIterator>
{
  // TODO: improve this
  for (size_t i = 0; i < n; ++i) {
    decrement(iterator_index);
  }
}

#if POSITIONLESS_DEBUG_PRINT
template <std::forward_iterator BaseIterator>
inline void algorithm_data<BaseIterator>::print_debug() const {
  std::cout << "  - data: ";
  for (size_t i = 0; i < partitioning_.parts_count(); ++i) {
    auto [begin, end] = partitioning_.part(i);
    std::cout << "[";
    for (auto it = begin; it != end; ++it) {
      std::cout << *it << " ";
    }
    std::cout << "]";
  }
  std::cout << "\n";
  std::cout << "  - mapping: [";
  for (size_t i = 0; i < parts_mapping_.size(); ++i) {
    if (parts_mapping_[i] == tombstone_part) {
      std::cout << "x ";
    } else {
      std::cout << parts_mapping_[i] << " ";
    }
  }
  std::cout << "]\n";
}
#endif

} // namespace positionless::detail
