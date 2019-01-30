#pragma once
#include <map>
#include <mutex>
#include <memory>

namespace top
{
template<typename KeyType,typename ValueType>
class ManagerTemplate {
public:
    ManagerTemplate() {}
    virtual ~ManagerTemplate() {}
    virtual bool AddData(
        const KeyType& in_key,
        const ValueType& in_value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it_find = map_.find(in_key);
        if (it_find != map_.end()) {
            return false;
        }
        map_[in_key] = in_value;
        return true;
    }

    virtual void DeleteKey(
        const KeyType& in_key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it_find = map_.find(in_key);
        if (it_find == map_.end()) {
            return;
        }
        map_.erase(in_key);
    }

    virtual bool ModData(
        const KeyType& in_key,
        const ValueType& in_value) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it_find = map_.find(in_key);
        if (it_find == map_.end()) {
            return false;
        }
        map_[in_key] = in_value;
        return true;
    }

    virtual bool HaveKey(
        const KeyType& in_key) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it_find = map_.find(in_key);
        return it_find != map_.end();
    }
    virtual bool FindData(
        const KeyType& in_key,
        ValueType& out_value) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it_find = map_.find(in_key);
        if (it_find == map_.end()) {
            return false;
        }
        out_value = it_find->second;
        return true;
    }

protected:
    mutable std::mutex mutex_;
    std::map<KeyType, ValueType> map_;
};
}  //  namespace top