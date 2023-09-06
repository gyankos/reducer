//
// Created by giacomo on 26/08/23.
//

#include <declare_to_powerdecl.h>

void stream_powerdecl(std::ostream& os,
                      const std::vector<DatalessCases>& model,
                      yaucl::structures::any_to_uint_bimap<std::string>& act_map) {
    if (model.empty()){
        os << "ff";
        return;
    } else if ((model.size() == 1) && (model.at(0).casus==TRUTH)) {
        os << "tt";
        return;
    } else {
        os << "declare ";
//        std::vector<ltlf> atoms;
//        for (const std::string& a : act_map.int_to_T)
//            atoms.emplace_back(a);
//        std::unordered_set<ltlf> binary_operands;
        // Generating the axioms being required to...
        for (size_t i = 0, N =model.size(); i<N; i++) {
            const auto& clause = model.at(i);
            switch (clause.casus) {
                case Existence:
                    os << "\t\"Exists1\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, 1)";
                    break;
                case Absence:
                    os << "\t\"Absence1\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, 1)";
                    break;
                case Choice:
                    os << "\t\"Choice\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case ExclChoice:
                    os << "\t\"ExclChoice\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case RespExistence:
                    os << "\t\"RespExistence\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case CoExistence:
                    os << "\t\"CoExistence\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case Response:
                    os << "\t\"Response\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case Precedence:
                    os << "\t\"Precedence\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case ChainResponse:
                    os << "\t\"ChainResponse\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;

                case ChainPrecedence:
                    os << "\t\"ChainPrecedence\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case ChainSuccession:
                    os << "\t\"ChainSuccession\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;

                case Succession:
                    os << "\t\"Succession\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;

                case AltPrecedence: // (¬bW a) ∧ G(b → X (¬bW a))
                    os << "\t\"AltPrecedence\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case AltSuccession:
                    os << "\t\"AltPrecedence\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)" << std::endl;
                    os << "\t\"AltResponse\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case AltResponse:
                    os << "\t\"AltResponse\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case NegSuccession:
                    os << "\t\"NegSuccession\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case NegChainSuccession:
                    throw std::runtime_error("Unexpected clause!");
                    break;
                case NotCoexistence:
                    os << "\t\"NotCoExistence\"( \"" << act_map.int_to_T.at(clause.left) << "\", true, \""<< act_map.int_to_T.at(clause.right) <<"\", true)";
                    break;
                case TRUTH:
                    throw std::runtime_error("Unexpected clause!");
            }
            os << std::endl;
        }
    }
}
