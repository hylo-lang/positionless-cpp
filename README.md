# Postionless algorithms -- C++ prototypes

We want to have an implementation of positionless algorithms and a translation from C++ iterators to positionless, so that we can use positionless with STL algorithms.


## Positionless features
- Construction from a range (forward access, bidirectional and random access)
- `paritioning` accessors:
  - `parts_count() -> size_t`
  - `part(size_t part_index) -> std::pair<Iterator, Iterator>`
  - `is_part_empty(size_t part_index) -> bool`
  - `part_size(size_t part_index) -> size_t` -- constant time for random access, otherwise linear
- growing parts (at the end):
  - `grow`
  - `grow_by`
- shrinking parts (for bidirectional collections):
  - `shrink`
  - `shrink_by`
- transferring elements between parts:
  - `transfer_to_prev`
  - `transfer_to_next`
- creation and destruction of parts:
  - `add_part_end` / `add_part_begin`
  - `add_parts_end` / `add_parts_begin`
  - `remove_part`

## Translation from iterators
TODO

## Building & testing
```
cmake -D CMAKE_BUILD_TYPE=Release -G Ninja -S . -B .build
cmake --build .build
ctest --test-dir .build
```