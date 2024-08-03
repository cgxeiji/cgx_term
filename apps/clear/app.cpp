#include "app.hpp"

namespace cgx::term::apps {

cmd_t clear = {
    "clear",
    "clear the screen",
    nullptr,                             // init
    [](const auto& term, const auto*) {  // run
        term.print("\033[2J");
        term.print("\033[H");
        return cgx::term::cmd_t::ret_code::ok;
    },
    nullptr,  // exit
};

}

