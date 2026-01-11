#pragma once

#include "positionless/detail/algorithm_data.hpp"
#include "positionless/detail/precondition.hpp"
#include "positionless/partitioning.hpp"

#include <utility>
#include <vector>

namespace positionless {

/// An iterator that operates over a partitioned range, with safety checks.
template <std::forward_iterator BaseIterator>
class partitioning_iterator {
public:
  /// The difference type of the iterator.
  using difference_type = std::ptrdiff_t;
  /// The value type of the iterator.
  using value_type = typename std::iterator_traits<BaseIterator>::value_type;

  /// An instance coming directly from `algorithm_data`; used for begin/end iterators.
  explicit partitioning_iterator(
      detail::algorithm_data_ptr<BaseIterator> data, size_t iterator_index
  );

  /// Destroys `this`, notifying the shared data.
  ~partitioning_iterator();

  /// An instance that compares equal to `other`.
  partitioning_iterator(const partitioning_iterator& other);
  /// Assigns `other` to `this`, modifying the shared data accordingly.
  partitioning_iterator& operator=(const partitioning_iterator& other);

  partitioning_iterator(partitioning_iterator&& other) noexcept = default;
  partitioning_iterator& operator=(partitioning_iterator&& other) noexcept = default;

  /// Dereferences `this`.
  [[nodiscard]]
  value_type operator*() const;

  // Increments `this` to point to the next element.
  partitioning_iterator& operator++();
  // Increments `this` to point to the next element, returning a copy of the previous state.
  partitioning_iterator operator++(int);

  // Increments `this` to point to the previous element.
  partitioning_iterator& operator--()
    requires std::bidirectional_iterator<BaseIterator>;
  // Increments `this` to point to the previous element, returning a copy of the previous state.
  partitioning_iterator operator--(int)
    requires std::bidirectional_iterator<BaseIterator>;

  /// Returns `true` if `this` and `other` point to the same element.
  [[nodiscard]]
  bool operator==(const partitioning_iterator& other) const;
  /// Returns `true` if `this` and `other` point to different elements.
  [[nodiscard]]
  bool operator!=(const partitioning_iterator& other) const;

private:
  /// The shared algorithm data.
  detail::algorithm_data_ptr<BaseIterator> data_;

  /// The index of this iterator in shared data's `parts_mapping_`.
  size_t iterator_index_{0};
};

/// Creates a pair of partitioning iterators for the range [`begin`, `end`).
///
/// This allows us to express algorithms in positionless way, adding extra safety checks.
template <std::forward_iterator BaseIterator>
[[nodiscard]]
std::pair<partitioning_iterator<BaseIterator>, partitioning_iterator<BaseIterator>>
make_partitioning_iterators(BaseIterator begin, BaseIterator end);

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator>::partitioning_iterator(
    detail::algorithm_data_ptr<BaseIterator> data, size_t iterator_index
)
    : data_(std::move(data)), iterator_index_(iterator_index) {}

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator>::~partitioning_iterator() {
  if (data_) {
    data_->destroy_iterator(iterator_index_);
  }
}

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator>::partitioning_iterator(const partitioning_iterator& other
)
    : data_(other.data_), iterator_index_(data_->copy_iterator(other.iterator_index_)) {}

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator>&
partitioning_iterator<BaseIterator>::operator=(const partitioning_iterator& other) {
  if (this != &other) {
    if (data_) {
      data_->destroy_iterator(iterator_index_);
    }
    data_ = other.data_;
    iterator_index_ = data_->copy_iterator(other.iterator_index_);
  }
  return *this;
}

template <std::forward_iterator BaseIterator>
inline typename partitioning_iterator<BaseIterator>::value_type
partitioning_iterator<BaseIterator>::operator*() const {
  return *data_->base(iterator_index_);
}

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator>& partitioning_iterator<BaseIterator>::operator++() {
  data_->increment(iterator_index_);
  return *this;
}

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator> partitioning_iterator<BaseIterator>::operator++(int) {
  partitioning_iterator copy(*this);
  data_->increment(iterator_index_);
  return copy;
}

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator>& partitioning_iterator<BaseIterator>::operator--()
  requires std::bidirectional_iterator<BaseIterator>
{
  data_->decrement(iterator_index_);
  return *this;
}

template <std::forward_iterator BaseIterator>
inline partitioning_iterator<BaseIterator> partitioning_iterator<BaseIterator>::operator--(int)
  requires std::bidirectional_iterator<BaseIterator>
{
  partitioning_iterator copy(*this);
  data_->decrement(iterator_index_);
  return copy;
}

template <std::forward_iterator BaseIterator>
inline bool partitioning_iterator<BaseIterator>::operator==(const partitioning_iterator& other
) const {
  return data_ == other.data_ && data_->base(iterator_index_) == data_->base(other.iterator_index_);
}

template <std::forward_iterator BaseIterator>
inline bool partitioning_iterator<BaseIterator>::operator!=(const partitioning_iterator& other
) const {
  return !(*this == other);
}

template <std::forward_iterator BaseIterator>
[[nodiscard]]
inline std::pair<partitioning_iterator<BaseIterator>, partitioning_iterator<BaseIterator>>
make_partitioning_iterators(BaseIterator begin, BaseIterator end) {
  using iterator_t = partitioning_iterator<BaseIterator>;
  auto data = std::make_shared<detail::algorithm_data<BaseIterator>>(begin, end);
  return {
      iterator_t(data, data->create_begin_iterator()), iterator_t(data, data->create_end_iterator())
  };
}

} // namespace positionless
