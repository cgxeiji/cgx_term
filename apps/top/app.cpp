#include "app.hpp"

#include "../../../scheduler/scheduler.hpp"

namespace cgx::term::apps {

namespace ns_top {
void stats_screen(std::function<void(const char*)> print) {
    const auto& tasks_list   = cgx::sch::scheduler.tasks();
    const auto& task_watches = cgx::sch::scheduler.watches();

    static int32_t        last_lines = 0;
    std::array<char, 128> buf;

    std::snprintf(
        buf.data(), buf.size(), "%93s\n", "TOP (q)uit (r)eset_stats (n)ow");
    print("\033[2K");
    print("\033[1m");
    print(buf.data());
    print("\033[0m");

    int32_t lines = 0;
    for (uint8_t p = 0; p < tasks_list.size(); ++p) {
        auto available_tasks = 0;
        for (const auto& task : tasks_list[p]) {
            if (task) {
                available_tasks++;
            }
        }

        if (available_tasks == 0) {
            continue;
        }

        auto min = task_watches[p].duration().min();
        if (min == std::numeric_limits<cgx::sch::scheduler_t::time_t>::max()) {
            min = 0;
        }
        auto max = task_watches[p].duration().max();
        if (max ==
            std::numeric_limits<cgx::sch::scheduler_t::time_t>::lowest()) {
            max = 0;
        }
        std::snprintf(
            buf.data(), buf.size(),
            "== THREAD %1u == [ tasks: %-2u, mean: %lluus, "
            "min: %lluus, max: %lluus ]",
            p, available_tasks, task_watches[p].duration().mean(), min, max);
        std::snprintf(buf.data() + std::strlen(buf.data()), buf.size(), "%*s\n",
                      93 - std::strlen(buf.data()), "");
        print("\033[2K");
        print("\033[30;42m");
        print(buf.data());
        print("\033[0m");
        lines++;

        std::snprintf(
            buf.data(), buf.size(), "   %10s %12s %12s %12s %12s %12s %12s\n",
            "task", "every", "actual", "next", "mean_us", "min_us", "max_us");
        print("\033[2K");
        print("\033[90m");
        print(buf.data());
        print("\033[0m");
        lines++;

        for (const auto& task : tasks_list[p]) {
            if (!task) {
                continue;
            }
            char state[3] = "  ";
            switch (task.status()) {
                case cgx::sch::task_t::status_t::running:
                    state[0] = 'O';
                    print("\033[1;32m");
                    break;
                case cgx::sch::task_t::status_t::stopped:
                    state[1] = 'S';
                    print("\033[1;91m");
                    break;
                case cgx::sch::task_t::status_t::paused:
                    state[1] = 'p';
                    break;
                case cgx::sch::task_t::status_t::delayed:
                    state[0] = 'd';
                    print("\033[31m");
                    break;
                case cgx::sch::task_t::status_t::invalid:
                    state[1] = '-';
                    break;
            }
            const auto run_time = task.run_time();
            auto       min      = run_time.min();
            if (min ==
                std::numeric_limits<cgx::sch::scheduler_t::time_t>::max()) {
                min = 0;
            }
            auto max = run_time.max();
            if (max ==
                std::numeric_limits<cgx::sch::scheduler_t::time_t>::lowest()) {
                max = 0;
            }

            std::snprintf(
                buf.data(), buf.size(),
                "%2s [%8s] %12lld %12lld %12lld %12llu %12llu %12llu\n", state,
                task.name().data(), task.period(), task.actual_period().mean(),
                task.ticks_left(), run_time.mean(), min, max);
            print("\033[2K");
            print(buf.data());
            print("\033[0m");
            lines++;
        }
        print("\033[2K");
        print("\n");
        lines++;
    }

    for (int32_t i = 0; i < last_lines - lines; ++i) {
        print("\033[2K");
        print("\n");
    }
    print("\033[H");
}
}  // namespace ns_top

cmd_t top = {
    "top",
    [](auto print, const auto*) {  // init
        print("\033[2J");
        print("\033[H");
        cgx::sch::scheduler.add(
            "top", 1000'000,
            [print] {
                ns_top::stats_screen(print);
                return true;
            },
            0);
        return true;
    },
    [](auto print, const auto* args) {  // run
        if (strcmp(args, "q") == 0) {
            return cgx::term::cmd_t::ret_code::ok;
        }
        if (strcmp(args, "r") == 0) {
            cgx::sch::scheduler.reset_stats();
            ns_top::stats_screen(print);
            return cgx::term::cmd_t::ret_code::alive;
        }
        if (strcmp(args, "n") == 0) {
            ns_top::stats_screen(print);
            return cgx::term::cmd_t::ret_code::alive;
        }
        return cgx::term::cmd_t::ret_code::alive;
    },
    [](auto print, const auto*) {  // exit
        cgx::sch::scheduler.pkill("top");
        print("\033[2J");
        print("\033[H");
        return true;
    },
};

}  // namespace cgx::term::apps
