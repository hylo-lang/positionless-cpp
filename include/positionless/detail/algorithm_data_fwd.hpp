#pragma once

#include <iterator>
#include <memory>

namespace positionless::detail {

template <std::forward_iterator BaseIterator>
class algorithm_data;

template <std::forward_iterator BaseIterator>
using algorithm_data_ptr = std::shared_ptr<algorithm_data<BaseIterator>>;

} // namespace positionless::detail
