#pragma once
#include "container.hpp"
#include <vector>

namespace safe
{
    template<typename T, typename Mode=mode>
    using vector = container<std::vector<T>, Mode>;
};
