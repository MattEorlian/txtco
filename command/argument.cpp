#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <iterator>

#include "internal\argument.h"

void arg_dict::restore_default()
{
    for(auto it = dict.begin();it != dict.end();++it)
    {
        it->second->restore_default();
    }
}