#pragma once
#include "config.hpp"
#include <chrono>
#include <filesystem>

class Daemon {
  public:
    static Daemon &create(const std::string &path_to_config);

    [[noreturn]] void start() const;

    Daemon() = delete;
    Daemon(const Daemon &) = delete;
    Daemon(Daemon &&) = delete;

  protected:
    static void signal_handler(int sig);

    void close_running() const;
    void daemonize() const;

  private:
    explicit Daemon(const std::string &path_to_config);

    static std::string config_path;
    static Config config;
    const std::string pid_file_path = std::filesystem::absolute("mydaemon.pid");
    const std::string proc_dir = "/proc";
    const std::string syslog_proc_name = "mydaemon";
    const unsigned int interval_s = 30;
};
