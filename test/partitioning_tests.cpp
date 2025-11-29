#include "positionless/partitioning.hpp"

#include "detail/rapidcheck_wrapper.hpp"
#include "detail/vector_partitioning.hpp"

#include <vector>

using positionless::partitioning;

TEST_PROPERTY("parts of a partitioning cover the entire data", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  size_t sum = 0;
  for (size_t i = 0; i < k; ++i) {
    sum += vp.part_size(i);
  }
  RC_ASSERT(sum == vp.data_.size());
  return true;
})

TEST_PROPERTY("partitioning allows accessing all the elements", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  std::vector<int> reconstructed;
  for (size_t i = 0; i < k; ++i) {
    const auto part = vp.partitioning_.part(i);
    reconstructed.insert(reconstructed.end(), part.first, part.second);
  }
  RC_ASSERT(reconstructed == vp.data_);
  return true;
})

TEST_PROPERTY("partitioning part count matches vector_partitioning", [](vector_partitioning<int> vp) {
  RC_ASSERT(vp.partitioning_.parts_count() >= 1);
  return true;
})

TEST_PROPERTY("`is_part_empty` returns true for empty parts, and false otherwise", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  for (size_t i = 0; i < k; ++i) {
    const bool empty = vp.partitioning_.is_part_empty(i);
    const size_t size = vp.part_size(i);
    RC_ASSERT(empty == (size == 0));
  }
  return true;
})

TEST_PROPERTY("`grow` increases the size of a part by 1", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  RC_PRE(k >= 2);
  
  // Find a valid index where next part is non-empty
  size_t idx = 0;
  for (size_t i = 0; i + 1 < k; ++i) {
    if (!vp.partitioning_.is_part_empty(i + 1)) {
      idx = i;
      break;
    }
  }
  RC_PRE(!vp.partitioning_.is_part_empty(idx + 1));
  
  const size_t before = vp.part_size(idx);
  const size_t next_before = vp.part_size(idx + 1);
  vp.partitioning_.grow(idx);
  const size_t after = vp.part_size(idx);
  const size_t next_after = vp.part_size(idx + 1);
  
  RC_ASSERT(after == before + 1);
  RC_ASSERT(next_after == next_before - 1);
  return true;
})
  
TEST_PROPERTY("`grow_by` increases the size of a part by `n`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  RC_PRE(k >= 2);
  
  // Find a valid index where next part is non-empty
  size_t idx = 0;
  size_t max_grow = 0;
  for (size_t i = 0; i + 1 < k; ++i) {
    const size_t next_size = vp.part_size(i + 1);
    if (next_size > max_grow) {
      idx = i;
      max_grow = next_size;
    }
  }
  RC_PRE(max_grow > 0);
  
  const size_t n = *rc::gen::inRange<size_t>(1, max_grow + 1);
  const size_t before = vp.part_size(idx);
  const size_t next_before = vp.part_size(idx + 1);
  
  vp.partitioning_.grow_by(idx, n);
  
  const size_t after = vp.part_size(idx);
  const size_t next_after = vp.part_size(idx + 1);
  
  RC_ASSERT(after == before + n);
  RC_ASSERT(next_after == next_before - n);
  return true;
})

TEST_PROPERTY("`add_part_end` adds a new empty part at the end `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  
  const size_t size_before = vp.part_size(idx);
  vp.partitioning_.add_part_end(idx);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + 1);
  RC_ASSERT(vp.part_size(idx) == size_before);
  RC_ASSERT(vp.partitioning_.is_part_empty(idx + 1));
  return true;
})

TEST_PROPERTY("`add_part_begin` adds a new empty part at the begin `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  
  const size_t size_before = vp.part_size(idx);
  vp.partitioning_.add_part_begin(idx);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + 1);
  RC_ASSERT(vp.partitioning_.is_part_empty(idx));
  RC_ASSERT(vp.part_size(idx + 1) == size_before);
  return true;
})

TEST_PROPERTY("`add_parts_end` adds `n` empty parts at the end `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  const size_t n = *rc::gen::inRange<size_t>(1, 6);
  
  const size_t size_before = vp.part_size(idx);
  vp.partitioning_.add_parts_end(idx, n);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + n);
  RC_ASSERT(vp.part_size(idx) == size_before);
  for (size_t i = 1; i <= n; ++i) {
    RC_ASSERT(vp.partitioning_.is_part_empty(idx + i));
  }
  return true;
})

TEST_PROPERTY("`add_parts_begin` adds `n` empty parts at the begin `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  const size_t n = *rc::gen::inRange<size_t>(1, 6);
  
  const size_t size_before = vp.part_size(idx);
  vp.partitioning_.add_parts_begin(idx, n);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + n);
  for (size_t i = 0; i < n; ++i) {
    RC_ASSERT(vp.partitioning_.is_part_empty(idx + i));
  }
  RC_ASSERT(vp.part_size(idx + n) == size_before);
  return true;
})

TEST_PROPERTY("`remove_part` decreases the number of parts by 1", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  RC_PRE(k >= 2);
  
  const size_t idx = *rc::gen::inRange<size_t>(1, k);
  vp.partitioning_.remove_part(idx);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k - 1);
  return true;
})

TEST_PROPERTY("`remove_part` transfer all the elements of `part_index` to part `part_index-1`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  RC_PRE(k >= 2);
  
  const size_t idx = *rc::gen::inRange<size_t>(1, k);
  const size_t expected_size = vp.part_size(idx - 1) + vp.part_size(idx);
  
  vp.partitioning_.remove_part(idx);
  
  RC_ASSERT(vp.part_size(idx - 1) == expected_size);
  return true;
})

TEST_PROPERTY("`add_parts_end` is equivalent to calling `add_part_end` `n` times", [](vector_partitioning<int> vp) {
  auto copy1 = vp;
  auto copy2 = vp;
  const auto k = copy1.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  const size_t n = *rc::gen::inRange<size_t>(1, 6);
  
  // Use add_parts_end
  copy1.partitioning_.add_parts_end(idx, n);
  
  // Use add_part_end n times
  for (size_t i = 0; i < n; ++i) {
    copy2.partitioning_.add_part_end(idx);
  }
  
  RC_ASSERT(copy1.partitioning_.parts_count() == copy2.partitioning_.parts_count());
  for (size_t i = 0; i < copy1.partitioning_.parts_count(); ++i) {
    RC_ASSERT(copy1.part_size(i) == copy2.part_size(i));
  }
  return true;
})

TEST_PROPERTY("`add_parts_begin` is equivalent to calling `add_part_begin` `n` times", [](vector_partitioning<int> vp) {
  auto copy1 = vp;
  auto copy2 = vp;
  const auto k = copy1.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  const size_t n = *rc::gen::inRange<size_t>(1, 6);
  
  // Use add_parts_begin
  copy1.partitioning_.add_parts_begin(idx, n);
  
  // Use add_part_begin n times
  for (size_t i = 0; i < n; ++i) {
    copy2.partitioning_.add_part_begin(idx);
  }
  
  RC_ASSERT(copy1.partitioning_.parts_count() == copy2.partitioning_.parts_count());
  for (size_t i = 0; i < copy1.partitioning_.parts_count(); ++i) {
    RC_ASSERT(copy1.part_size(i) == copy2.part_size(i));
  }
  return true;
})


