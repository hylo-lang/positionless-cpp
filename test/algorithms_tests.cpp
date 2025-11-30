#include "positionless/algorithms.hpp"

#include "detail/rapidcheck_wrapper.hpp"
#include "detail/vector_partitioning.hpp"

#include <vector>

using positionless::partitioning;
using positionless::swap_first;

TEST_PROPERTY(
    "`swap_first` swaps the first elements of two parts",
    [](vector_partitioning<int> vp) {
      RC_PRE(vp.partitioning_.parts_count() >= size_t{2});

      // Find all non-empty parts
      std::vector<size_t> non_empty_parts;
      for (size_t idx = 0; idx < vp.partitioning_.parts_count(); ++idx) {
        if (!vp.partitioning_.is_part_empty(idx)) {
          non_empty_parts.push_back(idx);
        }
      }
      RC_PRE(non_empty_parts.size() >= size_t{2});

      // Generate two distinct non-empty part indices
      const auto i_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
      const auto j_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
      const auto i = non_empty_parts[i_idx];
      const auto j = non_empty_parts[j_idx];

      // Get the original first elements
      const auto part_i = vp.partitioning_.part(i);
      const auto part_j = vp.partitioning_.part(j);
      const auto original_first_i = *part_i.first;
      const auto original_first_j = *part_j.first;

      // Perform the swap
      swap_first(vp.partitioning_, i, j);

      // Verify the swap occurred
      const auto after_part_i = vp.partitioning_.part(i);
      const auto after_part_j = vp.partitioning_.part(j);

      if (i == j) {
        // Swapping with itself should leave it unchanged
        RC_ASSERT(*after_part_i.first == original_first_i);
      } else {
        // First elements should be swapped
        RC_ASSERT(*after_part_i.first == original_first_j);
        RC_ASSERT(*after_part_j.first == original_first_i);
      }
    }
);

TEST_PROPERTY(
    "`swap_first` can be used multiple times without error",
    [](vector_partitioning<int> vp) {
      RC_PRE(vp.partitioning_.parts_count() >= size_t{2});

      // Find all non-empty parts
      std::vector<size_t> non_empty_parts;
      for (size_t idx = 0; idx < vp.partitioning_.parts_count(); ++idx) {
        if (!vp.partitioning_.is_part_empty(idx)) {
          non_empty_parts.push_back(idx);
        }
      }
      RC_PRE(non_empty_parts.size() >= size_t{2});

      // Generate multiple swap operations
      const auto num_swaps = *rc::gen::inRange(1, 20);

      for (int k = 0; k < num_swaps; ++k) {
        // Pick two random non-empty parts
        const auto i_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
        const auto j_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
        const auto i = non_empty_parts[i_idx];
        const auto j = non_empty_parts[j_idx];

        // This should not throw or violate any invariants
        swap_first(vp.partitioning_, i, j);
      }

      // Verify the partitioning is still valid
      RC_ASSERT(vp.partitioning_.parts_count() >= size_t{1});

      // Verify parts still cover the entire data
      size_t total_size = 0;
      for (size_t idx = 0; idx < vp.partitioning_.parts_count(); ++idx) {
        total_size += vp.partitioning_.part_size(idx);
      }
      RC_ASSERT(total_size == vp.data_.size());
    }
);

TEST_PROPERTY("`swap_first` on the same part should be a no-op", [](vector_partitioning<int> vp) {
  RC_PRE(vp.partitioning_.parts_count() >= size_t{1});

  // Find a non-empty part
  std::vector<size_t> non_empty_parts;
  for (size_t idx = 0; idx < vp.partitioning_.parts_count(); ++idx) {
    if (!vp.partitioning_.is_part_empty(idx)) {
      non_empty_parts.push_back(idx);
    }
  }
  RC_PRE(non_empty_parts.size() >= size_t{1});

  // Pick a random non-empty part
  const auto i_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
  const auto i = non_empty_parts[i_idx];

  // Save original data
  const auto original_data = vp.data_;

  // Swap part with itself
  swap_first(vp.partitioning_, i, i);

  // Verify data is unchanged
  RC_ASSERT(vp.data_ == original_data);
});

TEST_PROPERTY(
    "`swap_first` twice returns to original state (idempotent)",
    [](vector_partitioning<int> vp) {
      RC_PRE(vp.partitioning_.parts_count() >= size_t{2});

      // Find all non-empty parts
      std::vector<size_t> non_empty_parts;
      for (size_t idx = 0; idx < vp.partitioning_.parts_count(); ++idx) {
        if (!vp.partitioning_.is_part_empty(idx)) {
          non_empty_parts.push_back(idx);
        }
      }
      RC_PRE(non_empty_parts.size() >= size_t{2});

      // Generate two distinct non-empty part indices
      const auto i_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
      const auto j_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
      const auto i = non_empty_parts[i_idx];
      const auto j = non_empty_parts[j_idx];

      // Save original data
      const auto original_data = vp.data_;

      // Swap twice
      swap_first(vp.partitioning_, i, j);
      swap_first(vp.partitioning_, i, j);

      // Verify data is back to original state
      RC_ASSERT(vp.data_ == original_data);
    }
);

TEST_PROPERTY("`swap_first` preserves data as a permutation", [](vector_partitioning<int> vp) {
  RC_PRE(vp.partitioning_.parts_count() >= size_t{2});

  // Find all non-empty parts
  std::vector<size_t> non_empty_parts;
  for (size_t idx = 0; idx < vp.partitioning_.parts_count(); ++idx) {
    if (!vp.partitioning_.is_part_empty(idx)) {
      non_empty_parts.push_back(idx);
    }
  }
  RC_PRE(non_empty_parts.size() >= size_t{2});

  // Generate two distinct non-empty part indices
  const auto i_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
  const auto j_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
  const auto i = non_empty_parts[i_idx];
  const auto j = non_empty_parts[j_idx];

  // Save original data
  const auto original_data = vp.data_;

  // Perform the swap
  swap_first(vp.partitioning_, i, j);

  // Verify it's still a permutation of the original data
  RC_ASSERT(std::is_permutation(vp.data_.begin(), vp.data_.end(), original_data.begin()));
});

TEST_PROPERTY(
    "`swap_first` only modifies first elements of the two parts",
    [](vector_partitioning<int> vp) {
      RC_PRE(vp.partitioning_.parts_count() >= size_t{2});

      // Find all non-empty parts
      std::vector<size_t> non_empty_parts;
      for (size_t idx = 0; idx < vp.partitioning_.parts_count(); ++idx) {
        if (!vp.partitioning_.is_part_empty(idx)) {
          non_empty_parts.push_back(idx);
        }
      }
      RC_PRE(non_empty_parts.size() >= size_t{2});

      // Generate two part indices
      const auto i_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
      const auto j_idx = *rc::gen::inRange<size_t>(0, non_empty_parts.size());
      const auto i = non_empty_parts[i_idx];
      const auto j = non_empty_parts[j_idx];

      // Save all elements except the first from each part (if they have more than 1 element)
      const auto part_i = vp.partitioning_.part(i);
      const auto part_j = vp.partitioning_.part(j);
      std::vector<int> rest_of_i(part_i.first + 1, part_i.second);
      std::vector<int> rest_of_j(part_j.first + 1, part_j.second);

      // Perform the swap
      swap_first(vp.partitioning_, i, j);

      // Verify non-first elements remain unchanged
      const auto after_part_i = vp.partitioning_.part(i);
      const auto after_part_j = vp.partitioning_.part(j);

      std::vector<int> new_rest_of_i(after_part_i.first + 1, after_part_i.second);
      std::vector<int> new_rest_of_j(after_part_j.first + 1, after_part_j.second);

      RC_ASSERT(new_rest_of_i == rest_of_i);
      RC_ASSERT(new_rest_of_j == rest_of_j);
    }
);
