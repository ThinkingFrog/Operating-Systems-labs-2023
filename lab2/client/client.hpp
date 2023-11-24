#pragma once
#include "conn.hpp"
#include "goat.hpp"
#include "message.hpp"
#include "utils.hpp"
#include <iostream>
#include <semaphore.h>
#include <stdexcept>

class Client {
  public:
    Client(sem_t *sem_host, sem_t *sem_client, int client_id) {
        sem_host_msg = sem_host;
        sem_client_msg = sem_client;
        connection = Conn::create(false, client_id);
        id = client_id;
    }

    void run() {
        while (true) {
            Message msg = {goat.roll(), goat.alive(), true};
            connection->write(msg);
            sem_post(sem_client_msg);

            wait_sem_with_timeout(sem_host_msg, sem_timeout_secs);
            connection->read(msg);

            if (!msg.active)
                std::exit(0);

            goat.set_status(msg.alive);
        }
    }

  private:
    Goat goat;
    sem_t *sem_client_msg, *sem_host_msg;
    Conn *connection;
    const size_t sem_timeout_secs = 5;
    int id;
};