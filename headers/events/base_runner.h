#pragma once
#ifndef H_MLN_DB_BASE_RUNNER_H
#define H_MLN_DB_BASE_RUNNER_H

#include <map>
#include <functional>

class base_runner{
    public:
        template<typename T_Key, class B, typename K>
        static bool run(const std::map<T_Key, std::function<void(B&, const K&)>>& events, const T_Key& key, B& arg, const K& event_data){
            bool result = events.contains(key);
            if(result){
                events.at(key)(arg, event_data);
            }
            return result;
        }
};

#endif