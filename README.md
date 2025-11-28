# Postionless algorithms -- C++ prototypes

We want to have an implementation of positionless algorithms and a translation from C++ iterators to positionless, so that we can use positionless with STL algorithms.


## Positionless features
TODO

## Translation from iterators
TODO

## Building & testing
```
cmake -D CMAKE_BUILD_TYPE=Release -G Ninja -S . -B .build
cmake --build .build
ctest --test-dir .build
```