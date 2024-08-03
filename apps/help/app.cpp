#include "app.hpp"

#include "../../../scheduler/scheduler.hpp"

namespace cgx::term::apps {

cmd_t help = {
    "help",
    "show the list of cmds and their descriptions",
    nullptr,                            // init
    [](auto& term, const auto* args) {  // run
        for (const auto& cmd : term.commands()) {
            term.printf("  % 8s: %s\n", cmd.name(), cmd.description());
        }
        return cgx::term::cmd_t::ret_code::ok;
    },
    nullptr,  // exit
};

}  // namespace cgx::term::apps
