#pragma once

#include <array>
#include <cstdarg>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace cgx::term {

class term_t;

class cmd_t {
   public:
    enum class ret_code {
        ok,
        error,
        alive,
        killed,
    };

    cmd_t(const char* cmd, const char* description,
          std::function<bool(term_t&, const char*)>     init,
          std::function<ret_code(term_t&, const char*)> fn,
          std::function<bool(term_t&, const char*)>     exit)
        : m_init_fn(init), m_fn(fn), m_exit_fn(exit) {
        size_t len = std::strlen(cmd);
        if (len >= m_cmd.size() - 1) {
            len = m_cmd.size() - 1;
        }
        memcpy(m_cmd.data(), cmd, len);
        len = std::strlen(description);
        if (len >= m_description.size() - 1) {
            len = m_description.size() - 1;
        }
        memcpy(m_description.data(), description, len);
    }

    ret_code run(term_t& term, const char* s) const { return m_fn(term, s); }
    bool     init(term_t& term, const char* s) const {
        if (!m_init_fn) {
            return true;
        }
        return m_init_fn(term, s);
    }
    bool exit(term_t& term, const char* s) const {
        if (!m_exit_fn) {
            return true;
        }
        return m_exit_fn(term, s);
    }
    auto& cmd() const { return m_cmd; }

    const char* name() const { return m_cmd.data(); }
    const char* description() const { return m_description.data(); }

   private:
    std::array<char, 9>                           m_cmd{};
    std::array<char, 64>                          m_description{};
    std::function<bool(term_t&, const char*)>     m_init_fn{};
    std::function<ret_code(term_t&, const char*)> m_fn{};
    std::function<bool(term_t&, const char*)>     m_exit_fn{};
};

class term_t {
   public:
    term_t(std::function<void(const char*)> print) : m_print(print) {}

    void add(const cmd_t& cmd) { m_cmds.push_back(cmd); }
    void print(const char* s) const { m_print(s); }

    void printf(const char* fmt, ...) const {
        char    buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        m_print(buf);
    }

    void run() {
        process_buffer();
        if (m_last_ret == cmd_t::ret_code::alive) {
            // exit if ctrl+c
            if (std::strncmp(m_line.data(), "\x03", m_line.size()) == 0) {
                m_cmds[m_cmd_index].exit(*this, "");
                m_last_ret = cmd_t::ret_code::killed;
                reset_line();
                return;
            }
            m_last_ret = m_cmds[m_cmd_index].run(*this, m_line.data());
            if (m_last_ret != cmd_t::ret_code::alive) {
                m_cmds[m_cmd_index].exit(*this, "");
                reset_line();
                return;
            }
            reset_line(false);
            return;
        }

        if (!m_is_line_valid) {
            return;
        }
        // split line by cmd name and arguments "cmd args"
        auto args = std::strchr(m_line.data(), ' ');
        if (args) {
            *args = '\0';
            args++;
        }
        size_t i = 0;
        // printf("cmd: %s, args: %s\n", m_line.data(), args);
        for (const auto& cmd : m_cmds) {
            if (std::strncmp(m_line.data(), cmd.cmd().data(), m_line.size()) ==
                0) {
                m_cmd_index = i;
                if (!cmd.init(*this, args)) {
                    m_last_ret = cmd_t::ret_code::error;
                    reset_line();
                    return;
                }
                m_last_ret = cmd.run(*this, args);
                if (m_last_ret != cmd_t::ret_code::alive) {
                    m_cmds[m_cmd_index].exit(*this, args);
                }
                reset_line();
                return;
            }
            i++;
        }
        reset_line();
    }

    const auto& commands() const { return m_cmds; }

    void input(const char input) {
        m_input_buffer[m_input_tail] = input;
        m_input_tail = (m_input_tail + 1) % m_input_buffer.size();
    }

   private:
    std::vector<cmd_t>               m_cmds{};
    std::array<char, 1024>           m_input_buffer{};
    std::array<char, 1024>           m_line{};
    std::function<void(const char*)> m_print{nullptr};
    bool                             m_is_line_valid{false};

    size_t m_input_head{0};
    size_t m_input_tail{0};

    size_t m_line_index{0};

    size_t          m_cmd_index{0};
    cmd_t::ret_code m_last_ret{cmd_t::ret_code::ok};

    void process_buffer() {
        if (m_is_line_valid) {
            return;
        }
        while (m_input_head != m_input_tail) {
            const auto c = m_input_buffer[m_input_head];
            m_input_head = (m_input_head + 1) % m_input_buffer.size();
            if (c == '\b' || c == 127) {
                if (m_line_index == 0) {
                    continue;
                }
                m_line_index         = (m_line_index - 1) % m_line.size();
                m_line[m_line_index] = '\0';
                m_print("\b \b");
                continue;
            }
            if (c == '\n' || c == '\r') {
                m_print("\n");
                m_line[m_line_index] = '\0';
                m_is_line_valid      = true;
                return;
            }
            m_line[m_line_index] = c;
            m_line_index         = (m_line_index + 1) % m_line.size();
            m_line[m_line_index] = '\0';
            // pass through ctrl+c
            if (c == '\x03') {
                m_is_line_valid = true;
                return;
            }
            char buf[2] = {c, '\0'};
            m_print(buf);
        }
    }

    void reset_line(bool prompt = true) {
        m_line_index = 0;
        m_line.fill('\0');
        m_is_line_valid = false;
        if (prompt) {
            m_print("> ");
        }
    }
};

}  // namespace cgx::term
