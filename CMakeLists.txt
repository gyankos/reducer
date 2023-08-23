cmake_minimum_required(VERSION 3.22)
project(reducer)
set(CMAKE_CXX_STANDARD 23)

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1 # or a later release
)
FetchContent_MakeAvailable(Catch2)

add_subdirectory(submodules/yaucl)
include_directories(submodules/yaucl/include)
include_directories(submodules/yaucl/submodules/magic_enum/include)

add_library(reducerlib declare_cases.h declare_cases.cpp ltlf.cpp ltlf.h my_unordered_set.h model_reducer.cpp model_reducer.h)
target_link_libraries(reducerlib yaucl_hashing roaring antlr4_static yaucl_graph )

add_executable(reducer main.cpp)
target_link_libraries(reducer yaucl_hashing roaring antlr4_static yaucl_graph reducerlib)


add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain reducerlib)