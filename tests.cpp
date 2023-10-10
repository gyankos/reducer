/*
 * tests.cpp
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

 

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <declare_cases.h>
#include <model_reducer.h>

TEST_CASE( "file-based testing" ) {
    auto path = std::filesystem::current_path().parent_path() / "tests";
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path)){
        if (dirEntry.is_directory()) {
            SECTION(dirEntry.path().filename().string()) {
                std::cout << dirEntry.path().filename().string() << std::endl;
                auto expected = dirEntry.path() / "expected.txt";
                auto model = dirEntry.path() / "model.txt";
                std::ifstream file{model};
                yaucl::structures::any_to_uint_bimap<std::string> bijection;
                auto M = streamDeclare(file, bijection);
                std::ifstream expected_file{expected};
                std::vector<DatalessCases> E;
                if (exists(expected)) {
                    E = streamDeclare(expected_file, bijection);
                }
                DeclareStraightforwardPrinter printer{bijection};
                for (const auto& clause : M) {
                    printer.to_print = &clause;
                    std::cout << printer << std::endl;
                }
                std::cout << std::endl << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl << std::endl;
                std::cout << "Final model:" << std::endl;
                std::vector<DatalessCases> v = model_reducer{}.run(M);
                for (const auto& clause : v){
                    printer.to_print = &clause;
                    std::cout << printer << std::endl;
                }
                std::cout << std::endl << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl << std::endl;
                for (const auto& e : E) {
                    REQUIRE(std::find(v.begin(), v.end(), e) != v.end());
                }
                for (const auto& c : v) {
                    REQUIRE(std::find(E.begin(), E.end(), c) != E.end());
                }
            }
        }
    }
}
