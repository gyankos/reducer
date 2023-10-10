cmake_minimum_required(VERSION 3.22)
project(reducer)
set(CMAKE_CXX_STANDARD 23)


set(CMAKE_CXX_FLAGS_RELEASE "-O3")
Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1 # or a later release
)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

include_directories(include)
add_subdirectory(submodules/yaucl)
include_directories(submodules/yaucl/include)
include_directories(submodules/args/)
include_directories(submodules/yaucl/submodules/magic_enum/include)
include_directories(submodules/yaucl/submodules/CRoaring/)
include_directories(submodules/yaucl/submodules/CRoaring/cpp)
include_directories(submodules/yaucl/submodules/CRoaring/include)

add_library(reducerlib include/declare_cases.h src/declare_cases.cpp src/ltlf.cpp include/ltlf.h include/my_unordered_set.h src/model_reducer.cpp include/model_reducer.h src/declare_to_lydialtlf.cpp include/declare_to_lydialtlf.h src/declare_to_powerdecl.cpp include/declare_to_powerdecl.h)
target_link_libraries(reducerlib yaucl_hashing roaring antlr4_static yaucl_graph )

add_executable(reducer main.cpp)
target_link_libraries(reducer yaucl_hashing roaring antlr4_static yaucl_graph reducerlib)

add_executable(to_automaton to_automaton.cpp)
target_link_libraries(to_automaton yaucl_hashing roaring antlr4_static yaucl_graph reducerlib)


add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain reducerlib)
catch_discover_tests(tests)
