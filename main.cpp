#include<iostream>
#include<exception>
#include<string>
#include<unordered_map>

#include "command.h"
#include "command_txtco.h"
#include "command_help.h"

int main(int argc, char* argv[]) {
    cmd_dict dict;
    command_txtco txtco;
    command_help help(dict);
    dict.add(txtco);
    dict.add(help);

    if (argc < 2) {
        std::cerr << "Usage: txtco <command> [args...]\n";
        return 1;
    }

    try {
        command& cmd = dict[argv[1]];
        cmd.injectArgs(argc - 2, argv + 2);
        cmd();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}