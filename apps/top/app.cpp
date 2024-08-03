#include "app.hpp"

#include "../../../scheduler/scheduler.hpp"

namespace cgx::term::apps {

namespace ns_top {
void stats_screen(term_t& term) {
    const auto& threads = cgx::sch::scheduler.threads();

    static int32_t        last_lines = 0;
    std::array<char, 128> buf;

    term.print("\033[2K");
    term.print("\033[1m");

    term.printf("%93s\n", "TOP (q)uit (r)eset_stats (n)ow");

    term.print("\033[0m");

    int32_t lines = 0;
    for (uint8_t idx = 0; idx < threads.size(); ++idx) {
        if (!threads[idx]) {
            continue;
        }
        auto& thread          = threads[idx];
        auto  available_tasks = thread->size();
        if (available_tasks == 0) {
            continue;
        }

        thread->lock();
        auto watch = thread->watch();
        thread->unlock();

        auto min = watch.duration().min();
        if (min == std::numeric_limits<cgx::sch::scheduler_t::time_t>::max()) {
            min = 0;
        }
        auto max = watch.duration().max();
        if (max ==
            std::numeric_limits<cgx::sch::scheduler_t::time_t>::lowest()) {
            max = 0;
        }
        std::snprintf(buf.data(), buf.size(),
                      "== THREAD %1u == [ tasks: %-2u, mean: %lluus, "
                      "min: %lluus, max: %lluus ]",
                      idx, available_tasks, watch.duration().mean(), min, max);
        std::snprintf(buf.data() + std::strlen(buf.data()), buf.size(), "%*s\n",
                      93 - std::strlen(buf.data()), "");
        term.print("\033[2K");
        term.print("\033[30;42m");
        term.print(buf.data());
        term.print("\033[0m");
        lines++;

        std::snprintf(
            buf.data(), buf.size(), "   %10s %12s %12s %12s %12s %12s %12s\n",
            "task", "every", "actual", "next", "mean_us", "min_us", "max_us");
        term.print("\033[2K");
        term.print("\033[90m");
        term.print(buf.data());
        term.print("\033[0m");
        lines++;

        thread->lock();
        for (const auto& task : *thread) {
            if (!task) {
                continue;
            }
            char state[3] = "  ";
            switch (task.status()) {
                case cgx::sch::task_t::status_t::running:
                    state[0] = 'O';
                    term.print("\033[1;32m");
                    break;
                case cgx::sch::task_t::status_t::stopped:
                    state[1] = 'S';
                    term.print("\033[1;91m");
                    break;
                case cgx::sch::task_t::status_t::paused:
                    state[1] = 'p';
                    break;
                case cgx::sch::task_t::status_t::delayed:
                    state[0] = 'd';
                    term.print("\033[31m");
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
            term.print("\033[2K");
            term.print(buf.data());
            term.print("\033[0m");
            lines++;
        }
        thread->unlock();
        term.print("\033[2K");
        term.print("\n");
        lines++;
    }

    for (int32_t i = 0; i < last_lines - lines; ++i) {
        term.print("\033[2K");
        term.print("\n");
    }
    term.print("\033[H");
}
}  // namespace ns_top

cmd_t top = {
    "top",
    "show current processes",
    [](auto& term, const auto*) {  // init
        term.print("\033[2J");
        term.print("\033[H");
        cgx::sch::scheduler.add({
            "top",
            1000'000,
            [&term] {
                ns_top::stats_screen(term);
                return true;
            },
        });
        return true;
    },
    [](auto& term, const auto* args) {  // run
        if (strcmp(args, "q") == 0) {
            return cgx::term::cmd_t::ret_code::ok;
        }
        if (strcmp(args, "r") == 0) {
            cgx::sch::scheduler.reset_stats();
            ns_top::stats_screen(term);
            return cgx::term::cmd_t::ret_code::alive;
        }
        if (strcmp(args, "n") == 0) {
            ns_top::stats_screen(term);
            return cgx::term::cmd_t::ret_code::alive;
        }
        return cgx::term::cmd_t::ret_code::alive;
    },
    [](auto& term, const auto*) {  // exit
        cgx::sch::scheduler.pkill("top");
        term.print("\033[2J");
        term.print("\033[H");
        return true;
    },
};

}  // namespace cgx::term::apps
