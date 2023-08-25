#include <iostream>


#include "declare_cases.h"
#include "model_reducer.h"

#include "ltlf.h"

void to_automaton_example() {
    ltlf a{"a"};
    ltlf b{"b"};
    ltlf f{"f"};
    ltlf d{"d"};


    // G(f -> X(d)) &  G(d ->X(a)) & G(a ->X(!b)) & G(a-> F(b)) & ((!b) U a)

    auto gg = ltlf::and_(ltlf::globally_(ltlf::implies(f, ltlf::x(d))),
    ltlf::and_(ltlf::globally_(ltlf::implies(d, ltlf::x(a))),
    ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::globally_(ltlf::not_(b)))),
    ltlf::globally_(ltlf::implies(a, ltlf::future_(b))))));

    my_unordered_set<my_unordered_set<ltlf::atom_t>> j;

//    auto f = ltlf::and_(ltlf::until_(ltlf::not_(a), b),
//                        ltlf::until_(ltlf::not_(b), a)
//    );
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
    to_automaton(gg, finals, final_transition);
//    std::cout << f.to_nnf() << std::endl;
}

int reducing() {
    std::ifstream file{"/home/giacomo/model_example.txt"};
    std::cout << "Initial model:" << std::endl;
    yaucl::structures::any_to_uint_bimap<std::string> bijection;
    auto M = streamDeclare(file, bijection);
    DeclareStraightforwardPrinter printer{bijection};
    for (const auto& clause : M) {
        printer.to_print = &clause;
        std::cout << printer << std::endl;
    }
    std::cout << std::endl << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl << std::endl;
    std::cout << "Final model:" << std::endl;
    auto v = model_reducer{}.run(M);
    for (const auto& clause : v){
        printer.to_print = &clause;
        std::cout << printer << std::endl;
    }
    return 0;
}

int main() {
    reducing();


    return 0;
}
