cmake_minimum_required(VERSION 3.10)
project(ProyectoParalela)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

include_directories(include src)

add_executable(main
    src/main.cpp
    src/csv_parser.cpp
    src/processor.cpp
)

target_link_libraries(main OpenMP::OpenMP_CXX)

# Unit tests
add_executable(test_csv_parser
    tests/unit/test_csv_parser.cpp
    src/csv_parser.cpp
)

add_executable(test_processor
    tests/unit/test_processor.cpp
    src/model.hpp
    src/processor.cpp
    src/csv_parser.cpp
)

target_link_libraries(test_processor OpenMP::OpenMP_CXX)

# Integration test
add_executable(test_integration
    tests/integration/test_integration.cpp
    src/csv_parser.cpp
    src/processor.cpp
)

target_link_libraries(test_integration OpenMP::OpenMP_CXX)
