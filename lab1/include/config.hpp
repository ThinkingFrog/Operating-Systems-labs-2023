#include <filesystem>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

class Config {
  public:
    Config() = default;
    explicit Config(const std::string &filepath) { read(filepath); }

    void read(const std::string &filepath) {
        entries.clear();

        std::ifstream f(filepath);
        std::string dir1, dir2, ext;

        while (f >> dir1 >> dir2 >> ext) {
            entries.emplace_back(fs::path(dir1), fs::path(dir2), ext);
        }
    }

    inline const std::vector<std::tuple<fs::path, fs::path, std::string>> &get_entries() const { return entries; }

  private:
    std::vector<std::tuple<fs::path, fs::path, std::string>> entries;
};
