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
using act_t = size_t;

#define BINARY_PRESENCE     (0b01)
#define BINARY_ABSENCE    (0b10)
#define BINARY_EXCL     (0b11)

template <typename T>
static inline void exclude_from_map(map_inout<act_t, T>& map, const act_t& elem) {
    auto it = map.find_out(elem);
    if (it != map.end_out()) {
        map.erase_out(it);
    }
}

static inline bool exclude_from_precedence_map(map_inout<act_t, act_t>& map,
                                               const act_t& elem,
                                               std::unordered_map<act_t, char>& Future) {
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
    std::unordered_map<act_t, char> Future;
    std::vector<std::pair<act_t,act_t>> not_coexistence, chain_response, response, choice, neg_succession, resp_existence, chain_precedence, alt_response;

    map_inout<act_t, std::pair<is_negated, act_t>> MFuture;
    map_inout<act_t, act_t> Malt_response,
            Mprecedence,
            MNext,
            Mneg_chainsuccession,
            Malt_precedence,
            Mresp_existence,
            Mchoice,
            Mnot_coex;

    //,
//            MResp_exsistence;

    inline bool exclude_from_existance(const act_t& cf) { // SR1
        auto it2 = Future.emplace(cf, BINARY_ABSENCE);
        if ((!it2.second) && ((it2.first->second & BINARY_ABSENCE) != BINARY_ABSENCE)) {
            return false;  /* Inconsistent model, as both clauses coexist */
        }
        return true;
    }

    inline bool allow_existance(const act_t& cf) { // SR1
        auto it = Future.emplace(cf, BINARY_PRESENCE);
        if ((!it.second) && ((it.first->second & BINARY_PRESENCE) != BINARY_PRESENCE)) {
            return false;  // Inconsistent model, as both clauses coexist
        }
        return true;
    }

    inline bool exclude_from_all_maps(const act_t& cf,
                                      bool malt=true) {
        exclude_from_map(MNext, cf);
        if (malt) exclude_from_map(Malt_response, cf);
        exclude_from_map(Mneg_chainsuccession, cf);
        exclude_from_map(MNext, cf);
        exclude_from_map(MFuture, cf);
        exclude_from_map(Mresp_existence, cf);
        auto excludeChoiceLeft = Mchoice.eraseFirst(cf, true);
        auto excludeChoiceRight = Mchoice.eraseSecond(cf, true);
        for (const auto& x : excludeChoiceLeft)
            if (!allow_existance(x)) return false;
        for (const auto& x : excludeChoiceRight)
            if (!allow_existance(x)) return false;
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
    bool expand_forward_re(const act_t& presentLabel);
    bool reduce_map_to_be_considered(map_inout<act_t, act_t>& to_reduce,  bool alsoPassedMap = false);
    std::vector<DatalessCases> run(const std::vector<DatalessCases>& model);
    bool reduce_cr(map_inout<act_t, act_t>& MN,
                   const act_t& label);
    bool reduce_r(const act_t& label);
    bool reduce_c(const act_t& absentLabel);
    bool reduce_p(const act_t& absentLabel);
};



#endif //REDUCER_MODEL_REDUCER_H
