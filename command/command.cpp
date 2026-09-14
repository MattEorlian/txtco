#include<iostream>
#include<string>
#include<stdexcept>
#include<exception>
#include<iterator>

#include "command.h"
#include "internal\argument.h"

command::command(const char Key[],const char Doc[]) : key(Key),doc(Doc){}

void command::injectArgs(int argc,char* argv[])//从argv中读取参数，要求argv就是命令行参数表as is；argc为参数个数
{
    if(argc < 0) throw std::invalid_argument("Expect argc >= 0.");
    else if(argc == 0) return;
    if(argv == nullptr) throw std::invalid_argument("Expect argv != nullptr.");
    int cur = 0;
    try{
        while(cur < argc)
        {
            if(dict.findKey(argv[cur]))//如果该字符串是参数键值，则调用转换函数写入参数
            {
                int cnt = 0;
                cnt = dict[argv[cur]].convert(argc - cur - 1, argv + cur + 1);
                cur += cnt + 1;
            }
            else
            {
                throw std::invalid_argument(
                    std::string("Failed injecting arguments. No argument called ") + argv[cur] + "."
                );
            }
        }
    }catch(const std::exception& e)
    {
        dict.restore_default();
        throw;
    }
}