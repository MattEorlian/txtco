#include<iostream>
#include<exception>
#include<string>

#include "command.h"
#include "command_help.h"

command_help::command_help(const cmd_dict& obj)
    : command("help",
              "Show help for all commands or a specific one"),
      dst_cmd_dict(obj),
      help_which("all",
          [](std::string& dst, int argc, char* argv[]) -> int {
              if (argc != 1)
                  throw std::invalid_argument("help is intended to take only one argument.");
              dst = argv[0];
              return 1;
          },
          "Which command to show help for, or \"all\" (default: all)")
{
    dict.pair("-which", help_which);
}

void command_help::operator()() {
    const std::string& which = help_which.arg;

    // 打印单个命令的帮助
    auto print_one = [](const std::string& key, const command& cmd) {
        std::cout << "Command: " << key << "\n"
                  << "  " << cmd.doc << "\n";

        if (cmd.dict.begin() == cmd.dict.end()) {
            std::cout << "  (no arguments)\n\n";
            return;
        }

        std::cout << "  Arguments:\n";
        for (auto ait = cmd.dict.begin(); ait != cmd.dict.end(); ++ait) {
            std::cout << "    " << ait->first << " : " << ait->second->doc << "\n";
        }
        std::cout << "\n";
    };

    if (which == "all") {
        for (auto it = dst_cmd_dict.begin(); it != dst_cmd_dict.end(); ++it) {
            print_one(it->first, *(it->second));
        }
        return;
    }

    // 只输出指定命令
    if (!dst_cmd_dict.findKey(which)) {
        throw std::invalid_argument(
            std::string("No such command: ") + which + ". Try \"-which all\"."
        );
    }

    print_one(which, dst_cmd_dict[which]);
}