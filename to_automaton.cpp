/*
 * to_automaton.cpp
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

 
#include <iostream>

#include <fstream>
#include <declare_cases.h>
#include <model_reducer.h>
#include "include/ltlf.h"
#include <chrono>
#include <declare_to_lydialtlf.h>
#include <declare_to_powerdecl.h>
#include <filesystem>

int main() {
    ltlf a{"a"};
    ltlf b{"b"};
    ltlf f{"f"};
    ltlf d{"d"};

//
//    // G(f -> X(d)) &  G(d ->X(a)) & G(a ->G(!b)) & G(a-> F(b)) & (((!b) U a) | G(!b))
//
//
//    auto gg = ltlf::and_(ltlf::globally_(ltlf::implies(f, ltlf::x(d))),
//              ltlf::and_(ltlf::globally_(ltlf::implies(d, ltlf::x(a))),
//              ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::globally_(ltlf::not_(b)))),
//               ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::future_(b))),ltlf::or_(ltlf::until_(ltlf::not_(b), a), ltlf::globally_(ltlf::not_(b))))
//               )));
//
//    my_unordered_set<my_unordered_set<ltlf::atom_t>> j;
//
    auto formula = ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::x(ltlf::future_(b)))),
                              ltlf::globally_(ltlf::implies(b, ltlf::x(ltlf::future_(a))))
    );
    /*
     * auto f = ltlf::and_(ltlf::globally_(ltlf::implies(ltlf::x(a), b)),
                        ltlf::until_(ltlf::not_(b), a)
    );
     * auto f = ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::x(b))),
                        ltlf::globally_(ltlf::implies(b, ltlf::x(c)))
    );
     *  auto f = ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::x(b))),
                        ltlf::and_(ltlf::globally_(ltlf::implies(b, ltlf::x(c))),
                                   ltlf::globally_(ltlf::implies(c, ltlf::x(a))))
    );
     */

    std::unordered_set<size_t> finals;
    std::unordered_map<size_t, std::unordered_map<ltlf::atom_t, size_t>> final_transition;

    auto start = std::chrono::system_clock::now();
    to_automaton(formula, finals, final_transition);
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration<double,std::milli>(end - start).count();
    std::cout << "Time: " << elapsed << "ms" << std::endl;
    std::cout << f.to_nnf() << std::endl;
    return 0;
}
