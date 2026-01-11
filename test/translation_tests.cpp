#include "positionless/translation.hpp"

#include "detail/rapidcheck_wrapper.hpp"

#include <forward_list>
#include <vector>

using positionless::make_partitioning_iterators;
using positionless::partitioning_iterator;

TEST_PROPERTY(
    "partitioning_iterator allows accessing all the elements of the sequence",
    ([](std::vector<int> data) {
      auto i0 = data.begin();
      auto [it_begin, it_end] = make_partitioning_iterators(data.begin(), data.end());
      for (auto it = it_begin; it != it_end; ++it, ++i0) {
        RC_ASSERT(i0 != data.end());
        RC_ASSERT(*it == *i0);
      }
    })
);

TEST_PROPERTY(
    "partitioning_iterator allows accessing all the elements of a forward sequence",
    ([](std::forward_list<int> data) {
      auto i0 = data.begin();
      auto [it_begin, it_end] = make_partitioning_iterators(data.begin(), data.end());
      for (auto it = it_begin; it != it_end; ++it, ++i0) {
        RC_ASSERT(i0 != data.end());
        RC_ASSERT(*it == *i0);
      }
    })
);

TEST_PROPERTY(
    "partitioning_iterator's preincrement is in sync with postincrement",
    ([](std::vector<int> data) {
      auto [it_begin, it_end] = make_partitioning_iterators(data.begin(), data.end());
      auto it1 = it_begin;
      auto it2 = it_begin;
      while (it1 != it_end && it2 != it_end) {
        RC_ASSERT(*it1 == *it2);
        auto it2_post = it2++;
        RC_ASSERT(it1 == it2_post);
        RC_ASSERT(!(it1 == it2));
        ++it1;
        RC_ASSERT(it1 == it2);
      }
    })
);

TEST_PROPERTY(
    "partitioning_iterator allows accessing all the elements of a bidirectional sequence in "
    "reverse order",
    ([](std::vector<int> data) {
      RC_PRE(!data.empty());
      auto i0 = data.crbegin();
      auto [it_begin, it_end] = make_partitioning_iterators(data.begin(), data.end());
      auto it = it_end;
      it--;
      while (true) {
        RC_ASSERT(i0 != data.crend());
        RC_ASSERT(*it == *i0);
        if (it == it_begin) {
          break;
        }
        --it;
        ++i0;
      }
    })
);
