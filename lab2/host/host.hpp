#pragma once
#include "client.hpp"
#include "client_info.hpp"
#include "conn.hpp"
#include "message.hpp"
#include "utils.hpp"
#include "wolf.hpp"
#include <algorithm>
#include <iostream>
#include <semaphore.h>
#include <signal.h>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>

class Host {
  public:
    Host() {
        auto ui_dist = std::uniform_int_distribution<std::mt19937::result_type>(min_clients, max_clients);
        std::random_device rd;
        auto rng = std::mt19937(rd());
        active_clients = ui_dist(rng);

        for (unsigned int idx = 0; idx < active_clients; ++idx) {
            sem_t *sem_host_msg =
                (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
            sem_t *sem_client_msg =
                (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
            if (sem_host_msg == MAP_FAILED || sem_client_msg == MAP_FAILED)
                throw std::runtime_error("Couldn't allocate shared memory for semaphores with mmap");

            if (sem_init(sem_host_msg, 1, 0) == -1 || sem_init(sem_client_msg, 1, 0) == -1)
                throw std::runtime_error("Couldn't initialize semaphores with sem_init");

            Conn *connection = Conn::create(true, idx + 1);

            cl_info_vec.emplace_back(ClientInfo{sem_host_msg, sem_client_msg, 0, connection, 0, idx + 1, true});
        }
    }

    ~Host() {
        for (const auto &cl_info : cl_info_vec) {
            if (sem_destroy(cl_info.sem_host_msg) == -1 || sem_destroy(cl_info.sem_client_msg) == -1) {
                std::cout << "Couldn't destroy semaphores" << std::endl;
                std::exit(1);
            }

            if (munmap(cl_info.sem_host_msg, sizeof(sem_t)) == -1 ||
                munmap(cl_info.sem_client_msg, sizeof(sem_t)) == -1) {
                std::cout << "Couldn't free shared memory from semaphores" << std::endl;
                std::exit(1);
            }

            delete cl_info.connection;
        }
    }

    void start_game() {
        for (auto &cl_info : cl_info_vec) {
            add_client(cl_info);
        }

        std::cout << "Playing with " << active_clients << " goats" << std::endl << std::endl;

        while (active_clients > 0) {
            size_t wolf_roll = (size_t)-1;
            pthread_t read_thread;
            std::cout << "Input wolf roll value: ";

            pthread_create(&read_thread, NULL, Host::manual_roll_input, &wolf_roll);
            sleep(3);

            if (wolf_roll == (size_t)-1) {
                pthread_cancel(read_thread);
                pthread_join(read_thread, NULL);
                wolf_roll = wolf.roll();
            } else {
                pthread_join(read_thread, NULL);
            }

            std::cout << std::endl << "Wolf rolled " << wolf_roll << std::endl;

            for (auto &cl_info : cl_info_vec) {
                if (!cl_info.active)
                    continue;

                wait_sem_with_timeout(cl_info.sem_client_msg, sem_timeout_secs);

                Message msg;
                cl_info.connection->read(msg);
                std::cout << "Goat " << cl_info.client_id << " rolled " << msg.roll << std::endl;

                // Determine new goat state
                if (msg.alive) {
                    if ((abs((int)msg.roll - (int)wolf_roll) <= ALIVE_GOAT_GAP_REQUIRED))
                        msg.alive = true;
                    else
                        msg.alive = false;
                } else {
                    if ((abs((int)msg.roll - (int)wolf_roll) <= DEAD_GOAT_GAP_REQUIRED))
                        msg.alive = true;
                    else
                        msg.alive = false;
                }

                auto bool_to_str = [](bool val) -> std::string { return val ? "True" : "False"; };
                std::cout << "Goat " << cl_info.client_id << " alive: " << bool_to_str(msg.alive) << std::endl;

                if (!msg.alive)
                    ++cl_info.death_row;
                else
                    cl_info.death_row = 0;

                std::cout << "Goat " << cl_info.client_id << " death row: " << cl_info.death_row << std::endl;

                if (cl_info.death_row > 1) {
                    std::cout << "Goat " << cl_info.client_id << " died" << std::endl;
                    msg.active = false;
                    cl_info.active = false;
                    --active_clients;
                }

                cl_info.connection->write(msg);
                sem_post(cl_info.sem_host_msg);
            }
            std::cout << "End of round" << std::endl << std::endl;
        }

        for (const auto &cl_info : cl_info_vec)
            kill(cl_info.child_pid, SIGTERM);
    }

  private:
    void add_client(ClientInfo &cl_info) {
        pid_t child_pid = fork();
        cl_info.child_pid = child_pid;
        if (child_pid == 0) {
            Client cl(cl_info.sem_host_msg, cl_info.sem_client_msg, cl_info.client_id);
            cl.run();
        } else if (child_pid == -1)
            throw std::runtime_error("Couldn't create child proccess");
    }

    static void *manual_roll_input(void *roll) {
        size_t tmp;

        if (std::cin.fail())
            std::cin.clear();

        std::cin >> tmp;

        (*(size_t *)roll) = tmp;

        pthread_exit(NULL);
    }

    std::vector<ClientInfo> cl_info_vec;
    unsigned int active_clients;
    int min_clients = 1;
    int max_clients = 5;
    const size_t sem_timeout_secs = 10;

    Wolf wolf;
    const size_t ALIVE_GOAT_GAP_REQUIRED = 70;
    const size_t DEAD_GOAT_GAP_REQUIRED = 20;
};