#pragma once
#include <cstdlib>
#include <random>

class Wolf {
  public:
    inline size_t roll() { return ui_dist(rng); }

  private:
    const size_t WOLF_MIN_ROLL = 1;
    const size_t WOLF_MAX_ROLL = 100;

    std::uniform_int_distribution<std::mt19937::result_type> ui_dist =
        std::uniform_int_distribution<std::mt19937::result_type>(WOLF_MIN_ROLL, WOLF_MAX_ROLL);
    std::random_device rd;
    std::mt19937 rng = std::mt19937(rd());
};