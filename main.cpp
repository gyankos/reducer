/*
 * main.cpp
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
//#include "include/ltlf.h"
#include <chrono>
#include <declare_to_lydialtlf.h>
#include <declare_to_powerdecl.h>
#include <filesystem>

//void to_automaton_example() {
//    ltlf a{"a"};
//    ltlf b{"b"};
//    ltlf f{"f"};
//    ltlf d{"d"};
//
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
////    auto f = ltlf::and_(ltlf::until_(ltlf::not_(a), b),
////                        ltlf::until_(ltlf::not_(b), a)
////    );
//    /*
//     * auto f = ltlf::and_(ltlf::globally_(ltlf::implies(ltlf::x(a), b)),
//                        ltlf::until_(ltlf::not_(b), a)
//    );
//     * auto f = ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::x(b))),
//                        ltlf::globally_(ltlf::implies(b, ltlf::x(c)))
//    );
//     *  auto f = ltlf::and_(ltlf::globally_(ltlf::implies(a, ltlf::x(b))),
//                        ltlf::and_(ltlf::globally_(ltlf::implies(b, ltlf::x(c))),
//                                   ltlf::globally_(ltlf::implies(c, ltlf::x(a))))
//    );
//     */
//
//    std::unordered_set<size_t> finals;
//    std::unordered_map<size_t, std::unordered_map<ltlf::atom_t, size_t>> final_transition;
//
//    auto start = std::chrono::system_clock::now();
//    to_automaton(gg, finals, final_transition);
//    auto end = std::chrono::system_clock::now();
//    auto elapsed = std::chrono::duration<double,std::milli>(end - start).count();
//    std::cout << "Time: " << elapsed << "ms" << std::endl;
////    std::cout << f.to_nnf() << std::endl;
//}



double reducing(const std::string& name,
                bool verbose,
             const std::string& in,
             const std::string& out_in_lydia,
             const std::string& out_in_lydia_ax,
             const std::string& out_red_lydia,
                const std::string& out_in_pd,
                const std::string& out_red_pd,
                const std::string& out_in_aaltaf,
                const std::string& out_in_aaltaf_ax,
                const std::string& out_red_aaltaf) {
//    std::ifstream file{"/home/giacomo/model_example.txt"};
    std::ifstream file{in};
    if (verbose) std::cout << "Initial model:" << std::endl;
    yaucl::structures::any_to_uint_bimap<std::string> bijection;
    auto M = streamDeclare(file, bijection);
    DeclareStraightforwardPrinter printer{bijection};
    if (verbose) {
        for (const auto& clause : M) {
            printer.to_print = &clause;
            std::cout << printer << std::endl;
        }
        std::cout << std::endl << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl << std::endl;
        std::cout << "Final model:" << std::endl;
    }
    if (!out_in_lydia.empty()) {
        std::ofstream lydida_file{out_in_lydia};
        stream_lydia_ltlf(lydida_file, M, bijection, false);
    }
    if (!out_in_lydia_ax.empty()) {
        std::ofstream lydida_file{out_in_lydia_ax};
        stream_lydia_ltlf(lydida_file, M, bijection, true);
    }
    if (!out_in_aaltaf.empty()) {
        std::ofstream lydida_file{out_in_aaltaf};
        stream_aaltaf_ltlf(lydida_file, M, bijection, false);
    }
    if (!out_in_aaltaf_ax.empty()) {
        std::ofstream lydida_file{out_in_aaltaf_ax};
        stream_aaltaf_ltlf(lydida_file, M, bijection, true);
    }
    if (!out_in_pd.empty()) {
        std::ofstream lydida_file{out_in_pd};
        stream_powerdecl(lydida_file, M, bijection);
    }
    model_reducer obj;
    auto start = std::chrono::system_clock::now();
    auto v = obj.run(M);
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration<double,std::milli>(end - start).count();
    if (verbose) {
        for (const auto& clause : v){
            printer.to_print = &clause;
            std::cout << printer << std::endl;
        }
        std::cout << "Time: " << elapsed << "ms" << std::endl;
    }
    if (!name.empty()) {
        std::ofstream csv{"out.csv", std::ios_base::app};
        std::filesystem::path p{in};
        csv << name << ",proposed," << elapsed << std::endl;
    }
    if (!out_red_lydia.empty()) {
        std::ofstream lydida_file{out_red_lydia};
        stream_lydia_ltlf(lydida_file, v, bijection, false);
    }
    if (!out_red_aaltaf.empty()) {
        std::ofstream lydida_file{out_red_aaltaf};
        stream_lydia_ltlf(lydida_file, v, bijection, false);
    }
    if (!out_red_pd.empty()) {
        std::ofstream lydida_file{out_red_pd};
        stream_powerdecl(lydida_file, v, bijection);
    }
    return elapsed;
}

#include <args.hxx>
#include <filesystem>

int main(int argc, char **argv) {
    args::ArgumentParser parser("Declare reduction model", "This program provides model reduction or its rewriting into a specified format");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Positional<std::string> in_filename(parser, "file", "The file containing the declare dataless model specification");
    args::Flag foo(parser, "verbose", "Verbose flag", {'v', "verbose"});
    args::ValueFlag<std::string> out_namef(parser, "name", "name to associate to the file" , {"name", 'n'});
    args::ValueFlag<std::string> out_ltlf(parser, "ltlf_file", "ltlf representation of the input model, with lydia/logaut compatible syntax" , {"ltlf", 'l'});
    args::ValueFlag<std::string> out_ltlfax(parser, "ltlf_axiom_file", "ltlf representation of the input model and the axiom, with lydia/logaut compatible syntax", {"ltlfax", 'x'});
    args::ValueFlag<std::string> out_ltlfred(parser, "ltlf_axiom_red_file", "ltlf representation of the obtained reduced model", {"ltlfreduced", 'r'});
    args::ValueFlag<std::string> out_aaltaf(parser, "aaltaf_file", "ltlf representation of the input model, with lydia/logaut compatible syntax" , {"aaltaf", 'a'});
    args::ValueFlag<std::string> out_aaltafax(parser, "aaltaf_axiom_file", "ltlf representation of the input model and the axiom, with lydia/logaut compatible syntax", {"aaltafax", 't'});
    args::ValueFlag<std::string> out_aaltafred(parser, "aaltaf_axiom_red_file", "ltlf representation of the obtained reduced model", {"aaltafreduced", 'd'});
    args::ValueFlag<std::string> out_pd(parser, "powerdecl_file", "powerdecl representation of the input model", {"powerdecl", 'p'});
    args::ValueFlag<std::string> out_pdred(parser, "powerdeclreduced_file", "powerdecl representation of the obtained reduced model", {"powerdeclreduced", 'e'});
    std::string out_in_lydia;
    std::string out_in_lydia_ax;
    std::string out_red_lydia;
    std::string out_in_aaltaf;
    std::string out_in_aaltaf_ax;
    std::string out_red_aaltaf;
    std::string out_in_powerdecl;
    std::string out_red_powerdecl;
    std::string out_name;
    bool verbose = false;

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help& ) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    } catch (args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    if (out_ltlf) {
        out_in_lydia = args::get(out_ltlf);
    }
    if (out_ltlfax) {
        out_in_lydia_ax = args::get(out_ltlfax);
    }
    if (out_ltlfred) {
        out_red_lydia = args::get(out_ltlfred);
    }
    if (out_aaltaf) {
        out_in_aaltaf = args::get(out_aaltaf);
    }
    if (out_aaltafax) {
        out_in_aaltaf_ax = args::get(out_aaltafax);
    }
    if (out_aaltafred) {
        out_red_aaltaf = args::get(out_aaltafred);
    }
    if (foo) {
        verbose = true;
    }
    if (out_pd) {
        out_in_powerdecl = args::get(out_pd);
    }
    if (out_pdred) {
        out_red_powerdecl = args::get(out_pdred);
    }
    if (out_namef) {
        out_name = args::get(out_namef);
    }
    if (in_filename) {
        auto file = std::filesystem::path(args::get(in_filename));
        auto folder = file.parent_path();
        auto fn = file.filename().string();
        auto reduced_file = folder / (file.stem().string() + "_reduced" + fn.substr(fn.find_last_of('.') + 1));
        reducing(out_name, verbose, file, out_in_lydia, out_in_lydia_ax, out_red_lydia, out_in_powerdecl, out_red_powerdecl, out_in_aaltaf, out_in_aaltaf_ax, out_red_aaltaf);
    } else {
        std::cerr << "ERROR: Missing input file!" << std::endl;
        return 1;
    }

    return 0;
}
