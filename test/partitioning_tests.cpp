#include "positionless/partitioning.hpp"

#include "detail/rapidcheck_wrapper.hpp"
#include "detail/vector_partitioning.hpp"
#include "detail/forward_list_partitioning.hpp"

#include <vector>

using positionless::partitioning;

TEST_PROPERTY("parts of a partitioning cover the entire data", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  size_t sum = 0;
  for (size_t i = 0; i < k; ++i) {
    sum += vp.partitioning_.part_size(i);
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
  RC_ASSERT(vp.partitioning_.parts_count() >= size_t(1));
  return true;
})

TEST_PROPERTY("`is_part_empty` returns true for empty parts, and false otherwise", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  for (size_t i = 0; i < k; ++i) {
    const bool empty = vp.partitioning_.is_part_empty(i);
    const size_t size = vp.partitioning_.part_size(i);
    RC_ASSERT(empty == (size == 0));
  }
  return true;
})

TEST_PROPERTY("`grow` increases the size of a part by 1", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  RC_PRE(k >= size_t(2));
  
  // Find a valid index where next part is non-empty
  size_t idx = 0;
  for (size_t i = 0; i + 1 < k; ++i) {
    if (!vp.partitioning_.is_part_empty(i + 1)) {
      idx = i;
      break;
    }
  }
  RC_PRE(!vp.partitioning_.is_part_empty(idx + 1));
  
  const size_t before = vp.partitioning_.part_size(idx);
  const size_t next_before = vp.partitioning_.part_size(idx + 1);
  vp.partitioning_.grow(idx);
  const size_t after = vp.partitioning_.part_size(idx);
  const size_t next_after = vp.partitioning_.part_size(idx + 1);
  
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
    const size_t next_size = vp.partitioning_.part_size(i + 1);
    if (next_size > max_grow) {
      idx = i;
      max_grow = next_size;
    }
  }
  RC_PRE(max_grow > size_t(0));
  
  const size_t n = *rc::gen::inRange<size_t>(1, max_grow + 1);
  const size_t before = vp.partitioning_.part_size(idx);
  const size_t next_before = vp.partitioning_.part_size(idx + 1);
  
  vp.partitioning_.grow_by(idx, n);
  
  const size_t after = vp.partitioning_.part_size(idx);
  const size_t next_after = vp.partitioning_.part_size(idx + 1);
  
  RC_ASSERT(after == before + n);
  RC_ASSERT(next_after == next_before - n);
  return true;
})

TEST_PROPERTY("`add_part_end` adds a new empty part at the end `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  
  const size_t size_before = vp.partitioning_.part_size(idx);
  vp.partitioning_.add_part_end(idx);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + 1);
  RC_ASSERT(vp.partitioning_.part_size(idx) == size_before);
  RC_ASSERT(vp.partitioning_.is_part_empty(idx + 1));
  return true;
})

TEST_PROPERTY("`add_part_begin` adds a new empty part at the begin `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  
  const size_t size_before = vp.partitioning_.part_size(idx);
  vp.partitioning_.add_part_begin(idx);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + 1);
  RC_ASSERT(vp.partitioning_.is_part_empty(idx));
  RC_ASSERT(vp.partitioning_.part_size(idx + 1) == size_before);
  return true;
})

TEST_PROPERTY("`add_parts_end` adds `n` empty parts at the end `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  const size_t n = *rc::gen::inRange<size_t>(1, 6);
  
  const size_t size_before = vp.partitioning_.part_size(idx);
  vp.partitioning_.add_parts_end(idx, n);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + n);
  RC_ASSERT(vp.partitioning_.part_size(idx) == size_before);
  for (size_t i = 1; i <= n; ++i) {
    RC_ASSERT(vp.partitioning_.is_part_empty(idx + i));
  }
  return true;
})

TEST_PROPERTY("`add_parts_begin` adds `n` empty parts at the begin `part_index`", [](vector_partitioning<int> vp) {
  const auto k = vp.partitioning_.parts_count();
  const size_t idx = *rc::gen::inRange<size_t>(0, k);
  const size_t n = *rc::gen::inRange<size_t>(1, 6);
  
  const size_t size_before = vp.partitioning_.part_size(idx);
  vp.partitioning_.add_parts_begin(idx, n);
  
  RC_ASSERT(vp.partitioning_.parts_count() == k + n);
  for (size_t i = 0; i < n; ++i) {
    RC_ASSERT(vp.partitioning_.is_part_empty(idx + i));
  }
  RC_ASSERT(vp.partitioning_.part_size(idx + n) == size_before);
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
  const size_t expected_size = vp.partitioning_.part_size(idx - 1) + vp.partitioning_.part_size(idx);
  
  vp.partitioning_.remove_part(idx);
  
  RC_ASSERT(vp.partitioning_.part_size(idx - 1) == expected_size);
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
    RC_ASSERT(copy1.partitioning_.part_size(i) == copy2.partitioning_.part_size(i));
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
    RC_ASSERT(copy1.partitioning_.part_size(i) == copy2.partitioning_.part_size(i));
  }
  return true;
})

TEST_PROPERTY("forward_list partitioning covers entire data", [](forward_list_partitioning<int> fp) {
  const auto k = fp.partitioning_.parts_count();
  size_t sum = 0;
  for (size_t i = 0; i < k; ++i) sum += std::distance(fp.partitioning_.part(i).first, fp.partitioning_.part(i).second);
  RC_ASSERT(static_cast<std::ptrdiff_t>(sum) == std::distance(fp.data_.begin(), fp.data_.end()));
  return true;
})

TEST_PROPERTY("forward_list basic ops: grow/grow_by/add/remove", [](forward_list_partitioning<int> fp) {
  const auto k0 = fp.partitioning_.parts_count();
  RC_PRE(k0 >= 1);

  // add_part_begin at a random index
  const size_t idx_add = *rc::gen::inRange<size_t>(0, k0);
  fp.partitioning_.add_part_begin(idx_add);
  RC_ASSERT(fp.partitioning_.is_part_empty(idx_add));

  // ensure we have at least two parts to operate grow/grow_by
  const auto k1 = fp.partitioning_.parts_count();
  RC_PRE(k1 >= 2);

  // pick an index with non-empty next part for grow
  size_t idx_grow = 0;
  bool found = false;
  for (size_t i = 0; i + 1 < k1; ++i) {
    auto next = fp.partitioning_.part(i + 1);
    if (next.first != next.second) { idx_grow = i; found = true; break; }
  }
  RC_PRE(found);

  // measure sizes
  auto cur = fp.partitioning_.part(idx_grow);
  auto nxt = fp.partitioning_.part(idx_grow + 1);
  const size_t cur_before = std::distance(cur.first, cur.second);
  const size_t nxt_before = std::distance(nxt.first, nxt.second);

  fp.partitioning_.grow(idx_grow);

  cur = fp.partitioning_.part(idx_grow);
  nxt = fp.partitioning_.part(idx_grow + 1);
  RC_ASSERT(std::distance(cur.first, cur.second) == cur_before + 1);
  RC_ASSERT(std::distance(nxt.first, nxt.second) == nxt_before - 1);

  // grow_by on possibly different index
  size_t idx_grow_by = idx_grow;
  size_t max_grow = 0;
  for (size_t i = 0; i + 1 < k1; ++i) {
    auto next2 = fp.partitioning_.part(i + 1);
    const size_t ns = std::distance(next2.first, next2.second);
    if (ns > max_grow) { max_grow = ns; idx_grow_by = i; }
  }
  RC_PRE(max_grow > size_t(0));
  const size_t n = *rc::gen::inRange<size_t>(1, max_grow + 1);
  auto cur2 = fp.partitioning_.part(idx_grow_by);
  auto nxt2 = fp.partitioning_.part(idx_grow_by + 1);
  const size_t cur2_before = std::distance(cur2.first, cur2.second);
  const size_t nxt2_before = std::distance(nxt2.first, nxt2.second);
  fp.partitioning_.grow_by(idx_grow_by, n);
  cur2 = fp.partitioning_.part(idx_grow_by);
  nxt2 = fp.partitioning_.part(idx_grow_by + 1);
  RC_ASSERT(std::distance(cur2.first, cur2.second) == cur2_before + n);
  RC_ASSERT(std::distance(nxt2.first, nxt2.second) == nxt2_before - n);

  // remove_part at valid index (>0)
  const auto k2 = fp.partitioning_.parts_count();
  RC_PRE(k2 >= 2);
  const size_t idx_rem = *rc::gen::inRange<size_t>(1, k2);
  auto left = fp.partitioning_.part(idx_rem - 1);
  auto rem = fp.partitioning_.part(idx_rem);
  const size_t expected = std::distance(left.first, left.second) + std::distance(rem.first, rem.second);
  fp.partitioning_.remove_part(idx_rem);
  auto after_left = fp.partitioning_.part(idx_rem - 1);
  RC_ASSERT(std::distance(after_left.first, after_left.second) == expected);
  RC_ASSERT(fp.partitioning_.parts_count() == k2 - 1);
  return true;
})


