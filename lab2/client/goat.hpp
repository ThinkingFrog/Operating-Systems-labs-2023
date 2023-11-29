#pragma once
#include <cstdlib>
#include <random>

class Goat {
  public:
    inline size_t roll() { return is_alive ? ui_dist_alive(rng) : ui_dist_dead(rng); }
    inline void set_status(bool alive) { is_alive = alive; }
    inline bool alive() { return is_alive; }

  private:
    bool is_alive = true;
    const size_t GOAT_MIN_ROLL = 1;
    const size_t GOAT_ALIVE_MAX_ROLL = 100;
    const size_t GOAT_DEAD_MAX_ROLL = 50;

    std::uniform_int_distribution<std::mt19937::result_type> ui_dist_alive =
        std::uniform_int_distribution<std::mt19937::result_type>(GOAT_MIN_ROLL, GOAT_ALIVE_MAX_ROLL);
    std::uniform_int_distribution<std::mt19937::result_type> ui_dist_dead =
        std::uniform_int_distribution<std::mt19937::result_type>(GOAT_MIN_ROLL, GOAT_DEAD_MAX_ROLL);
    std::random_device rd;
    std::mt19937 rng = std::mt19937(rd());
};