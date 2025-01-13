cmake_minimum_required(VERSION 3.15)
project(ChessAI VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

add_executable(chess_engine
    src/main.cpp
    src/engine/engine.cpp
    src/board/board.cpp
    src/search/search.cpp
    src/eval/evaluator.cpp
    src/neural/network.cpp
    src/mcts/mcts.cpp
    src/utils/move_generator.cpp
)

target_include_directories(chess_engine PRIVATE include)
target_link_libraries(chess_engine PRIVATE OpenMP::OpenMP_CXX ${BLAS_LIBRARIES} ${LAPACK_LIBRARIES})

if(MSVC)
    target_compile_options(chess_engine PRIVATE /O2 /W4)
else()
    target_compile_options(chess_engine PRIVATE -O3 -march=native -Wall -Wextra)
endif()
