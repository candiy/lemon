#include <iostream>

#include <boost/filesystem.hpp>

#include <chemfiles.hpp>

#include "lemon/entries.hpp"
#include "lemon/parse.hpp"
#include "lemon/run.hpp"

using namespace boost::filesystem;

int main(int argc, char* argv[]) {
    path entries(argc > 1 ? argv[1] : "entries.idx");
    path p(argc > 2 ? argv[2] : ".");
    size_t ncpu = argc > 3 ? std::strtoul(argv[3], nullptr, 0) : 1;
    size_t chun = argc > 4 ? std::strtoul(argv[4], nullptr, 0) : 1;

    if (!is_regular_file(entries)) {
        std::cerr << "You must supply a valid entries file" << std::endl;
        return 1;
    }

    if (!is_directory(p)) {
        std::cerr << "You must supply a valid directory" << std::endl;
        return 2;
    }

    std::vector<std::array<char, 4>> vec;
    lemon::read_entry_file(entries.string(), vec);

    std::unordered_map<std::thread::id, lemon::ResidueNameCount>
        resn_counts;
    auto worker = [&resn_counts](const chemfiles::Frame& complex,
                                 const std::string& pdbid) {

        // Desired info is calculated directly, no pruning, output is done
        auto th = std::this_thread::get_id();
        lemon::count_residues(complex, resn_counts[th]);
    };

    current_path(p);
    lemon::call_multithreaded(worker, vec, ncpu, chun);

    lemon::ResidueNameCount resn_total;

    for (const auto& resn_count : resn_counts) {
        resn_total += resn_count.second;
    }

    std::cout << resn_total;
}
