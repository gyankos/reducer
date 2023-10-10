/*
 * declare_to_lydialtlf.cpp
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
// Created by Anonymous Ⅳ on 26/08/23.
//

#include <declare_to_lydialtlf.h>
#include <ltlf.h>

std::ostream& generate_axiom_ltlf(std::ostream& os,
                         yaucl::structures::any_to_uint_bimap<std::string>& act_map,
                         std::vector<ltlf>& atoms,
                         const std::string& and_ = " & ") {
    size_t N = act_map.int_to_T.size();
    for (size_t i = 0; i<N; i++) {
        for (size_t j=i+1; j<N; j++) {
            os << (
                    ltlf::globally_(ltlf::implies(ltlf::and_(atoms.at(i), atoms.at(j)), ltlf::ltlf_false)));
            if (((i+2)!=N) || ((i+1) != j)) {
                os << and_;
            }
        }

    }
    return os;
}

void stream_lydia_ltlf(std::ostream& os,
                       const std::vector<DatalessCases>& model,
                       yaucl::structures::any_to_uint_bimap<std::string>& act_map,
                       bool withAxiom) {
    if (model.empty()){
        os << "ff";
        return;
    } else if ((model.size() == 1) && (model.at(0).casus==TRUTH)) {
        os << "tt";
        return;
    } else {
        std::vector<ltlf> atoms;
        for (std::string s : act_map.int_to_T) {
            s.erase(std::remove_if(s.begin(), s.end(),
                                   []( auto const& c ) -> bool { return !std::isalnum(c); } ), s.end());
            atoms.emplace_back(s);
        }
        std::unordered_set<ltlf> binary_operands;
        // Generating the axioms being required to...
        for (size_t i = 0, N =model.size(); i<N; i++) {
            const auto& clause = model.at(i);
            switch (clause.casus) {
                case Existence:
                    (ltlf::future_(atoms.at(clause.left))).to_aaltaf(os);
                    break;
                case Absence:
                    (ltlf::globally_(ltlf::not_(atoms.at(clause.left)))).to_aaltaf(os);
                    break;
                case Choice:
                    (ltlf::or_(ltlf::future_(atoms.at(clause.left)),
                                     ltlf::future_(atoms.at(clause.right)))).to_aaltaf(os);;
                    break;
                case ExclChoice:
                    (ltlf::or_(ltlf::future_(atoms.at(clause.left)),
                                     ltlf::future_(atoms.at(clause.right)))).to_aaltaf(os) << " & ";
                    (ltlf::not_(ltlf::and_(atoms.at(clause.left), atoms.at(clause.right)))).to_aaltaf(os);
                    break;
                case RespExistence:
                   (ltlf::implies(ltlf::future_(atoms.at(clause.left)),
                                         ltlf::future_(atoms.at(clause.right))).to_aaltaf(os));
                    break;
                case CoExistence:
                    (ltlf::and_(ltlf::implies(ltlf::future_(atoms.at(clause.left)),
                                                    ltlf::future_(atoms.at(clause.right))),
                                      ltlf::implies(ltlf::future_(atoms.at(clause.right)),
                                                    ltlf::future_(atoms.at(clause.left))))).to_aaltaf(os);;
                    break;
                case Response:
                {
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::future_(atoms.at(clause.right))))).to_aaltaf(os);;

                }
                    break;
                case Precedence:
                    (ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)), atoms.at(clause.left))).to_aaltaf(os);;
                    break;
                case ChainResponse:
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(atoms.at(clause.left))))).to_aaltaf(os);;
                    break;
                case ChainPrecedence:
                    (ltlf::globally_(ltlf::implies(ltlf::x(atoms.at(clause.left)), atoms.at(clause.right)))).to_aaltaf(os);;
                    break;
                case ChainSuccession:
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(atoms.at(clause.left))))).to_aaltaf(os)
                     << " & ";
                    (ltlf::globally_(ltlf::implies(ltlf::x(atoms.at(clause.right)), atoms.at(clause.left)))).to_aaltaf(os);;
                    break;
                case Succession:
                    os << (ltlf::until_(ltlf::not_(atoms.at(clause.right)), atoms.at(clause.left)));
                    os << " & " << (ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::future_(atoms.at(clause.right)))));
                    break;
                case AltPrecedence: // (¬bW a) ∧ G(b → X (¬bW a))
                    os <<(ltlf::and_(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                   atoms.at(clause.left)),
                                     ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                                   ltlf::x(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                                                         atoms.at(clause.left)))))));
                    break;
                case AltSuccession:
                    os <<(ltlf::and_(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                   atoms.at(clause.left)),
                                     ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                                   ltlf::x(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                                                         atoms.at(clause.left)))))));
                    os << " & " << (ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                                  ltlf::x(ltlf::until_(ltlf::not_(atoms.at(clause.right)) ,
                                                                                       atoms.at(clause.left))))));
                    break;
                case AltResponse:
                    os <<(ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                        ltlf::x(ltlf::until_(ltlf::not_(atoms.at(clause.right)) ,
                                                                             atoms.at(clause.left))))));
                    break;
                case NegSuccession:
                    os <<(ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::globally_(ltlf::not_(atoms.at(clause.right))))));
                    break;
                case NegChainSuccession:
                    os <<(ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(ltlf::not_(atoms.at(clause.left))))));
                    os << " & " << (ltlf::globally_(ltlf::implies(ltlf::x(ltlf::not_(atoms.at(clause.right))), atoms.at(clause.left))));
                    break;
                case NotCoexistence:
                    os <<(ltlf::not_(ltlf::and_(atoms.at(clause.left), atoms.at(clause.right))));
                    break;
                case TRUTH:
                    os <<(ltlf::ltlf_true);
                    break;
            }
            if ((i+1) != N) {
                os << " & ";
            }
        }
        if (withAxiom) {
            os << " & ";
            generate_axiom_ltlf(os, act_map, atoms);
        }
    }
}

void stream_aaltaf_ltlf(std::ostream& os,
                       const std::vector<DatalessCases>& model,
                       yaucl::structures::any_to_uint_bimap<std::string>& act_map,
                       bool withAxiom) {
    if (model.empty()){
        os << "ff";
        return;
    } else if ((model.size() == 1) && (model.at(0).casus==TRUTH)) {
        os << "tt";
        return;
    } else {
        std::vector<ltlf> atoms;
        for (std::string s : act_map.int_to_T) {
            s.erase(std::remove_if(s.begin(), s.end(),
            []( auto const& c ) -> bool { return !std::isalnum(c); } ), s.end());
            atoms.emplace_back(s);
        }
        std::unordered_set<ltlf> binary_operands;
        // Generating the axioms being required to...
        for (size_t i = 0, N =model.size(); i<N; i++) {
            const auto& clause = model.at(i);
            switch (clause.casus) {
                case Existence:
                    os << (ltlf::future_(atoms.at(clause.left)));
                    break;
                case Absence:
                    os << (ltlf::globally_(ltlf::not_(atoms.at(clause.left))));
                    break;
                case Choice:
                    os << (ltlf::or_(ltlf::future_(atoms.at(clause.left)),
                                                      ltlf::future_(atoms.at(clause.right))));
                    break;
                case ExclChoice:
                    os << (ltlf::or_(ltlf::future_(atoms.at(clause.left)),
                                                      ltlf::future_(atoms.at(clause.right))));
                    os << " && " << (ltlf::not_(ltlf::and_(atoms.at(clause.left), atoms.at(clause.right))));
                    break;
                case RespExistence:
                    os << (ltlf::implies(ltlf::future_(atoms.at(clause.left)),
                                                          ltlf::future_(atoms.at(clause.right))));
                    break;
                case CoExistence:
                    os << (ltlf::and_(ltlf::implies(ltlf::future_(atoms.at(clause.left)),
                                                                     ltlf::future_(atoms.at(clause.right))),
                                                       ltlf::implies(ltlf::future_(atoms.at(clause.right)),
                                                                     ltlf::future_(atoms.at(clause.left)))));
                    break;
                case Response:
                {
                    std::stringstream  ss;
                    ss << (ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::future_(atoms.at(clause.right)))));
                    auto s = ss.str();
                    os << s;
                }
                    break;
                case Precedence:
                    os << (ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)), atoms.at(clause.left)));
                    break;
                case ChainResponse:
                    os << (ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(atoms.at(clause.left)))));
                    break;
                case ChainPrecedence:
                    os << (ltlf::globally_(ltlf::implies(ltlf::x(atoms.at(clause.left)), atoms.at(clause.right))));
                    break;
                case ChainSuccession:
                    os <<(ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(atoms.at(clause.left)))));
                    os << " && " << (ltlf::globally_(ltlf::implies(ltlf::x(atoms.at(clause.right)), atoms.at(clause.left))));
                    break;
                case Succession:
                    (ltlf::until_(ltlf::not_(atoms.at(clause.right)), atoms.at(clause.left))).to_aaltaf(os)
                     << " && ";
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::future_(atoms.at(clause.right))))).to_aaltaf(os);
                    break;
                case AltPrecedence: // (¬bW a) ∧ G(b → X (¬bW a))
                    (ltlf::and_(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                                     atoms.at(clause.left)),
                                                       ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                                                     ltlf::x(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                                                                           atoms.at(clause.left))))))).to_aaltaf(os);;
                    break;
                case AltSuccession:
                    (ltlf::and_(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                                     atoms.at(clause.left)),
                                                       ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                                                     ltlf::x(ltlf::wuntil_(ltlf::not_(atoms.at(clause.right)) ,
                                                                                                           atoms.at(clause.left))))))).to_aaltaf(os)<< " && ";
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                                          ltlf::x(ltlf::until_(ltlf::not_(atoms.at(clause.right)) ,
                                                                                               atoms.at(clause.left)))))).to_aaltaf(os);;
                    break;
                case AltResponse:
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.right),
                                                                          ltlf::x(ltlf::until_(ltlf::not_(atoms.at(clause.right)) ,
                                                                                                atoms.at(clause.left)))))).to_aaltaf(os);;
                    break;
                case NegSuccession:
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::globally_(ltlf::not_(atoms.at(clause.right)))))).to_aaltaf(os);;
                    break;
                case NegChainSuccession:
                    (ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(ltlf::not_(atoms.at(clause.left)))))).to_aaltaf(os)
                    << " && ";
                    (ltlf::globally_(ltlf::implies(ltlf::x(ltlf::not_(atoms.at(clause.right))), atoms.at(clause.left)))).to_aaltaf(os);;
                    break;
                case NotCoexistence:
                    (ltlf::not_(ltlf::and_(atoms.at(clause.left), atoms.at(clause.right)))).to_aaltaf(os);
                    break;
                case TRUTH:
                    (ltlf::ltlf_true).to_aaltaf(os);;
                    break;
            }
            if ((i+1) != N) {
                os << " && ";
            }
        }
        if (withAxiom) {
            os << " && ";
            generate_axiom_ltlf(os, act_map, atoms, " && ");
        }
    }
}
