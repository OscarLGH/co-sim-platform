
#pragma once
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <atomic>
#include <mutex>

class debugger {
public:
    enum LEVEL {
        OFF = 0,
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4
    };

    static void set_level(LEVEL level) { 
        instance().m_level.store(level); 
    }

    template<typename... Args>
    static void log(LEVEL level, const char* file, int line, 
                   const char* format, Args... args) {
        auto& inst = instance();
        if(level <= inst.m_level.load()) {
            std::lock_guard<std::mutex> lock(inst.m_mutex);
            inst.print_log(level, file, line, format, args...);
        }
    }

private:
    std::atomic<LEVEL> m_level{DEBUG};
    std::mutex m_mutex;

    debugger() = default;
    static debugger& instance() {
        static debugger inst;
        return inst;
    }

    const char* get_color(LEVEL level) {
        switch(level) {
            case ERROR: return "\033[31m";
            case WARN:  return "\033[33m";
            case INFO:  return "\033[32m";
            case DEBUG: return "\033[34m";
            default:    return "";
        }
    }

    const char* get_level_str(LEVEL level) {
        switch(level) {
            case ERROR: return "ERROR";
            case WARN:  return "WARN";
            case INFO:  return "INFO";
            case DEBUG: return "DEBUG";
            default:    return "";
        }
    }

    std::string get_timestamp() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        auto timer = system_clock::to_time_t(now);
        std::tm bt = *std::localtime(&timer);
        
        std::ostringstream oss;
        oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    template<typename... Args>
    void print_log(LEVEL level, const char* file, int line, 
                 const char* format, Args... args) {
        printf("%s[%s][%s] %s:%d | ", 
              get_color(level), get_timestamp().c_str(), 
              get_level_str(level), file, line);
        printf(format, args...);
        printf("\033[0m\n");
    }
};

#define LOG_DEBUG(fmt, ...) \
    debugger::log(debugger::DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_INFO(fmt, ...) \
    debugger::log(debugger::INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    debugger::log(debugger::WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    debugger::log(debugger::ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
