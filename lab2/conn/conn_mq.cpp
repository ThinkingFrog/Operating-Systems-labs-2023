#include "conn.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <mqueue.h>
#include <stdexcept>
#include <sys/mman.h>

class ConnMq : public Conn {
  public:
    ConnMq(const std::string &name, bool create) {
        queue_name = name;

        if (create) {
            mq_attr attr;
            attr.mq_msgsize = sizeof(Message);
            attr.mq_maxmsg = 1;

            descriptor = mq_open(name.c_str(), O_RDWR | O_CREAT, 0666, &attr);
        } else {
            descriptor = mq_open(name.c_str(), O_RDWR);
        }

        if (descriptor == -1) {
            throw std::runtime_error("Couldn't open shared queue");
        }
    }
    ~ConnMq() {
        if (mq_close(descriptor) == -1 || mq_unlink(queue_name.c_str()) == -1) {
            std::cout << errno << std::endl;

            std::cout << "Couldn't free shared memory" << std::endl;
            std::exit(1);
        }
    }
    inline void read(Message &msg) override { mq_receive(descriptor, (char *)&msg, sizeof(msg), &default_prio); }
    inline void write(const Message &msg) override {
        mq_send(descriptor, (const char *)&msg, sizeof(msg), default_prio);
    }

  private:
    mqd_t descriptor;
    std::string queue_name;
    unsigned int default_prio = 10;
};

Conn *Conn::create(bool create, int conn_id) {
    std::string name = "/mq_wolf_and_goats_" + std::to_string(conn_id);

    return new ConnMq(name, create);
}
