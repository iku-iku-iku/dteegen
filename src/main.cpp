#include "pch.h"
// pch above
#include <filesystem>

#include "convertor.h"
#include "creator.h"

void convert(std::string project_path) { generate_secgear(project_path); }
// 主函数
int main(int argc, char **argv)
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [create]/[convert]"
                  << " [project_path]\n";
        return 1;
    }

    // measure time
    auto start = std::chrono::high_resolution_clock::now();
    if (!strcmp(argv[1], "create")) {
        const auto creator = ProjectCreator(argv[2]);
        creator();
    }
    else if (!strcmp(argv[1], "convert")) {
        // default to secgear convertor now
        const auto convertor = SecGearConvertor(argv[2]);
        convertor();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    std::cout << "Time: " << time << "ms" << std::endl;
    return 0;
}
