#pragma once

#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace cgx::term {

class term_t;

class param_t {
   public:
    virtual const char id() const = 0;
    virtual const char* description() const = 0;

    virtual bool needs_input() const { return true; };

    ~param_t() = default;
};

template <typename T>
class param : public param_t {
   public:
    T parse(const char* s) {
        size_t i = 0;
        while (s[i + 1] != '\0') {
            if (s[i] == '-' && s[i + 1] == m_id && s[i + 2] == '=') {
                m_value = true;
                return parse_value(s + i + 3);
            }
            i++;
        }
        return T{};
    }

    T parse_value(const char* s) {
        if constexpr (std::is_same_v<T, int>) {
            return std::atoi(s);
        } else if constexpr (std::is_same_v<T, int8_t>) {
            return std::atoi(s);
        } else if constexpr (std::is_same_v<T, int16_t>) {
            return std::atoi(s);
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return std::atoi(s);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::atoll(s);
        } else if constexpr (std::is_same_v<T, uint8_t>) {
            return std::strtoul(s, nullptr, 10);
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            return std::strtoul(s, nullptr, 10);
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return std::strtoul(s, nullptr, 10);
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return std::strtoull(s, nullptr, 10);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::atof(s);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::atof(s);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return std::string(s);
        } else {
            return T{};
        }
    }

    bool needs_input() const override {
        if constexpr (std::is_same_v<T, bool>) {
            return false;
        } else {
            return true;
        }
    }

    const char id() const override { return m_id; }
    const char* description() const override { return m_description.data(); }

    operator bool() const { return m_valid; }
    bool is_valid() const { return m_valid; }
    operator T() const { return m_value; }
    T value() const { return m_value; }

    param(const char id, const char* description, const char* s) : m_id(id) {
        size_t len = std::strlen(description);
        if (len >= m_description.size() - 1) {
            len = m_description.size() - 1;
        }
        memcpy(m_description.data(), description, len);
        m_value = parse(s);
    }

   private:
    const char m_id;
    std::array<char, 64> m_description{0};
    T m_value{};
    bool m_valid{false};
};

template <>
class param<bool> : public param_t {
   public:
    bool parse(const char* s) {
        size_t i = 0;
        while (s[i + 1] != '\0') {
            if (s[i] == '-' && s[i + 1] == m_id) {
                return true;
            }
            i++;
        }
        return false;
    }

    bool needs_input() const override { return false; }

    const char id() const override { return m_id; }
    const char* description() const override { return m_description.data(); }

    operator bool() const { return m_value; }
    bool value() const { return m_value; }

    param(const char id, const char* description, const char* s) : m_id(id) {
        size_t len = std::strlen(description);
        if (len >= m_description.size() - 1) {
            len = m_description.size() - 1;
        }
        memcpy(m_description.data(), description, len);
        m_value = parse(s);
    }

   private:
    const char m_id;
    std::array<char, 64> m_description{0};
    bool m_value{};
};

template <>
class param<void> : public param_t {
   public:
    const char* parse(const char* s) const {
        size_t i = 0;
        size_t pos = 0;
        while (s[i] != '\0') {
            if (s[i] == '-') {
                pos = i + 1;
            }
            i++;
        }
        if (pos == 0) {
            return s;
        }
        if (s[pos + 1] == ' ') {
            return &s[pos + 2];
        }
        i = pos + 2;
        bool valid = false;
        while (s[i] != '\0') {
            if (s[i] == ' ') {
                pos = i + 1;
                valid = true;
                break;
            }
            i++;
        }
        if (!valid) {
            return &s[i];
        }
        return &s[pos];
    }

    const char id() const override { return m_id; }
    const char* description() const override { return m_description.data(); }

    operator const char*() const { return m_value; }
    const char* value() const { return m_value; }

    param(const char* description, const char* s) : m_value(parse(s)) {
        size_t len = std::strlen(description);
        if (len >= m_description.size() - 1) {
            len = m_description.size() - 1;
        }
        memcpy(m_description.data(), description, len);
    }

   private:
    const char m_id{' '};
    std::array<char, 64> m_description{0};
    const char* m_value{};
};

class cmd_t {
   public:
    enum class ret_code {
        ok,
        error,
        alive,
        killed,
    };

    cmd_t(const char* cmd, const char* description,
          std::function<bool(term_t&, const char*)> init,
          std::function<ret_code(term_t&, const char*)> fn,
          std::function<bool(term_t&, const char*)> exit)
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
    bool init(term_t& term, const char* s) const {
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
    std::array<char, 9> m_cmd{0};
    std::array<char, 64> m_description{0};
    std::function<bool(term_t&, const char*)> m_init_fn{};
    std::function<ret_code(term_t&, const char*)> m_fn{};
    std::function<bool(term_t&, const char*)> m_exit_fn{};
};

class term_t {
   public:
    term_t(std::function<void(const char*)> print) : m_print(print) {}

    void add(const cmd_t& cmd) { m_cmds.push_back(cmd); }
    void print(const char* s) const { m_print(s); }

    void printf(const char* fmt, ...) const {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        m_print(buf);
    }

    void enable_quick_cmd(bool enable) { m_is_quick_cmd_enabled = enable; }

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
        auto len = std::strlen(m_line.data());
        if (len == 0) {
            reset_line();
            return;
        }
        size_t i = 0;
        // printf("cmd: %s, args: %s\n", m_line.data(), args);
        for (const auto& cmd : m_cmds) {
            if (std::strncmp(cmd.cmd().data(), m_line.data(), len) == 0) {
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
    std::vector<cmd_t> m_cmds{};
    std::array<char, 1024> m_input_buffer{};
    std::array<char, 1024> m_line{};
    std::array<char, 1024> m_last_line{};
    std::function<void(const char*)> m_print{nullptr};
    bool m_is_line_valid{false};
    bool m_is_quick_cmd_enabled{false};

    size_t m_input_head{0};
    size_t m_input_tail{0};

    size_t m_line_index{0};

    size_t m_cmd_index{0};
    cmd_t::ret_code m_last_ret{cmd_t::ret_code::ok};

    void process_buffer() {
        if (m_is_line_valid) {
            return;
        }
        while (m_input_head != m_input_tail) {
            const auto c = m_input_buffer[m_input_head];
            m_input_head = (m_input_head + 1) % m_input_buffer.size();
            // if arrow
            if (c == '\x1b') {
                if (m_input_head == m_input_tail) {
                    continue;
                }
                const auto c1 = m_input_buffer[m_input_head];
                m_input_head = (m_input_head + 1) % m_input_buffer.size();
                const auto c2 = m_input_buffer[m_input_head];
                m_input_head = (m_input_head + 1) % m_input_buffer.size();
                if (c1 == '[' && c2 == 'A') {
                    // arrow up
                    m_print("\033[2K\033[1G");
                    reset_line();
                    m_line_index = strlen(m_last_line.data());
                    if (m_line_index > 0) {
                        memcpy(m_line.data(), m_last_line.data(),
                               m_line_index + 1);
                        m_print(m_line.data());
                    }
                    continue;
                }
                if (c1 == '[' && c2 == 'B') {
                    // arrow down
                    m_print("\033[2K\033[1G");
                    m_line_index = 0;
                    m_line[m_line_index] = '\0';
                    reset_line();
                    continue;
                }
                continue;
            }

            if (c == '\b' || c == 127) {
                if (m_line_index == 0) {
                    continue;
                }
                m_line_index = (m_line_index - 1) % m_line.size();
                m_line[m_line_index] = '\0';
                m_print("\b \b");
                continue;
            }
            if (c == '\n' || c == '\r') {
                m_print("\n");
                m_line[m_line_index] = '\0';
                m_is_line_valid = true;
                if (m_line_index > 0) {
                    memcpy(m_last_line.data(), m_line.data(), m_line_index + 1);
                }
                return;
            }
            m_line[m_line_index] = c;
            m_line_index = (m_line_index + 1) % m_line.size();
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

inline bool param_help(term_t& term, const char* cmd, const char* s,
                       const std::initializer_list<param_t*>& args) {
    param<bool> help{'h', "show help", s};
    if (!help) {
        return false;
    }
    term.printf("Usage: %s", cmd);
    for (const auto& arg : args) {
        if (arg->id() == ' ') {
            term.print(" INPUT");
        } else {
            if (arg->needs_input()) {
                term.printf(" -%c=X", arg->id());
            } else {
                term.printf(" -%c", arg->id(), arg->description());
            }
        }
    }
    term.print("\n");
    for (const auto& arg : args) {
        if (arg->id() == ' ') {
            term.printf("  INPUT: %s\n", arg->description());
        } else {
            term.printf("     -%c: %s\n", arg->id(), arg->description());
        }
    }
    return true;
}

}  // namespace cgx::term
