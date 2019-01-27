#pragma once
#include <map>
#include <mutex>

template<typename KeyType,typename ValueType>
class ManagerTemplate {
public:
    virtual bool AddData(
        const KeyType& in_key,
        const ValueType& in_value) {
        auto it_find = map_.find(in_key);
        if (it_find != map_.end()) {
            return false;
        }
        map_[in_key] = in_value;
        return true;
    }

    virtual void DeleteKey(
        const KeyType& in_key) {
        auto it_find = map_.find(in_key);
        if (it_find == map_.end()) {
            return;
        }
        map_.erase(in_key);
    }

    virtual bool ModData(
        const KeyType& in_key,
        const ValueType& in_value) {
        auto it_find = map_.find(in_key);
        if (it_find == map_.end()) {
            return false;
        }
        map_[in_key] = in_value;
        return true;
    }

    virtual bool HaveKey(
        const KeyType& in_key) {
        auto it_find = map_.find(in_key);
        return it_find != map_.end();
    }

protected:
    std::map<KeyType, ValueType> map_;
};