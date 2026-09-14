#pragma once

#include <string>
#include <vector>

#include "command.h"
#include "converters.h"

class command_txtco : public command {
public:
    argument<std::string>              dir;            // -dir
    argument<bool>                     recursive;      // -recursive
    argument<std::vector<std::string>> exclude_dirs;   // -exclude_dir
    argument<std::vector<std::string>> exclude_files;  // -exclude_file
    argument<std::vector<std::string>> formats;        // -format（必需）
    argument<std::string>              output_path;    // -o
    argument<std::string>              output_encoding;// -o_code
    argument<bool>                     pasteboard;     // 实际键为-clipboard

    command_txtco();
    void operator()() override;
};