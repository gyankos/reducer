//
// Created by giacomo on 22/08/23.
//

#ifndef REDUCER_MODEL_REDUCER_H
#define REDUCER_MODEL_REDUCER_H

#include <yaucl/hashing/pair_hash.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "declare_cases.h"
#include "map_inout.h"
using is_negated = bool;


#define BINARY_PRESENCE     (0b01)
#define BINARY_ABSENCE    (0b10)
#define BINARY_EXCL     (0b11)

template <typename T>
static inline void exclude_from_map(map_inout<std::string, T>& map, const std::string& elem) {
    auto it = map.find_out(elem);
    if (it != map.end_out()) {
        map.erase_out(it);
    }
}

static inline bool exclude_from_precedence_map(map_inout<std::string, std::string>& map,
                                               const std::string& elem,
                                               std::unordered_map<std::string, char>& Future) {
    auto it = map.find_out(elem);
    if (it != map.end_out()) {
        for (const auto& b : it->second) {
            auto it2 = Future.emplace(b, BINARY_ABSENCE);
            if ((!it2.second) && ((it2.first->second & BINARY_ABSENCE) != BINARY_ABSENCE))
                return true;
        }
        map.erase_out(it);
    }
    return false;
}

struct model_reducer {
    std::unordered_map<std::string, char> Future;
    std::vector<std::pair<std::string,std::string>> not_coexistence, chain_response, response, choice, neg_succession, resp_existence, chain_precedence, alt_response;

    map_inout<std::string, std::pair<is_negated, std::string>> MFuture;
    map_inout<std::string, std::string> Malt_response,
            Mprecedence,
            MNext,
            Mneg_chainsuccession,
            Malt_precedence,
            Mresp_existence,
            Mchoice;//,
//            MResp_exsistence;

    inline bool exclude_from_existance(const std::string& cf) { // SR1
        auto it2 = Future.emplace(cf, BINARY_ABSENCE);
        if ((!it2.second) && ((it2.first->second & BINARY_ABSENCE) != BINARY_ABSENCE)) {
            return false;  /* Inconsistent model, as both clauses coexist */
        }
        return true;
    }

    inline bool allow_existance(const std::string& cf) { // SR1
        auto it = Future.emplace(cf, BINARY_PRESENCE);
        if ((!it.second) && ((it.first->second & BINARY_PRESENCE) != BINARY_PRESENCE)) {
            return false;  // Inconsistent model, as both clauses coexist
        }
        return true;
    }

    inline bool exclude_from_all_maps(const std::string& cf, bool malt=true) {
        exclude_from_map(MNext, cf);
        if (malt) exclude_from_map(Malt_response, cf);
        exclude_from_map(Mneg_chainsuccession, cf);
        exclude_from_map(MNext, cf);
        exclude_from_map(MFuture, cf);
        exclude_from_map(Mresp_existence, cf);
        return (!exclude_from_precedence_map(Mprecedence, cf, Future));
    }

    void clear() {
        Future.clear();
        MFuture.clear();
        not_coexistence.clear();
        chain_response.clear();
        response.clear();
        choice.clear();
        neg_succession.clear();
        resp_existence.clear();
        chain_precedence.clear();
        Malt_response.clear();
    }
    bool reduce_forward_re(const std::string& absentLabel);
    bool reduce_map_to_be_considered(map_inout<std::string, std::string>& to_reduce,  bool alsoPassedMap = false);
    std::vector<DatalessCases> run(const std::vector<DatalessCases>& model);
    bool reduce_cr(map_inout<std::string, std::string>& MN,
                   const std::string& label);
    bool reduce_r(const std::string& label);
};



#endif //REDUCER_MODEL_REDUCER_H
