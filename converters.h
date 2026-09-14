#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>

namespace conv {

inline int to_bool(bool& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected true/false");
    std::string s = argv[0];
    if (s == "true" || s == "1")       dst = true;
    else if (s == "false" || s == "0") dst = false;
    else throw std::invalid_argument("expected true/false, got: " + s);
    return 1;
}

inline int to_string(std::string& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected a string");
    dst = argv[0];
    return 1;
}

// ✅ 贪婪消费：读到下一个以 '-' 开头的 token 为止，至少消费 1 个
inline int append_string(std::vector<std::string>& dst, int argc, char* argv[]) {
    int consumed = 0;
    while (consumed < argc) {
        std::string s = argv[consumed];
        if (!s.empty() && s[0] == '-') break;   // 遇到下一个键，停止
        dst.push_back(s);
        ++consumed;
    }
    if (consumed == 0)
        throw std::invalid_argument("expected at least one value");
    return consumed;
}

inline int to_encoding(std::string& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected an encoding name");
    std::string s = argv[0];
    if (s != "UTF-8" && s != "GBK" && s != "UTF-16")
        throw std::invalid_argument("unsupported encoding: " + s);
    dst = s;
    return 1;
}

inline int to_existing_dir(std::string& dst, int argc, char* argv[]) {
    if (argc < 1) throw std::invalid_argument("expected a directory path");
    namespace fs = std::filesystem;
    if (!fs::exists(argv[0]) || !fs::is_directory(argv[0]))
        throw std::invalid_argument(std::string("not a directory: ") + argv[0]);
    dst = argv[0];
    return 1;
}

} // namespace conv