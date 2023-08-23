#include <iostream>


#include "declare_cases.h"
#include "model_reducer.h"

#include "ltlf.h"

void to_automaton_example() {
    ltlf a{"a"};
    ltlf b{"b"};

    my_unordered_set<my_unordered_set<ltlf::atom_t>> j;

    auto f = ltlf::and_(ltlf::until_(ltlf::not_(a), b),
                        ltlf::until_(ltlf::not_(b), a)
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
    to_automaton(f, finals, final_transition);
//    std::cout << f.to_nnf() << std::endl;
}

int main() {
    std::ifstream file{"/home/giacomo/model_example.txt"};
    std::cout << "Initial model:" << std::endl;
    auto M = streamDeclare(file);
    for (const auto& clause : M)
        std::cout << clause << std::endl;
    std::cout << std::endl << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl << std::endl;
    std::cout << "Final model:" << std::endl;
    auto v = model_reducer{}.run(M);
    for (const auto& clause : v)
        std::cout << clause << std::endl;

    return 0;
}
