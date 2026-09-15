#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "my_ref_dict.h"

class argument_base//为类型擦除服务的argument_base。
{
public:
    const std::string doc;//文档字符串
public:
    argument_base(const char* Doc);
public:
    virtual int convert(int argc,char* argv[]) = 0;
    virtual void restore_default() = 0;
};

class arg_dict : public my::ref_dict<std::string,argument_base>
{
public:
    void restore_default();//所以参数设为默认值
};