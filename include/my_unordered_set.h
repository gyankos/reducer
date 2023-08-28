//
// Created by giacomo on 20/08/23.
//

#ifndef REDUCER_MY_UNORDERED_SET_H
#define REDUCER_MY_UNORDERED_SET_H

#include <unordered_set>

template <typename T>
struct my_unordered_set {
    std::unordered_set<T> actual;

    my_unordered_set() {}
    my_unordered_set(std::initializer_list<T> l) {
        for (const auto& x : l)
            actual.emplace(x);
    }
    inline void clear() {
        actual.clear();
    }
    inline bool add(const T& val) {
        return actual.emplace(val).second;
    }
    inline bool contains(const T& val) const {
        return actual.contains(val);
    }
    inline size_t remove(const T& val) {
        return actual.erase(val);
    }
    inline size_t size() const {
        return actual.size();
    }
    inline bool empty() const {
        return actual.empty();
    }
    std::unordered_set<T>::iterator begin() {
        return actual.begin();
    }
    std::unordered_set<T>::iterator end() {
        return actual.end();
    }
    std::unordered_set<T>::const_iterator begin() const {
        return actual.begin();
    }
    std::unordered_set<T>::const_iterator end() const {
        return actual.end();
    }
    std::unordered_set<T>::iterator cbegin() const {
        return actual.cbegin();
    }
    std::unordered_set<T>::iterator cend() const {
        return actual.cend();
    }



    bool operator==(const my_unordered_set<T> &rhs) const {
        if (size() != rhs.size()) return false;
        for (const auto& x : *this)
            if (!rhs.contains(x))
                return false;
        return true;
    }
};

namespace std {
    template <typename T> struct hash<my_unordered_set<T>> {
    size_t operator()(const my_unordered_set<T>& x) const {
        if (x.size() == 0) return 0;
        else {
            size_t val = 1;
            for (const auto&item : x)
                val *= (std::hash<T>{}(item)+1);
            return val;
        }
    }
};
}

#endif //REDUCER_MY_UNORDERED_SET_H
