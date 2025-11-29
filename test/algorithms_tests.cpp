#include "positionless/algorithms.hpp"

#include <doctest/doctest.h>

#include <vector>

using positionless::partitioning;
using positionless::swap_first;

SCENARIO("swap_first - basic functionality") {
  GIVEN("a partitioning with two parts") {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    partitioning p(data.begin(), data.end());

    // Create two parts: [1, 2, 3] and [4, 5, 6]
    p.add_part_begin(0);
    p.grow_by(0, 3);

    WHEN("swapping the first elements of both parts") {
      swap_first(p, 0, 1);

      THEN("the first element of part 0 is swapped with part 1") {
        auto [b0, e0] = p.part(0);
        auto [b1, e1] = p.part(1);

        CHECK(*b0 == 4);
        CHECK(*b1 == 1);
      }

      THEN("the data vector is modified") { CHECK(data == std::vector<int>{4, 2, 3, 1, 5, 6}); }

      THEN("other elements remain unchanged") {
        auto [b0, e0] = p.part(0);
        auto [b1, e1] = p.part(1);

        CHECK(*(b0 + 1) == 2);
        CHECK(*(b0 + 2) == 3);
        CHECK(*(b1 + 1) == 5);
        CHECK(*(b1 + 2) == 6);
      }
    }
  }
}

SCENARIO("swap_first - same part") {
  GIVEN("a partitioning with a single part") {
    std::vector<int> data = {10, 20, 30};
    partitioning p(data.begin(), data.end());

    WHEN("swapping the first element with itself") {
      swap_first(p, 0, 0);

      THEN("nothing changes") {
        auto [b, e] = p.part(0);
        CHECK(*b == 10);
        CHECK(data == std::vector<int>{10, 20, 30});
      }
    }
  }
}

SCENARIO("swap_first - multiple parts") {
  GIVEN("a partitioning with three parts") {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    partitioning p(data.begin(), data.end());

    // Create three parts: [1, 2, 3] [4, 5, 6] [7, 8, 9]
    p.add_parts_begin(0, 2);
    // Now: [empty] [empty] [1, 2, 3, 4, 5, 6, 7, 8, 9]

    // Grow part 1 to get 6 elements from part 2
    p.grow_by(1, 6);
    // Now: [empty] [1, 2, 3, 4, 5, 6] [7, 8, 9]

    // Grow part 0 to get 3 elements from part 1
    p.grow_by(0, 3);
    // Now: [1, 2, 3] [4, 5, 6] [7, 8, 9]

    WHEN("swapping first elements of parts 0 and 2") {
      swap_first(p, 0, 2);

      THEN("part 0 and part 2 first elements are swapped") {
        auto [b0, e0] = p.part(0);
        auto [b2, e2] = p.part(2);

        CHECK(*b0 == 7);
        CHECK(*b2 == 1);
      }

      THEN("part 1 is unchanged") {
        auto [b1, e1] = p.part(1);
        CHECK(*b1 == 4);
        CHECK(*(b1 + 1) == 5);
        CHECK(*(b1 + 2) == 6);
      }

      THEN("the data vector reflects the swap") {
        CHECK(data == std::vector<int>{7, 2, 3, 4, 5, 6, 1, 8, 9});
      }
    }

    WHEN("swapping first elements of parts 1 and 2") {
      swap_first(p, 1, 2);

      THEN("part 1 and part 2 first elements are swapped") {
        auto [b1, e1] = p.part(1);
        auto [b2, e2] = p.part(2);

        CHECK(*b1 == 7);
        CHECK(*b2 == 4);
      }
    }
  }
}

SCENARIO("swap_first - single element parts") {
  GIVEN("a partitioning where each part has exactly one element") {
    std::vector<int> data = {10, 20, 30, 40, 50};
    partitioning p(data.begin(), data.end());

    // Create 5 parts, each with 1 element
    p.add_parts_begin(0, 4);
    // Now: [empty] [empty] [empty] [empty] [10, 20, 30, 40, 50]

    // Build from right to left - each part needs to "accumulate" elements
    // for all parts to its left plus its own element
    p.grow_by(3, 4); // Part 3 needs to get 4 elements total (for parts 0,1,2,3)
    p.grow_by(2, 3); // Part 2 needs to get 3 elements total (for parts 0,1,2)
    p.grow_by(1, 2); // Part 1 needs to get 2 elements total (for parts 0,1)
    p.grow_by(0, 1); // Part 0 needs to get 1 element (for itself)
    // Now: [10] [20] [30] [40] [50]

    WHEN("swapping first (and only) elements") {
      swap_first(p, 1, 3);

      THEN("the single elements are swapped") {
        auto [b1, e1] = p.part(1);
        auto [b3, e3] = p.part(3);

        CHECK(*b1 == 40);
        CHECK(*b3 == 20);
        CHECK(std::distance(b1, e1) == 1);
        CHECK(std::distance(b3, e3) == 1);
      }

      THEN("the data reflects the swap") { CHECK(data == std::vector<int>{10, 40, 30, 20, 50}); }
    }
  }
}

SCENARIO("swap_first - different data types") {
  GIVEN("a partitioning with strings") {
    std::vector<std::string> data = {"apple", "banana", "cherry", "date", "elderberry"};
    partitioning p(data.begin(), data.end());

    // Create two parts: ["apple", "banana"] ["cherry", "date", "elderberry"]
    p.add_part_begin(0);
    p.grow_by(0, 2);

    WHEN("swapping first elements") {
      swap_first(p, 0, 1);

      THEN("strings are swapped") {
        auto [b0, e0] = p.part(0);
        auto [b1, e1] = p.part(1);

        CHECK(*b0 == "cherry");
        CHECK(*b1 == "apple");
      }

      THEN("the data vector is correctly modified") {
        CHECK(data[0] == "cherry");
        CHECK(data[1] == "banana");
        CHECK(data[2] == "apple");
        CHECK(data[3] == "date");
        CHECK(data[4] == "elderberry");
      }
    }
  }
}

SCENARIO("swap_first - sequential swaps") {
  GIVEN("a partitioning with three parts") {
    std::vector<int> data = {100, 200, 300, 400, 500, 600};
    partitioning p(data.begin(), data.end());

    // Create three parts: [100, 200] [300, 400] [500, 600]
    p.add_parts_begin(0, 2);
    // Build from right to left
    p.grow_by(1, 4); // Part 1 needs 4 elements (2 for itself + 2 for part 0)
    p.grow_by(0, 2); // Part 0 takes 2 elements

    WHEN("performing multiple swaps in sequence") {
      swap_first(p, 0, 1);
      swap_first(p, 1, 2);
      swap_first(p, 0, 2);

      THEN("all swaps are applied correctly") {
        auto [b0, e0] = p.part(0);
        auto [b1, e1] = p.part(1);
        auto [b2, e2] = p.part(2);

        CHECK(*b0 == 100);
        CHECK(*b1 == 500);
        CHECK(*b2 == 300);
      }

      THEN("final data state is correct") {
        CHECK(data == std::vector<int>{100, 200, 500, 400, 300, 600});
      }
    }
  }
}
