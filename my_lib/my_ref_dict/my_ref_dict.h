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
    ref_dict(){};
    ref_dict(const ref_dict& other) = delete;
    ref_dict& operator=(const ref_dict& other) = delete;
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
        auto it = dict.find(key);
        if (it == dict.end() || it->second == nullptr)
            throw std::invalid_argument("Cannot find key.");
        return *(it->second);
    }
    const T_Val& operator[](const T_Key& key) const
    {
        auto it = dict.find(key);
        if (it == dict.end() || it->second == nullptr)
            throw std::invalid_argument("Cannot find key.");
        return *(it->second);
    }
    auto begin()
    {
        return dict.begin();
    }

    auto end()
    {
        return dict.end();
    }

    auto begin() const
    {
        return dict.begin();
    }

    auto end() const
    {
        return dict.end();
    }
};
}