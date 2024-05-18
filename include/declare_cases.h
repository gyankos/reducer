/*
 * declare_cases.h
 * This file is part of DECLAREd
 *
 * Copyright (C) 2023 - Giacomo Bergami
 *
 * DECLAREd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DECLAREd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DECLAREd. If not, see <http://www.gnu.org/licenses/>.
 */
//
// Created by Anonymous Ⅳ on 19/08/23.
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
    NotCoexistence,
    TRUTH
};

#include <string>
#include <yaucl/structures/any_to_uint_bimap.h>

struct DatalessCases {
    declare_cases casus;
    size_t left, right;

    DatalessCases() {}
    DatalessCases(declare_cases casus, const size_t &left, const size_t &right);
    DatalessCases(const std::string& line, yaucl::structures::any_to_uint_bimap<std::string>& bijection);
    DatalessCases(const DatalessCases&  ) = default;
    DatalessCases(DatalessCases&&  ) = default;
    DatalessCases& operator=(const DatalessCases&  ) = default;
    DatalessCases& operator=(DatalessCases&&  ) = default;
    bool operator==(const DatalessCases& rhs) {
        return casus == rhs.casus && left == rhs.left && right == rhs.right;
    }
};

#include <iostream>
#include <sstream>
#include <vector>

static inline std::vector<DatalessCases> streamDeclare(std::istream& is, yaucl::structures::any_to_uint_bimap<std::string>& bijection) {
    std::string line;
    std::vector<DatalessCases> result;
    while (std::getline(is, line)) {
        result.emplace_back(line, bijection);
    }
    return result;
}

struct DeclareStraightforwardPrinter {
    const yaucl::structures::any_to_uint_bimap<std::string>& bijection;
    const DatalessCases* to_print = nullptr;

    DeclareStraightforwardPrinter(const yaucl::structures::any_to_uint_bimap<std::string>& bijection) : bijection{bijection}, to_print{
            nullptr} {}
    friend std::ostream& operator<<(std::ostream& os, const struct DeclareStraightforwardPrinter& d);
};

#include <list>

//static inline void expand_list(std::list<DatalessCases>& list) {
//    for (auto it = list.begin(); it != list.end(); it++) {
//        switch (it->casus) {
//            case Choice:
//            case ExclChoice:
//                if (it->right < it->left)
//                    std::swap(it->right, it->left);
//                break;
//            case CoExistence:
//                it->casus = RespExistence;
//                it = list.emplace(it, RespExistence, it->right, it->left);
//                break;
//            case ChainSuccession:
//                it->casus = ChainResponse;
//                it = list.emplace(it, ChainPrecedence, it->right, it->left);
//                break;
//            case Succession:
//                it->casus = Response;
//                it = list.emplace(it, Precedence, it->left, it->right);
//                break;
//            case AltSuccession:
//                it->casus = AltPrecedence;
//                it = list.emplace(it, AltResponse, it->left, it->right);
//                break;
//            case AltPrecedence:
//            case AltResponse:
//            case Existence:
//            case Absence:
//            case RespExistence:
//            case NegSuccession:
//            case NegChainSuccession:
//            case Response:
//            case Precedence:
//            case ChainResponse:
//            case ChainPrecedence:
//                break;
//        }
//    }
//}

#endif //REDUCER_DECLARE_CASES_H
