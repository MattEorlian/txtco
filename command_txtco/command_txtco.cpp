#include "command_txtco.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <cctype>

#include "clipboard.h"

namespace fs = std::filesystem;

// 扩展名统一小写
static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

command_txtco::command_txtco()
    : command("txtco", "Collect text files from one or more directories"),
      dir({"."}, conv::append_existing_dir,
          "Root directories to scan (multi-value, default: current directory)"),
      recursive(false, conv::to_bool,
          "Recurse into subdirectories, true/false (default: false)"),
      exclude_dirs({}, conv::append_string,
          "Directories to skip, relative to the first -dir (multi-value, optional)"),
      exclude_files({}, conv::append_string,
          "Files to skip, relative to the first -dir (multi-value, optional)"),
      formats({}, conv::append_string,
          "File extensions to collect, e.g. .cpp .h (required, multi-value)"),
      output_path(".", conv::to_string,
          "Output directory (default: current directory)"),
      output_encoding("UTF-8", conv::to_encoding,
          "Output encoding: UTF-8 / GBK / UTF-16 (default: UTF-8)"),
      pasteboard(false, conv::to_bool,
          "Copy result to clipboard instead of writing a file, true/false (default: false)"),
      keyword({}, conv::append_string,
          "Only collect files whose name (with extension) contains any of these substrings (multi-value, optional)")
{
    dict.pair("-dir",          dir);
    dict.pair("-recursive",    recursive);
    dict.pair("-exclude_dir",  exclude_dirs);
    dict.pair("-exclude_file", exclude_files);
    dict.pair("-format",       formats);
    dict.pair("-o",            output_path);
    dict.pair("-o_code",       output_encoding);
    dict.pair("-clipboard",    pasteboard);
    dict.pair("-keyword",      keyword);
}

void command_txtco::operator()() {
    // ---------- 1. 校验必需参数 ----------
    if (formats.arg.empty())
        throw std::runtime_error("-format is required (at least one)");

    if (dir.arg.empty())
        throw std::runtime_error("-dir must have at least one value");

    // ---------- 2. 构建查找表 ----------
    // 相对路径以第一个 -dir 为基准
    const fs::path base_dir = dir.arg.front();

    auto resolve_path = [&](const std::string& p) -> std::string {
        fs::path path(p);
        if (path.is_relative())
            path = base_dir / path;
        return fs::absolute(path).lexically_normal().string();
    };

    std::unordered_set<std::string> ex_dirs, ex_files, fmts;
    for (auto& d : exclude_dirs.arg)
        ex_dirs.insert(resolve_path(d));
    for (auto& f : exclude_files.arg)
        ex_files.insert(resolve_path(f));
    for (auto& e : formats.arg)
        fmts.insert(to_lower(e));

    // ---------- 3. 遍历并收集 ----------
    std::string result;
    int file_count = 0;
    std::unordered_set<std::string> visited_files;   // 文件级去重

    auto process_file = [&](const fs::path& p) {
        std::string abs = fs::absolute(p).lexically_normal().string();
        if (visited_files.count(abs)) return;        // 已收集过
        if (ex_files.count(abs)) return;

        // 扩展名忽略大小写匹配
        std::string ext = to_lower(p.extension().string());
        if (!fmts.count(ext)) return;

        // keyword 过滤：文件名（含后缀）包含任一 keyword 即通过
        if (!keyword.arg.empty()) {
            std::string fname = p.filename().string();
            bool matched = false;
            for (const auto& kw : keyword.arg) {
                if (fname.find(kw) != std::string::npos) {
                    matched = true;
                    break;
                }
            }
            if (!matched) return;
        }

        std::ifstream ifs(p, std::ios::binary);
        if (!ifs) {
            std::cerr << "Warning: cannot open " << abs << "\n";
            return;
        }
        std::string content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());

        result += "[" + abs + "]\n";
        result += content;
        if (!content.empty() && content.back() != '\n') result += '\n';
        result += "\n";

        visited_files.insert(abs);
        ++file_count;
    };

    std::function<void(const fs::path&)> walk = [&](const fs::path& d) {
        if (!fs::exists(d) || !fs::is_directory(d))
            throw std::runtime_error("Not a directory: " + d.string());

        for (auto& entry : fs::directory_iterator(d)) {
            std::string abs = fs::absolute(entry.path()).lexically_normal().string();
            if (entry.is_directory()) {
                if (ex_dirs.count(abs)) continue;
                if (recursive.arg) walk(entry.path());
            } else if (entry.is_regular_file()) {
                process_file(entry.path());
            }
        }
    };

    for (const auto& d : dir.arg) {
        walk(d);
    }

    std::cout << "Collected " << file_count << " files, "
              << result.size() << " bytes.\n";

    // ---------- 4. 输出 ----------
    if (pasteboard.arg) {
        if (!copy_to_clipboard(result))
            std::cerr << "Warning: failed to copy to clipboard.\n";
        else
            std::cout << "Copied to clipboard.\n";
        return;
    }

    // 生成输出文件名：txtco_output_YYYYMMDD_HHMMSS.txt
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm);
    std::string out_name = "txtco_output_" + std::string(buf) + ".txt";

    fs::path out_path = fs::path(output_path.arg) / out_name;
    std::ofstream ofs(out_path, std::ios::binary);
    if (!ofs) throw std::runtime_error("Cannot create: " + out_path.string());

    // UTF-8 带 BOM，方便 Windows 记事本识别
    if (output_encoding.arg == "UTF-8")
        ofs.write("\xEF\xBB\xBF", 3);

    ofs << result;
    std::cout << "Output: " << out_path << "\n";
}