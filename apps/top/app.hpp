#pragma once

#include <functional>

#include "../../term.hpp"

namespace cgx::term::apps {
namespace ns_top {
void stats_screen(std::function<void(const char*)> print);
}  // namespace ns_top

extern cmd_t top;
}  // namespace cgx::term::apps
