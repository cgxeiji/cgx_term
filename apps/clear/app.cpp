#include "app.hpp"

namespace cgx::term::apps {

cmd_t clear = {
    "clear",
    [](auto print, const auto*) {
        print("\033[2J");
        print("\033[H");
        return cgx::term::cmd_t::ret_code::ok;
    },
};

}

