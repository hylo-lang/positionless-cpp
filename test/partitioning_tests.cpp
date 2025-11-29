#include "positionless/partitioning.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <vector>

using positionless::partitioning;

SCENARIO("partitioning - constructor with range") {
  GIVEN("a vector with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    WHEN("constructing a partitioning from the range") {
      partitioning p(data.begin(), data.end());
      
      THEN("it creates one part initially") {
        CHECK(p.parts_count() == 1);
      }
      
      THEN("the first part covers the entire range") {
        auto [begin, end] = p.part(0);
        CHECK(begin == data.begin());
        CHECK(end == data.end());
        CHECK(std::distance(begin, end) == 5);
      }
      
      THEN("the first part is not empty") {
        CHECK_FALSE(p.is_part_empty(0));
      }
    }
  }
}

SCENARIO("partitioning - parts_count") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    THEN("it initially has one part") {
      CHECK(p.parts_count() == 1);
    }
    
    WHEN("adding a part at index 0") {
      p.add_part_end(0);
      
      THEN("it has two parts") {
        CHECK(p.parts_count() == 2);
      }
      
      AND_WHEN("adding another part at index 1") {
        p.add_part_end(1);
        
        THEN("it has three parts") {
          CHECK(p.parts_count() == 3);
        }
      }
    }
  }
}

SCENARIO("partitioning - part") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    WHEN("getting the part at index 0") {
      auto [begin, end] = p.part(0);
      
      THEN("it returns the correct iterators") {
        CHECK(begin == data.begin());
        CHECK(end == data.end());
      }
    }
    
    WHEN("adding a part and getting both parts") {
      p.add_part_end(0);
      auto [begin0, end0] = p.part(0);
      auto [begin1, end1] = p.part(1);
      
      THEN("part 0 still covers the full range") {
        CHECK(begin0 == data.begin());
        CHECK(end0 == data.end());
      }
      
      THEN("part 1 is empty at the end") {
        CHECK(begin1 == data.end());
        CHECK(end1 == data.end());
      }
    }
  }
}

SCENARIO("partitioning - is_part_empty") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    THEN("the initial part is not empty") {
      CHECK_FALSE(p.is_part_empty(0));
    }
    
    WHEN("adding a new empty part") {
      p.add_part_end(0);
      
      THEN("the original part is still not empty") {
        CHECK_FALSE(p.is_part_empty(0));
      }
      
      THEN("the newly added part is empty") {
        CHECK(p.is_part_empty(1));
      }
    }
  }
}

SCENARIO("partitioning - grow") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    WHEN("adding an empty part at the end") {
      p.add_part_end(0);
      
      THEN("the partitioning has two parts") {
        CHECK(p.parts_count() == 2);
      }
      
      THEN("the new part is empty") {
        CHECK(p.is_part_empty(1));
      }
      
      THEN("parts are contiguous") {
        auto [begin0, end0] = p.part(0);
        auto [begin1, end1] = p.part(1);
        CHECK(end0 == begin1);
      }
    }
  }
}

SCENARIO("partitioning - add_part_end") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    WHEN("adding a part at index 0") {
      p.add_part_end(0);
      
      THEN("the partitioning has two parts") {
        CHECK(p.parts_count() == 2);
      }
      
      THEN("the new part is empty") {
        CHECK(p.is_part_empty(1));
      }
    }
    
    WHEN("adding multiple parts sequentially at index 0") {
      p.add_part_end(0);
      p.add_part_end(0);
      p.add_part_end(0);
      
      THEN("the partitioning has four parts") {
        CHECK(p.parts_count() == 4);
      }
      
      THEN("all new parts are empty") {
        CHECK(p.is_part_empty(1));
        CHECK(p.is_part_empty(2));
        CHECK(p.is_part_empty(3));
      }
    }
    
    WHEN("adding parts at different indices") {
      p.add_part_end(0);
      p.add_part_end(1);
      
      THEN("the partitioning has three parts") {
        CHECK(p.parts_count() == 3);
      }
    }
  }
}

SCENARIO("partitioning - add_parts_end") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    WHEN("adding 3 parts at once") {
      p.add_parts_end(0, 3);
      
      THEN("the partitioning has four parts") {
        CHECK(p.parts_count() == 4);
      }
      
      THEN("all new parts are empty") {
        CHECK(p.is_part_empty(1));
        CHECK(p.is_part_empty(2));
        CHECK(p.is_part_empty(3));
      }
    }
    
    WHEN("adding 0 parts") {
      p.add_parts_end(0, 0);
      
      THEN("the part count remains unchanged") {
        CHECK(p.parts_count() == 1);
      }
    }
  }
  
  GIVEN("two partitionings with the same data") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p1(data.begin(), data.end());
    partitioning p2(data.begin(), data.end());
    
    WHEN("one adds 1 part and the other adds 1 part using add_parts") {
      p1.add_part_end(0);
      p2.add_parts_end(0, 1);
      
      THEN("both have the same part count") {
        CHECK(p1.parts_count() == p2.parts_count());
      }
    }
  }
}

SCENARIO("partitioning - remove_part") {
  GIVEN("a partitioning with three parts") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    p.add_part_end(0);
    p.add_part_end(0);
    
    THEN("it has three parts") {
      CHECK(p.parts_count() == 3);
    }
    
    WHEN("removing part 1") {
      p.remove_part(1);
      
      THEN("it has two parts") {
        CHECK(p.parts_count() == 2);
      }
    }
  }
  
  GIVEN("a partitioning with three parts created at once") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    p.add_parts_end(0, 2);
    
    THEN("it has three parts") {
      CHECK(p.parts_count() == 3);
    }
    
    WHEN("removing the middle part") {
      p.remove_part(1);
      
      THEN("it has two parts") {
        CHECK(p.parts_count() == 2);
      }
      
      THEN("part 0 extends to the original end") {
        auto [begin0, end0] = p.part(0);
        CHECK(begin0 == data.begin());
        CHECK(end0 == data.end());
      }
    }
  }
}

SCENARIO("partitioning - edge cases") {
  GIVEN("a vector with a single element") {
    std::vector<int> data = {42};
    
    WHEN("creating a partitioning from it") {
      partitioning p(data.begin(), data.end());
      
      THEN("it has one part") {
        CHECK(p.parts_count() == 1);
      }
      
      THEN("the part is not empty") {
        CHECK_FALSE(p.is_part_empty(0));
      }
      
      THEN("the part contains the single element") {
        auto [begin, end] = p.part(0);
        CHECK(std::distance(begin, end) == 1);
        CHECK(*begin == 42);
      }
    }
  }
  
  GIVEN("an empty vector") {
    std::vector<int> data;
    
    WHEN("creating a partitioning from it") {
      partitioning p(data.begin(), data.end());
      
      THEN("it has one part") {
        CHECK(p.parts_count() == 1);
      }
      
      THEN("the part is empty") {
        CHECK(p.is_part_empty(0));
      }
    }
  }
  
  GIVEN("a partitioning with multiple operations") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    WHEN("adding 3 parts and then removing 2") {
      p.add_parts_end(0, 3);
      
      THEN("it has four parts initially") {
        CHECK(p.parts_count() == 4);
      }
      
      AND_WHEN("removing part 2") {
        p.remove_part(2);
        
        THEN("it has three parts") {
          CHECK(p.parts_count() == 3);
        }
        
        AND_WHEN("removing part 1") {
          p.remove_part(1);
          
          THEN("it has two parts") {
            CHECK(p.parts_count() == 2);
          }
        }
      }
    }
  }
}

SCENARIO("partitioning - add_part_begin") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    WHEN("adding a part at the beginning of part 0") {
      p.add_part_begin(0);
      
      THEN("the partitioning has two parts") {
        CHECK(p.parts_count() == 2);
      }
      
      THEN("the new part 0 is empty") {
        CHECK(p.is_part_empty(0));
      }
      
      THEN("the new part 1 contains all elements") {
        auto [begin1, end1] = p.part(1);
        CHECK(std::distance(begin1, end1) == 5);
      }
    }
    
    WHEN("adding multiple parts at the beginning sequentially") {
      p.add_part_begin(0);
      p.add_part_begin(0);
      p.add_part_begin(0);
      
      THEN("the partitioning has four parts") {
        CHECK(p.parts_count() == 4);
      }
      
      THEN("the first three parts are empty") {
        CHECK(p.is_part_empty(0));
        CHECK(p.is_part_empty(1));
        CHECK(p.is_part_empty(2));
      }
      
      THEN("the last part contains all elements") {
        auto [begin3, end3] = p.part(3);
        CHECK(std::distance(begin3, end3) == 5);
      }
    }
  }
}

SCENARIO("partitioning - add_parts_begin") {
  GIVEN("a partitioning with 5 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p(data.begin(), data.end());
    
    WHEN("adding 3 parts at the beginning at once") {
      p.add_parts_begin(0, 3);
      
      THEN("the partitioning has four parts") {
        CHECK(p.parts_count() == 4);
      }
      
      THEN("the first three parts are empty") {
        CHECK(p.is_part_empty(0));
        CHECK(p.is_part_empty(1));
        CHECK(p.is_part_empty(2));
      }
      
      THEN("the last part contains all elements") {
        auto [begin3, end3] = p.part(3);
        CHECK(std::distance(begin3, end3) == 5);
      }
    }
    
    WHEN("adding 0 parts") {
      p.add_parts_begin(0, 0);
      
      THEN("the part count remains unchanged") {
        CHECK(p.parts_count() == 1);
      }
    }
  }
  
  GIVEN("two partitionings with the same data") {
    std::vector<int> data = {1, 2, 3, 4, 5};
    partitioning p1(data.begin(), data.end());
    partitioning p2(data.begin(), data.end());
    
    WHEN("one adds 1 part with add_part_begin and the other with add_parts_begin") {
      p1.add_part_begin(0);
      p2.add_parts_begin(0, 1);
      
      THEN("both have the same part count") {
        CHECK(p1.parts_count() == p2.parts_count());
      }
    }
  }
}

SCENARIO("partitioning - grow with add_part_begin") {
  GIVEN("a partitioning with 10 elements") {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    partitioning p(data.begin(), data.end());
    
    WHEN("creating a custom partitioning using add_part_begin and grow") {
      // Add two empty parts at the beginning
      p.add_parts_begin(0, 2);
      
      THEN("we have three parts, last one with all elements") {
        CHECK(p.parts_count() == 3);
        CHECK(p.is_part_empty(0));
        CHECK(p.is_part_empty(1));
        CHECK_FALSE(p.is_part_empty(2));
      }
      
      AND_WHEN("growing part 0 to take 3 elements from part 1") {
        // Part 1 is empty, so we need to grow part 1 first from part 2
        for (int i = 0; i < 3; ++i) {
          p.grow(1);
        }
        
        THEN("part 1 now has 3 elements") {
          auto [b1, e1] = p.part(1);
          CHECK(std::distance(b1, e1) == 3);
        }
        
        THEN("part 2 now has 7 elements") {
          auto [b2, e2] = p.part(2);
          CHECK(std::distance(b2, e2) == 7);
        }
        
        AND_WHEN("growing part 0 to take 2 elements from part 1") {
          for (int i = 0; i < 2; ++i) {
            p.grow(0);
          }
          
          THEN("part 0 has 2 elements") {
            auto [b0, e0] = p.part(0);
            CHECK(std::distance(b0, e0) == 2);
            std::vector<int> part0_data(b0, e0);
            CHECK(part0_data == std::vector<int>{1, 2});
          }
          
          THEN("part 1 has 1 element") {
            auto [b1, e1] = p.part(1);
            CHECK(std::distance(b1, e1) == 1);
            std::vector<int> part1_data(b1, e1);
            CHECK(part1_data == std::vector<int>{3});
          }
          
          THEN("part 2 has 7 elements") {
            auto [b2, e2] = p.part(2);
            CHECK(std::distance(b2, e2) == 7);
            std::vector<int> part2_data(b2, e2);
            CHECK(part2_data == std::vector<int>{4, 5, 6, 7, 8, 9, 10});
          }
        }
      }
    }
  }
}

SCENARIO("partitioning - complex scenario with begin and end operations") {
  GIVEN("a partitioning with 10 elements") {
    std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    partitioning p(data.begin(), data.end());
    
    WHEN("creating a 3-part partition: [10,20,30] [40,50,60] [70,80,90,100]") {
      // Add empty parts at beginning
      p.add_parts_begin(0, 2);
      // Now: [empty] [empty] [10,20,30,40,50,60,70,80,90,100]
      
      // Grow first part to get 3 elements from part 1
      // But part 1 is empty, so first grow part 1 from part 2
      for (int i = 0; i < 6; ++i) {
        p.grow(1);
      }
      // Now: [empty] [10,20,30,40,50,60] [70,80,90,100]
      
      // Now grow first part from part 1
      for (int i = 0; i < 3; ++i) {
        p.grow(0);
      }
      // Now: [10,20,30] [40,50,60] [70,80,90,100]
      
      THEN("we have three parts with the expected sizes") {
        CHECK(p.parts_count() == 3);
        
        auto [b0, e0] = p.part(0);
        auto [b1, e1] = p.part(1);
        auto [b2, e2] = p.part(2);
        
        CHECK(std::distance(b0, e0) == 3);
        CHECK(std::distance(b1, e1) == 3);
        CHECK(std::distance(b2, e2) == 4);
      }
      
      THEN("each part contains the expected elements") {
        auto [b0, e0] = p.part(0);
        auto [b1, e1] = p.part(1);
        auto [b2, e2] = p.part(2);
        
        std::vector<int> part0(b0, e0);
        std::vector<int> part1(b1, e1);
        std::vector<int> part2(b2, e2);
        
        CHECK(part0 == std::vector<int>{10, 20, 30});
        CHECK(part1 == std::vector<int>{40, 50, 60});
        CHECK(part2 == std::vector<int>{70, 80, 90, 100});
      }
    }
  }
}
