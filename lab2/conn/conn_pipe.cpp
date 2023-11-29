#include "conn.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

class ConnPipe : public Conn {
  public:
    ConnPipe(int host_pipe_fd[], int client_pipe_fd[], bool host) {
        host_fd[0] = host_pipe_fd[0];
        host_fd[1] = host_pipe_fd[1];
        client_fd[0] = client_pipe_fd[0];
        client_fd[1] = client_pipe_fd[1];
        is_host = host;
    }
    ~ConnPipe() {
        if (close(host_fd[0]) == -1 || close(host_fd[1]) == -1 || close(client_fd[0]) == -1 ||
            close(client_fd[1]) == -1) {
            std::cout << "Couldn't close pipe file descriptor" << std::endl;
            std::exit(1);
        }
    }

    void read(Message &msg) override {
        int err;

        if (is_host)
            err = ::read(client_fd[0], &msg, sizeof(msg));
        else
            err = ::read(host_fd[0], &msg, sizeof(msg));

        if (err == -1)
            throw std::runtime_error("Couldn't read pipe");
    }
    void write(const Message &msg) override {
        int err;

        if (is_host)
            err = ::write(host_fd[1], &msg, sizeof(msg));
        else
            err = ::write(client_fd[1], &msg, sizeof(msg));

        if (err == -1)
            throw std::runtime_error("Couldn't write to pipe");
    }

  private:
    int host_fd[2], client_fd[2];
    bool is_host;
};

Conn *Conn::create(bool create, int conn_id) {
    static std::vector<std::array<int, 2>> host_fd_vec;
    static std::vector<std::array<int, 2>> client_fd_vec;

    if ((conn_id > (int)host_fd_vec.size() || conn_id > (int)client_fd_vec.size()) && create) {
        host_fd_vec.push_back(std::array<int, 2>());
        client_fd_vec.push_back(std::array<int, 2>());
    }

    if (create && (pipe(host_fd_vec[conn_id - 1].data()) == -1 || pipe(client_fd_vec[conn_id - 1].data()) == -1))
        throw std::runtime_error("Couldn't create pipe");

    return new ConnPipe(host_fd_vec[conn_id - 1].data(), client_fd_vec[conn_id - 1].data(), create);
}