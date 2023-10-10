/*
 * declare_cases.cpp
 * This file is part of DECLAREd
 *
 * Copyright (C) 2023 - Anonymous Ⅳ
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

#include <declare_cases.h>

DatalessCases::DatalessCases(declare_cases casus, const size_t &left, const size_t &right) : casus(casus),
                                                                                                       left(left),
                                                                                                       right(right) {}
#include <stdexcept>
#include <yaucl/strings/string_utils.h>
#include <magic_enum.hpp>
#include <sstream>
#include <iomanip>

DatalessCases::DatalessCases(const std::string& line, yaucl::structures::any_to_uint_bimap<std::string>& bijection) {
    if (line == "TRUTH") {
        casus = TRUTH;
        left = right = -1;
    } else {
        size_t start_par = line.find('(');
        if (start_par == std::string::npos)
            throw std::runtime_error("ERROR: missing (!");
        std::string name = line.substr(0, start_par);
        yaucl::strings::trim(name);
        size_t end_par = line.find_last_of(')');
        if (end_par == std::string::npos)
            throw std::runtime_error("ERROR: missing )!");
        auto castingExpect = magic_enum::enum_cast<declare_cases>(name);
        if (!castingExpect) {
            throw std::runtime_error("ERROR: expecting a name from the declare_cases!");
        }
        casus = castingExpect.value();
        std::string args = line.substr(start_par+1, end_par-start_par-1);
        std::istringstream iss(args);
        std::string tmp_left, tmp_right;
        if (!(iss >> std::quoted(tmp_left))) {
            throw std::runtime_error("ERROR: expecting at least one argument!");
        } else {
            left = bijection.put(tmp_left).first;
        }
        if (iss >> std::quoted(tmp_right)) {
            right = bijection.put(tmp_right).first;
        } else {
            right = -1;
        }
    }
}


std::ostream& operator<<(std::ostream& os, const struct DeclareStraightforwardPrinter& d) {
    if (d.to_print == nullptr)
        return  os << "nullptr";
    else if (d.to_print->casus == TRUTH) {
        return os << "TRUTH";
    }  else {
            os << magic_enum::enum_name(d.to_print->casus) << "(" << std::quoted(d.bijection.getValue(d.to_print->left)) << "";
         if (d.to_print->right==(size_t)-1)
             return os << ")";
         else
             return os << " " << ( std::quoted(d.bijection.getValue(d.to_print->right))) << ")";
    }
}
