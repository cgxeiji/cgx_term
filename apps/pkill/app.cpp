#include "app.hpp"

#include <cstring>

#include "../../../scheduler/scheduler.hpp"

namespace cgx::term::apps {

cmd_t pkill = {
    "pkill",
    "kill a process",
    nullptr,                            // init
    [](auto& term, const auto* args) {  // run
        param<bool>    all{'a', "kill all processes with the same name", args};
        param<void>    name{"name of the process to kill", args};
        param<uint8_t> test{'t', "test number", args};

        auto is_help = param_help(
            term, "pkill", args,
            {
                &all,
                &test,
                &name,
            });
        if (is_help) {
            return cgx::term::cmd_t::ret_code::ok;
        }

        term.printf("test: %d\n", test.value());

        if (all) {
            bool ret = false;
            while (cgx::sch::scheduler.pkill(name)) {
                term.printf("%s killed\n", name.value());
                ret = true;
            }
            if (ret) {
                return cgx::term::cmd_t::ret_code::ok;
            }
        } else {
            if (cgx::sch::scheduler.pkill(name)) {
                term.printf("%s killed\n", name.value());
                return cgx::term::cmd_t::ret_code::ok;
            }
        }

        term.printf("%s not found\n", name.value());
        return cgx::term::cmd_t::ret_code::error;
    },
    nullptr,  // exit
};

}

