//
// Created by giacomo on 19/08/23.
//

#ifndef REDUCER_DECLARE_CASES_H
#define REDUCER_DECLARE_CASES_H

enum declare_cases{
    Existence,
    Absence,
    Choice,
    ExclChoice,
    RespExistence,
    CoExistence,
    Response,
    Precedence,
    ChainResponse,
    ChainPrecedence,
    ChainSuccession,
    Succession,
    AltPrecedence,
    AltSuccession,
    AltResponse,
    NegSuccession,
    NegChainSuccession,
    NotCoexistence
};

#include <string>

struct DatalessCases {
    declare_cases casus;
    std::string left, right;

    DatalessCases() {}
    DatalessCases(declare_cases casus, const std::string &left, const std::string &right);
    DatalessCases(const std::string& line);
    DatalessCases(const DatalessCases&  ) = default;
    DatalessCases(DatalessCases&&  ) = default;
    DatalessCases& operator=(const DatalessCases&  ) = default;
    DatalessCases& operator=(DatalessCases&&  ) = default;
    bool operator==(const DatalessCases& rhs) {
        return casus == rhs.casus && left == rhs.left && right == rhs.right;
    }
    friend std::ostream& operator<<(std::ostream& os, const struct DatalessCases& d);
};

#include <iostream>
#include <sstream>
#include <vector>

static inline std::vector<DatalessCases> streamDeclare(std::istream& is) {
    std::string line;
    std::vector<DatalessCases> result;
    while (std::getline(is, line)) {
        result.emplace_back(line);
    }
    return result;
}

#include <list>

static inline void expand_list(std::list<DatalessCases>& list) {
    for (auto it = list.begin(); it != list.end(); it++) {
        switch (it->casus) {
            case Choice:
            case ExclChoice:
                if (it->right < it->left)
                    std::swap(it->right, it->left);
                break;
            case CoExistence:
                it->casus = RespExistence;
                it = list.emplace(it, RespExistence, it->right, it->left);
                break;
            case ChainSuccession:
                it->casus = ChainResponse;
                it = list.emplace(it, ChainPrecedence, it->right, it->left);
                break;
            case Succession:
                it->casus = Response;
                it = list.emplace(it, Precedence, it->left, it->right);
                break;
            case AltSuccession:
                it->casus = AltPrecedence;
                it = list.emplace(it, AltResponse, it->left, it->right);
                break;
            case AltPrecedence:
            case AltResponse:
            case Existence:
            case Absence:
            case RespExistence:
            case NegSuccession:
            case NegChainSuccession:
            case Response:
            case Precedence:
            case ChainResponse:
            case ChainPrecedence:
                break;
        }
    }
}

#endif //REDUCER_DECLARE_CASES_H
