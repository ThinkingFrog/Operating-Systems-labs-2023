#include "daemon.hpp"
#include <chrono>
#include <csignal>
#include <ctime>
#include <filesystem>
#include <sys/stat.h>
#include <syslog.h>
#include <tuple>
#include <unistd.h>

namespace fs = std::filesystem;

std::string Daemon::config_path;
Config Daemon::config;

void Daemon::start() const {
    close_running();
    daemonize();
    syslog(LOG_NOTICE, "Daemon started");

    while (true) {
        const std::vector<std::tuple<fs::path, fs::path, std::string>> &entries = config.get_entries();

        for (const auto &[dir1, dir2, ext] : entries) {

            for (const auto &file : fs::directory_iterator(dir2))
                fs::remove(file);

            for (const auto &file : fs::directory_iterator(dir1)) {
                if (file.path().extension() == ext)
                    fs::copy(file.path(), dir2);
            }
        }

        sleep(interval_s);
    }
}

Daemon &Daemon::create(const std::string &path_to_config) {
    // local static variable ensures memory is freed when program terminates
    // initialized on first use and never after
    static Daemon instance(path_to_config);
    return instance;
}

Daemon::Daemon(const std::string &path_to_config) {
    config_path = fs::absolute(path_to_config);
    config = Config(config_path);
}

void Daemon::close_running() const {
    int pid;
    std::ifstream f(pid_file_path);
    f >> pid;
    if (fs::exists(proc_dir + "/" + std::to_string(pid)))
        kill(pid, SIGTERM);
}

void Daemon::signal_handler(int sig) {
    if (sig == SIGHUP)
        config.read(config_path);
    if (sig == SIGTERM) {
        syslog(LOG_NOTICE, "Deamon terminated");
        closelog();
        exit(EXIT_SUCCESS);
    }
}

void Daemon::daemonize() const {
    pid_t pid = fork();
    if (pid != 0)
        exit(EXIT_FAILURE);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    std::signal(SIGHUP, signal_handler);
    std::signal(SIGTERM, signal_handler);

    pid = fork();
    if (pid != 0)
        exit(EXIT_FAILURE);

    umask(0);
    if (chdir("/") != 0)
        exit(EXIT_FAILURE);

    for (long x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
        close(x);
    openlog(syslog_proc_name.c_str(), LOG_PID, LOG_DAEMON);

    std::ofstream f(pid_file_path, std::ios_base::trunc);
    f << getpid();
}
