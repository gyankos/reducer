#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include "declare_cases.h"
#include "model_reducer.h"

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