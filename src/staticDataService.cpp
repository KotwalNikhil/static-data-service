#include <filesystem>
#include "staticDataService.hpp"

using namespace std;

int main() {

    std::string directory_path = "/home/nikhilsingh/work/cpp/multiThreading/static_data";
    std::vector<string> files;

    for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
        if (entry.is_regular_file()) {
            files.emplace_back(entry.path());
        }
    }

    std::string output_filePath("/home/nikhilsingh/work/cpp/multiThreading/merged_data.txt");

    mergeMultipleFiles(files, output_filePath);

    return 0;
}
