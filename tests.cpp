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
                auto M = streamDeclare(file);
                for (const auto& clause : M)
                    std::cout << clause << std::endl;
                std::cout << std::endl << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl << std::endl;
                std::cout << "Final model:" << std::endl;
                std::vector<DatalessCases> v = model_reducer{}.run(M);
                for (const auto& clause : v)
                    std::cout << clause << std::endl;
                std::cout << std::endl << "~~~~~~~~~~~~~~~~~~~~~~" << std::endl << std::endl;
                std::ifstream expected_file{expected};
                std::vector<DatalessCases> E;
                if (exists(expected)) {
                    E = streamDeclare(expected_file);
                }
                for (const auto& e : E) {
                    REQUIRE(std::find(v.begin(), v.end(), e) != v.end());
                }
            }
        }
    }
}