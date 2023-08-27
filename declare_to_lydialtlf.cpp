//
// Created by giacomo on 26/08/23.
//

#include "declare_to_lydialtlf.h"
#include "ltlf.h"

ltlf generate_axiom_ltlf(yaucl::structures::any_to_uint_bimap<std::string>& act_map,
                         std::vector<ltlf>& atoms) {
    size_t N = act_map.int_to_T.size();
    std::unordered_set<ltlf> binary_operands;
    for (const std::string& a : act_map.int_to_T)
        atoms.emplace_back(a);
    for (size_t i = 0; i<N; i++) {
        for (size_t j=i+1; j<N; j++) {
            binary_operands.emplace(
                    ltlf::globally_(ltlf::implies(ltlf::and_(atoms.at(i), atoms.at(j)), ltlf::ltlf_false)));
        }
    }
    return unpack_binary_operator(binary_operands, ltlf::ltlf_true, true, false);
}

void stream_lydia_ltlf(std::ostream& os,
                       const std::vector<DatalessCases>& model,
                       yaucl::structures::any_to_uint_bimap<std::string>& act_map) {
    if (model.empty()){
        os << "ff";
        return;
    } else if ((model.size() == 1) && (model.at(0).casus==TRUTH)) {
        os << "tt";
        return;
    } else {
        std::vector<ltlf> atoms;
        std::unordered_set<ltlf> binary_operands;
        // Generating the axioms being required to...
        ltlf axiom = generate_axiom_ltlf(act_map, atoms);
        for (const auto& clause : model) {
            switch (clause.casus) {
                case Existence:
                    binary_operands.emplace(ltlf::future_(atoms.at(clause.left)));
                    break;
                case Absence:
                    binary_operands.emplace(ltlf::globally_(ltlf::not_(atoms.at(clause.left))));
                    break;
                case Choice:
                    binary_operands.emplace(ltlf::or_(ltlf::future_(atoms.at(clause.left)),
                                                      ltlf::future_(atoms.at(clause.right))));
                    break;
                case ExclChoice:
                    break;
                case RespExistence:
                    binary_operands.emplace(ltlf::implies(ltlf::future_(atoms.at(clause.left)),
                                                          ltlf::future_(atoms.at(clause.right))));
                    break;
                case CoExistence:
                    binary_operands.emplace(ltlf::and_(ltlf::implies(ltlf::future_(atoms.at(clause.left)),
                                                                     ltlf::future_(atoms.at(clause.right))),
                                                       ltlf::implies(ltlf::future_(atoms.at(clause.right)),
                                                                     ltlf::future_(atoms.at(clause.left)))));
                    break;
                case Response:
                    binary_operands.emplace(ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::future_(atoms.at(clause.right)))));
                    break;
                case Precedence:
                    binary_operands.emplace(ltlf::until_(ltlf::not_(atoms.at(clause.right)), atoms.at(clause.left)));
                    break;
                case ChainResponse:
                    binary_operands.emplace(ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(atoms.at(clause.left)))));
                    break;
                case ChainPrecedence:
                    binary_operands.emplace(ltlf::globally_(ltlf::implies(ltlf::x(atoms.at(clause.left)), atoms.at(clause.right))));
                    break;
                case ChainSuccession:
                    binary_operands.emplace(ltlf::globally_(ltlf::implies(atoms.at(clause.right), ltlf::x(atoms.at(clause.left)))));
                    binary_operands.emplace(ltlf::globally_(ltlf::implies(ltlf::x(atoms.at(clause.right)), atoms.at(clause.left))));
                    break;
                case Succession:
                    binary_operands.emplace(ltlf::until_(ltlf::not_(atoms.at(clause.right)), atoms.at(clause.left)));
                    binary_operands.emplace(ltlf::globally_(ltlf::implies(atoms.at(clause.left), ltlf::future_(atoms.at(clause.right)))));
                    break;
                case AltPrecedence:
                    break;
                case AltSuccession:
                    break;
                case AltResponse:
                    break;
                case NegSuccession:
                    break;
                case NegChainSuccession:
                    break;
                case NotCoexistence:
                    break;
                case TRUTH:
                    binary_operands.emplace(ltlf::ltlf_true);
                    break;
            }
        }

        os << unpack_binary_operator(binary_operands, ltlf::ltlf_true, true, false) << " & " << axiom;
    }
}