#pragma once

#include <unordered_set>
#include <unordered_map>

template <typename T, typename V> struct map_inout {
    std::unordered_map<T, std::unordered_set<V>> out;
    std::unordered_map<V, std::unordered_set<T>> in;

    std::unordered_set<V>& add(const T& left, const V& right) {
        auto& ptr = out[left];
        ptr.emplace(right);
        in[right].emplace(left);
        return ptr;
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