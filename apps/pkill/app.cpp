#include "app.hpp"

#include <cstring>

#include "../../../scheduler/scheduler.hpp"

namespace cgx::term::apps {

cmd_t pkill = {
    "pkill",
    [](auto print, const auto* args) {
        // get flags starting with '-'
        auto idx = std::strchr(args, '-');
        if (idx != nullptr) {
            char flag[32];
            // print the flags
            uint32_t i = 0;
            while (*idx != '\0' && *idx != ' ') {
                if (i >= 31) {
                    print("invalid flag\n");
                    return cgx::term::cmd_t::ret_code::error;
                }
                flag[i] = *idx;
                i++;
                idx++;
            }
            flag[i] = '\0';
            if (std::strcmp(flag, "-a") == 0) {
                bool ret = false;
                auto name = args + i + 1;
                while (cgx::sch::scheduler.pkill(name)) {
                    printf("%s killed\n", name);
                    ret = true;
                }
                if (ret) {
                    return cgx::term::cmd_t::ret_code::ok;
                }

                printf("%s not found\n", name);
                return cgx::term::cmd_t::ret_code::error;
            }
        }

        if (cgx::sch::scheduler.pkill(args)) {
            printf("%s killed\n", args);
            return cgx::term::cmd_t::ret_code::ok;
        }

        printf("%s not found\n", args);
        return cgx::term::cmd_t::ret_code::error;
    },
};

}

