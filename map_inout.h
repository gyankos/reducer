#pragma once
#define DEBUG
#include <unordered_set>
#include <unordered_map>
#include <yaucl/functional/assert.h>

template <typename T, typename V> struct map_inout {
    std::unordered_map<T, std::unordered_set<V>> out;
    std::unordered_map<V, std::unordered_set<T>> in;

    inline std::unordered_set<V>& add(const T& left, const V& right) {
        auto& ptr = out[left];
        ptr.emplace(right);
        in[right].emplace(left);
        return ptr;
    }

    inline std::unordered_set<V>& add(std::unordered_map<T, std::unordered_set<V>>::iterator& it, const T& left, const V& right) {
        if (it == end_out()) {
            return add(left, right);
        } else {
            it->second.emplace(right);
            return it->second;
        }
    }

    std::unordered_map<T, std::unordered_set<V>>::iterator erase_out(std::unordered_map<T, std::unordered_set<V>>::iterator& it) {
        if (it != out.end()) {
            for (const V& v : it->second) {
                auto it2 = in.find(v);
                if (it2 != in.end()) {
                    it2->second.erase(it->first);
                    if (it2->second.empty())
                        in.erase(it2);
                }
            }
            return out.erase(it);
        } else
            return out.end();
    }
    bool eraseFirst(const T& first) {
        auto it = find_out(first);
        if (it != end_out()) {
            auto cp = it->second;
            for (const auto& v : cp) {
                auto it2 = find_out(v);
                DEBUG_ASSERT(it2 != end_out());
                it2->second.erase(first);
            }
            out.erase(it);
        }
        return false;
    }

    bool erase(const T& first, const V& second) {
        auto it = find_out(first);
        if (it != end_out()) {
            if (it->second.erase(second) == 1) {
                auto it2 = find_in(second);
                if (it2 != end_in()) {
                    DEBUG_ASSERT(it2->second.erase(first) == 1);
                    if (it2->second.empty())
                        in.erase(it2);
                    if (it->second.empty())
                        out.erase(it);
                    return true;
                }
            }
        }
        return false;
    }

    bool contains(const T& first, const V& second) {
        auto it = find_out(first);
        if (it != end_out()) {
            if (it->second.contains(second)) {
                return true;
            }
        }
        return false;
    }




//    std::unordered_map<T, std::unordered_set<V>>::iterator begin() {
//        return out.begin();
//    }
//
//    std::unordered_map<T, std::unordered_set<V>>::iterator end() {
//        return out.end();
//    }

    std::unordered_map<T, std::unordered_set<V>>::iterator find_out(const T& left) {
        return out.find(left);
    }

    std::unordered_map<V, std::unordered_set<T>>::iterator find_in(const V& right) {
        return in.find(right);
    }

    std::unordered_map<V, std::unordered_set<T>>::iterator end_in() {
        return in.end();
    }

    std::unordered_map<T, std::unordered_set<V>>::iterator begin() {
        return out.begin();
    }

    std::unordered_map<T, std::unordered_set<V>>::iterator end() {
        return out.end();
    }

    std::unordered_map<T, std::unordered_set<V>>::iterator begin_out() {
        return out.begin();
    }

    std::unordered_map<T, std::unordered_set<V>>::iterator end_out() {
        return out.end();
    }

    bool empty() const {
        return out.empty();
    }

    void clear() {
        out.clear();
        in.clear();
    }
};