//
// Created by giacomo on 19/08/23.
//

#include "declare_cases.h"

DatalessCases::DatalessCases(declare_cases casus, const std::string &left, const std::string &right) : casus(casus),
                                                                                                       left(left),
                                                                                                       right(right) {}
#include <stdexcept>
#include <yaucl/strings/string_utils.h>
#include <magic_enum.hpp>
#include <sstream>
#include <iomanip>

DatalessCases::DatalessCases(const std::string& line) {
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
    if (!(iss >> std::quoted(left))) {
        throw std::runtime_error("ERROR: expecting at least one argument!");
    }
    iss >> std::quoted(right);
}


std::ostream& operator<<(std::ostream& os, const struct DatalessCases& d) {
    return os << magic_enum::enum_name(d.casus) << "(" << std::quoted(d.left) << " " << std::quoted(d.right) << ")";
}