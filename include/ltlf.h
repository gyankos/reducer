//
// Created by giacomo on 20/08/23.
//

#ifndef REDUCER_LTLF_H
#define REDUCER_LTLF_H

#include <iostream>
#include <vector>
#include <ostream>
#include <ostream>
#include <sstream>
#include <unordered_map>
#include <stdexcept>
#include <my_unordered_set.h>

#include <vector>
#include <cmath>
#include <list>
#include <yaucl/graphs/FlexibleFA.h>
#include <yaucl/graphs/algorithms/minimizeDFA.h>



template
        <typename T>
std::list<std::unordered_set<T>> powerset_list(const std::unordered_set<T> & a) {
    std::vector<T> set{a.begin(), a.end()};
    std::list<std::unordered_set<T>> result;
    /*set_size of power set of a set with set_size
    n is (2**n -1)*/

    unsigned long long int pow_set_size = std::pow(2, a.size()), set_size = a.size();
    unsigned long long int counter, j;

    /*Run from counter 000..0 to 111..1*/
    for(counter = 0; counter < pow_set_size; counter++)
    {
        std::unordered_set<T> subset;
        for(j = 0; j < set_size; j++)
        {

            /* Check if jth bit in the counter is set
                If set then print jth element from set */
            if(counter & (1 << j))
                subset.insert(set.at(j));
        }
        result.emplace_back(subset);
    }
    return result;
}

struct ltlf {
    using atom_t = std::string;
    enum cases {
        UNTIL,
        GLOBALLY,
        FUTURE,
        ATOM,
        NOT,
        AND,
        OR,
        NEXT,
        TRUE,
        FALSE,
        RELEASE,
        WEAKNEXT,
        END
    };
    cases t;
    atom_t atom;
    bool setAtomic =false;
    std::vector<ltlf> children;

    bool isAtomic() const {
        return (t == ATOM) || (t == TRUE) || (t == FALSE) || setAtomic;
    }
    ltlf(const atom_t& x) : t{ATOM}, atom{x} {}
    ltlf(cases casus) : t{casus} {};
    ltlf() = default;
    ltlf(const ltlf&) = default;
    ltlf(const ltlf& x, bool atomic) {
        *this = x;
        setAtomic = atomic;
    }
    ltlf(ltlf&&) = default;
    ltlf& operator=(const ltlf&) = default;
    ltlf& operator=(ltlf&&) = default;
    const static ltlf ltlf_true;
    const static ltlf ltlf_false;
    const static ltlf ltlf_end;

    ltlf to_nnf() const;
    ltlf negate() const;
    void collect_multi_arg(std::unordered_set<ltlf>& collect_args, cases t) const;
    ltlf simplify() const;
    ltlf multicollect() const;

    static ltlf not_(const ltlf& left);
    static ltlf x(const ltlf& left);
    static ltlf wx(const ltlf& left);
    static ltlf future_(const ltlf& left);
    static ltlf globally_(const ltlf& left);
    static ltlf implies(const ltlf& left, const ltlf& right);
    static ltlf or_(const ltlf& left, const ltlf& right);
    static ltlf until_(const ltlf& left, const ltlf& right);
    static ltlf release_(const ltlf& left, const ltlf& right);
    static ltlf and_(const ltlf& left, const ltlf& right);
    static ltlf platomic(const ltlf& argument);
    static ltlf wuntil_(const ltlf& left, const ltlf& right) {
        return ltlf::or_(ltlf::until_(left, right), ltlf::globally_(left));
    }

    bool truth(const std::unordered_set<atom_t> &map) const;
    bool truth(const std::unordered_map<atom_t,bool>& map) const;
    bool truth(const atom_t& map) const;

    inline bool pl_all_satisfiable_models(std::list<std::unordered_set<atom_t>>& possible_worlds) const {
        std::unordered_set<atom_t> all_atoms;
        collect_atoms(all_atoms);
        possible_worlds.clear();
        possible_worlds = powerset_list(all_atoms);
        size_t allPossibleWorlds = possible_worlds.size();
        for (auto it = possible_worlds.begin(); it != possible_worlds.end(); ) {
            if (truth(*it)) it++;
            else it = possible_worlds.erase(it);
        }
        return possible_worlds.size() == allPossibleWorlds;
    }

    ltlf delta(const std::unordered_map<atom_t,bool>& map, bool epsilon = false) const;
    ltlf delta(const atom_t& map, bool epsilon = false) const;
    void collect_atoms(std::unordered_set<atom_t>& S) const;
    void find_atomics(std::unordered_set<ltlf>& S) const;

    bool operator<(const ltlf &rhs) const;
    bool operator>(const ltlf &rhs) const;
    bool operator<=(const ltlf &rhs) const;
    bool operator>=(const ltlf &rhs) const;
    bool operator==(const ltlf &rhs) const;
    bool operator!=(const ltlf &rhs) const;
    friend std::ostream& operator<<(std::ostream& os, const ltlf &ltlf);
    std::ostream& to_aaltaf(std::ostream& os) const;

private:
    ltlf _delta(const std::unordered_map<atom_t,bool>& map, bool epsilon = false) const;
    ltlf _delta(const atom_t& map, bool epsilon = false) const;
};



namespace std {
    template <> struct hash<ltlf> {
        std::hash<std::string> hs;
        size_t operator()(const ltlf& x) const {
            std::stringstream ss;
            ss << x;
            return hs(ss.str());
        }
    };
}

static inline ltlf
unpack_binary_operator(const std::unordered_set<ltlf> &transformed_delta_formulas,
                       const ltlf &default_val,
                       bool isAnd,
                       bool doMultiCollect = false,
                       bool simplify = true) {
    ltlf counjunctions = default_val;
    if (!transformed_delta_formulas.empty()) {
        if (transformed_delta_formulas.size() == 1) {
            counjunctions = *transformed_delta_formulas.begin();
        } else {
            auto it = transformed_delta_formulas.begin();
            counjunctions = isAnd ? ltlf::and_(*it++, *it++) : ltlf::or_(*it++, *it++);
            while (it != transformed_delta_formulas.end()) {
                counjunctions = isAnd ? ltlf::and_(*it++, counjunctions) : ltlf::or_(*it++, counjunctions);
            }
        }
    } else {
        counjunctions = default_val;
    }
    // Ensuring an unique representation of the or/and by removing branches that contain the same formula
    // multiple times
    if (doMultiCollect)
        counjunctions = counjunctions.multicollect();
    // Provides some basic formula simplification for reducing its size. The main target is to get
    // rid of some predicates that do not substantially contribute to the model, as they are nihilated
    // by true/false predicates
    if (simplify)
        counjunctions = counjunctions.simplify();
    return counjunctions;
}

using state_t = my_unordered_set<my_unordered_set<ltlf>>;

static inline ltlf _transform_delta(const ltlf& f,const std::unordered_map<ltlf,std::string>&formula2AtomicFormula ) {
    if (f.setAtomic) {
        auto IT = formula2AtomicFormula.find(f);
        if (IT == formula2AtomicFormula.end())
            throw std::runtime_error("ERROR: the formula should be in the map!");
        return ltlf{IT->second};
    } else
    switch (f.t) {
        case ltlf::NOT:
            return ltlf::not_(_transform_delta(f.children.at(0), formula2AtomicFormula));
        case ltlf::AND:
            return ltlf::and_(_transform_delta(f.children.at(0), formula2AtomicFormula),
                              _transform_delta(f.children.at(1), formula2AtomicFormula));
        case ltlf::OR:
            return ltlf::or_(_transform_delta(f.children.at(0), formula2AtomicFormula),
                              _transform_delta(f.children.at(1), formula2AtomicFormula));

        case ltlf::TRUE:
        case ltlf::FALSE:
            return f;

        default: {
            auto IT = formula2AtomicFormula.find(f);
            if (IT == formula2AtomicFormula.end())
                throw std::runtime_error("ERROR: the formula should be in the map!");
            return ltlf{IT->second};
       }
    }
}



static inline state_t _make_transition(const state_t& marco_q,
                      const ltlf::atom_t* i) {
    static std::unordered_map<ltlf::atom_t,bool> other_item;
    std::list<std::unordered_set<ltlf::atom_t>> possible_worlds;
    state_t result;
//    std::cout << (i? *i : "nullptr") << std::endl;

    /**
     * Each state declares that we might run either one of these formulae. Therefore, we
     * are running each of those in the state
     */
    for (const auto& q : marco_q) {
        std::vector<ltlf> delta_formulas;
        for (const auto& f : q){
            if (i) {
                // If the mutually disjunctive atom is present, then move the interpretation
                // Remarking which step shall be moved. An atomic representation of until/release/(w)next
                // is used to remark the part of the formula that shall not be assessed at the current step,
                // rather in any other later interpretation of the formula
                auto r = f.delta(*i);
//                std::cout << r << std::endl;
                delta_formulas.emplace_back(r);
            } else {
                // Else, check the eligibility of moving with any other symbol not contemplated by
                // the atoms providing a finitary subset of the expected alphabet
                auto r = f.delta(other_item);
//                std::cout << r << std::endl;
                delta_formulas.emplace_back(r);
            }
        }
//        std::cout << "~~~" << std::endl;
//        for (const auto& df : delta_formulas)
//            std::cout << df << std::endl;

        // Determining the atomic components: any subformula which is blocked at the current
        // evaluation step, atoms, trues, and false. These will be 0-ary predicates
        std::unordered_set<ltlf> atomics;
        for (const auto& subf: delta_formulas)
            subf.find_atomics(atomics);
//        for (const auto& atomic : atomics)
//            std::cout << atomic << std::endl;
//        std::cout << "+++" << std::endl;
        std::unordered_map<std::string,ltlf> id2atomics;
        std::unordered_map<ltlf,std::string> atom2id;
        size_t count = 0;
        std::unordered_map<ltlf,std::string> formula2atomic_formulas;
        // representing the 0-ary predicates as atoms represented as numbers
        // This is an arbitrary representation (under the assumption that any other atom
        // is non-digit). This will be used to minimise the formula for minimising the state information
        for (const auto& a : atomics)
        {
//            id2atomics.emplace_back(a);
            std::string strid =  std::to_string(count);
            atom2id[a] = strid;
            id2atomics[strid] = a;
            if ((a.t == ltlf::TRUE) || (a.t == ltlf::FALSE))
                formula2atomic_formulas[a] = (a.t == ltlf::TRUE ? "true" : "false");
            else
                formula2atomic_formulas[a] = strid;
            count++;
        }
        // Now, representing each moved step into a 0-ary predicate based representation
        std::unordered_set<ltlf> transformed_delta_formulas;
        for (const auto& subf: delta_formulas)
            transformed_delta_formulas.emplace(_transform_delta(subf, formula2atomic_formulas));
//        for (const auto& atomic : transformed_delta_formulas)
//            std::cout << atomic << std::endl;

        // Ensuring some straightforward predicated logic formula simplification
        ltlf counjunctions = unpack_binary_operator(transformed_delta_formulas, ltlf::ltlf_true, true, true);

//        std::cout << "conj:" << counjunctions << std::endl;

        // In order to further minimise the formula, we get the set of all the possible worlds
        // satisfying it, and we make a disjunction out of it
        bool isTriviallyTrue = counjunctions.pl_all_satisfiable_models(possible_worlds);
        if (possible_worlds.empty()) {
            // Never solvable, do nothing
        } else if (isTriviallyTrue) {
            possible_worlds.clear();
            possible_worlds.emplace_back(std::unordered_set<ltlf::atom_t>{});
        } else {
            // Already listing the atoms being satisfied
        }

        // Now, determining the result as its representation in disjunctive normal form
        for (const auto& min_model : possible_worlds) {
            // Defining the current minimum model as a disjunction
            my_unordered_set<ltlf> S;
            for (const auto& s : min_model)
                S.add(id2atomics.at(s));
            result.add(S);
        }
    }
    // Providing the next state information in a disjunctive normal form.
    return result;
}

static inline
bool is_true(const state_t& Q) {
    static std::unordered_map<ltlf::atom_t,bool> other_item;
    if (Q.contains({}))
        return true;
    std::vector<ltlf> conj;
    for (const auto& q : Q) {
//        for (const auto& qq : q)
//            std::cout << qq <<", ";
//        std::cout << std::endl;
        if (q.size() >= 2) {
            auto it = q.begin();
            auto l = it->delta(other_item, true); it++;
            auto r = it->delta(other_item, true); it++;
            auto counjunctions = ltlf::and_(l, r);
            while (it != q.end()) {
                counjunctions = ltlf::and_(it->delta(other_item, true), counjunctions);
                it++;
            }
            conj.emplace_back(counjunctions);
        } else if (q.size() == 1) {
            conj.emplace_back(q.begin()->delta(other_item, true));
        } else {
            conj.emplace_back(ltlf::ltlf_false);
        }
    }
//    for (const auto& qq : conj)
//        std::cout << qq <<":: ";
//    std::cout << std::endl;
    if (conj.empty())
        return false;
    else if (conj.size() == 1)
        return conj.at(0).truth(other_item);
    else {
        auto it = conj.begin();
        auto counjunctions = ltlf::or_(*it++, *it++);
        while (it != conj.end()) {
            counjunctions = ltlf::or_(*it++, counjunctions);
        }
//        conj.emplace_back(counjunctions);
//std::cout << counjunctions << std::endl;
        return counjunctions.truth(other_item);
    }
}

#include <unordered_map>

static inline
void to_automaton(const ltlf& f,
                  std::unordered_set<size_t>& finals,
                  std::unordered_map<size_t, std::unordered_map<ltlf::atom_t, size_t>>& final_transition_function) {
    auto formula = f.to_nnf();
//    std::cout << formula << std::endl;
    state_t initial_state{{formula}};
    my_unordered_set<state_t> states{initial_state};
    std::unordered_map<state_t, size_t> states_to_id;
    my_unordered_set<state_t> final_states;
    my_unordered_set<state_t> visited;
    my_unordered_set<state_t> to_be_visited{initial_state};
    std::unordered_map<state_t, std::unordered_map<ltlf::atom_t, state_t>> transition_function;
    std::unordered_set<ltlf::atom_t> all_labels;
    formula.collect_atoms(all_labels);
    std::unordered_set<const ltlf::atom_t*> ptr;
    ptr.emplace(nullptr);
    for (const auto& x : all_labels) ptr.emplace(&x);
    std::unordered_map<ltlf::atom_t,bool> other_item;

    if (formula.delta(other_item, true).t == ltlf::TRUE)
        final_states.add(initial_state);

    while (!to_be_visited.empty()) {
        std::vector<state_t> L{to_be_visited.begin(), to_be_visited.end()};
        for (const auto& q : L) {
            to_be_visited.remove(q);
            for (const ltlf::atom_t* actions_set : ptr) {
                auto new_state = _make_transition(q, actions_set);
                if (states.add(new_state)) {
                    to_be_visited.add(new_state);
                }
                transition_function[q][actions_set ? *actions_set : "?"] = new_state;
                if (visited.add(new_state)) {
                    if (is_true(new_state))
                        final_states.add(new_state);
                }
            }
        }
    }

    FlexibleFA<size_t, ltlf::atom_t> g;
    for (const auto& s : states) {
        size_t id = states_to_id.size();
        states_to_id[s] = id;
        size_t val = g.addNewNodeWithLabel(id);
        DEBUG_ASSERT(id == val);
    }

    finals.clear();
    for (const auto& final : final_states) {
        g.addToFinalNodesFromId(states_to_id.at(final));
//        finals.emplace(states_to_id.at(final));
    }

    final_transition_function.clear();
    for (const auto& [s, m] : transition_function) {
        size_t srcId = states_to_id.at(s);
//        auto& M = final_transition_function[states_to_id.at(s)];
        for (const auto& [ptr2, sp] : m) {
            size_t dstId = states_to_id.at(sp);
            g.addNewEdgeFromId(srcId, dstId, ptr2);
//            M[ptr2] = states_to_id.at(sp);
        }
    }

    auto result = minimizeDFA(g);
//    result.dot(std::cout, false);

}




#endif //REDUCER_LTLF_H
