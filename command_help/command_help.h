#pragma once

#include "command.h"

class command_help : public command
{
public:
    const cmd_dict& dst_cmd_dict;//对该cmd_dict提供帮助
    argument<std::string> help_which;
public:
    command_help(const cmd_dict& dict_obj);
    void operator()() override;
};