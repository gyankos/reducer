//
// Created by giacomo on 22/08/23.
//

#ifndef REDUCER_MODEL_REDUCER_H
#define REDUCER_MODEL_REDUCER_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "declare_cases.h"
using is_negated = bool;

struct model_reducer {
    std::unordered_map<std::string, char> Future;
    std::vector<std::pair<std::string,std::string>> not_coexistence, chain_response, response, choice, neg_succession, resp_existence, chain_precedence;
    std::unordered_map<std::string, std::set<std::pair<is_negated, std::string>>> MFuture;
    std::unordered_map<std::string, std::unordered_set<std::string>> Malt_response,
            Mprecedence,
            MNext,
            Mneg_chainsuccession,
            Malt_precedence;//,
//            MResp_exsistence;

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
    bool reduce_map_to_be_considered(std::unordered_map<std::string, std::unordered_set<std::string>>& to_reduce);
    std::vector<DatalessCases> run(const std::vector<DatalessCases>& model);
    bool reduce_cr(std::unordered_map<std::string, std::unordered_set<std::string>>& MN,
                   const std::string& label);
};



#endif //REDUCER_MODEL_REDUCER_H
