#include "conn.hpp"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <unistd.h>

class ConnShm : public Conn {
  public:
    ConnShm(bool create, const std::string &name) {
        shm_name = name;
        int fd = shm_open(shm_name.c_str(), O_RDWR | O_CREAT, 0666);
        if (fd == -1)
            throw std::runtime_error("Couldn't open shared memory file descriptor");

        if (create)
            if (ftruncate(fd, mem_size) == -1)
                throw std::runtime_error("Couldn't truncate file descriptor to given size");

        mem = mmap(NULL, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED)
            throw std::runtime_error("Couldn't allocate shared memory with mmap");
    }
    ~ConnShm() {
        if (munmap(mem, mem_size) == -1 || shm_unlink(shm_name.c_str()) == -1) {
            std::cout << "Couldn't free shared memory" << std::endl;
            std::exit(1);
        }
    }
    inline void read(Message &msg) override { std::memcpy(&msg, mem, sizeof(msg)); }
    inline void write(const Message &msg) override { std::memcpy(mem, &msg, sizeof(msg)); }

  private:
    std::string shm_name;
    void *mem;
    const size_t mem_size = 4096;
};

Conn *Conn::create(bool create, int conn_id) {
    std::string name = "/shm_wolf_and_goats" + std::to_string(conn_id);
    return new ConnShm(create, name);
}