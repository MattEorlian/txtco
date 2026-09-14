#pragma once

#include <string>
#include <vector>
#include <functional>  
#include <unordered_map>

#include "my_ref_dict.h"
#include "internal\argument.h"

template<typename T> class argument : public argument_base
{

using converter_callable = std::function<int(T&,int argc,char* argv[])>;
public:
    T arg;//参数值
    T defaultArg;//默认值
private:
    converter_callable converter;//可执行的转换器，签名为int(*)(T&,int argc,char* argv[])。返回整数值为读取了多少个参数
public://构造函数
    argument(const T& default_arg,converter_callable Converter)
        :arg(default_arg),defaultArg(default_arg),converter(Converter)
    {
        
    }
public://功能函数
    int convert(int argc,char* argv[]) override//从argv中把字符串转换为参数
    {
        //注意，提供给convert的字符串必须已参数开头
        return converter(arg,argc,argv);
    }
    void restore_default()
    {
        arg = defaultArg;
    }
    T& operator*()
    {
        return arg;
    }
};

class command
{
public:
    const std::string key;//查询该命令所用关键字
    const std::string doc;//帮助字符串
    arg_dict dict;//参数键值对
public:
    command(const char Key[],const char Doc[]);//给key赋值的构造函数
public://父类成员函数
    
public://虚函数接口
    virtual void operator()() = 0;//命令执行函数
    void injectArgs(int argc,char* argv[]);//从argv中读取参数，要求argv就是命令行参数表(即去除文件路径、命令名后的部分)；argc为参数个数
};

class cmd_dict : public my::ref_dict<std::string,command>
{
public:
    void add(command& cmd)
    {
        pair(cmd.key,cmd);
    }
};