#include "file.h"

#include <filesystem>
#include <fstream>
#include <regex>

extern "C" int write_file(in_char* in_filename, int in_filename_len, in_char* in_content, int in_content_len)
{
    std::string filename = in_filename;
    std::ofstream ofs(filename);
    ofs.write(in_content,in_content_len);
    return 0;
}

extern "C" int get_emb_list(char out_list[sizeof(int) * MAX_EMB_CNT])
{
    int cnt = 0;
    int* p_list = (int*)out_list;

    for (const auto& e : std::filesystem::directory_iterator(".")) {
        const auto& path = e.path();
        const auto path_str = path.string();
        // if path_str belike "emb*.bin", take its id
        std::regex re("emb(\\d+)\\.bin");
        if (std::regex_search(path_str, re)) {
            std::smatch match;
            std::regex_search(path_str, match, re);
            int id = std::stoi(match[1]);
            p_list[cnt++] = id;

            if (cnt >= MAX_EMB_CNT) {
                break;
            }
        }
    }
    return cnt;
}

extern "C" int read_file(char* in_filename, int in_filename_len, char* out_content, int out_content_len) {
	std::ifstream ifs(std::string(in_filename, in_filename + in_filename_len), std::ios::binary);
	ifs.read(out_content, out_content_len);
	return 0;
}