#pragma once

#include "container.hpp"
#include <string>

namespace safe {

using string = container<std::string, mode>;
// Should it be this?
// using string = value<std::string>;
}
