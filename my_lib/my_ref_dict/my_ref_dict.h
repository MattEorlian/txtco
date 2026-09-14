#pragma once

#include<unordered_map>
#include<exception>
#include<stdexcept>

namespace my{
template<typename T_Key,typename T_Val> class ref_dict//可以存储引用（实际上是指针的）字典
{
protected:
    std::unordered_map<T_Key,T_Val*> dict;
public:
    void pair(const T_Key& key,T_Val& val)
    {
        dict[key] = &val;
    }
    bool findKey(const T_Key& key) const
    {
        return dict.count(key);
    }
    T_Val& operator[](const T_Key& key)
    {
        T_Val* ptr = dict[key];
        if(ptr == nullptr) throw std::invalid_argument("Cannot find argument. Expect a valid key string.");
        return *ptr;
    }
};
}