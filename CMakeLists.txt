cmake_minimum_required(VERSION 3.15)
project(ChessAI VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenMP REQUIRED)
find_package(BLAS)
find_package(LAPACK)

# Create source groups
set(SOURCES
    # Main
    src/main.cpp
    
    # Board
    src/board/board.cpp
    
    # Book
    src/book/book.cpp
    
    # Endgame Tablebases
    src/endgame/tablebases.cpp
    
    # Engine
    src/engine/engine.cpp
    src/engine/hypermodern_system.cpp
    src/engine/personality.cpp
    
    # Evaluation
    src/eval/evaluator.cpp
    
    # MCTS
    src/mcts/mcts.cpp
    
    # Move Generation
    src/movegen/movegen.cpp
    
    # Neural Network
    src/neural/neural_network.cpp
    
    # Search
    src/search/search.cpp
    
    # Utilities
    src/utils/move_generator.cpp
)

# Headers
set(HEADERS
    # Board
    include/board/board.hpp
    
    # Book
    include/book/book.hpp
    
    # Endgame Tablebases
    include/endgame/tablebases.hpp
    
    # Engine
    include/engine/engine.hpp
    
    # Evaluation
    include/eval/evaluator.hpp
    
    # MCTS
    include/mcts/mcts.hpp
    
    # Move Generation
    include/movegen/movegen.hpp
    
    # Neural Network
    include/neural/neural_network.hpp
    
    # Search
    include/search/search.hpp
    
    # Utilities
    include/utils/move_generator.hpp
)

# Add executable
add_executable(chess_engine ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(chess_engine 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(chess_engine 
    PRIVATE 
        OpenMP::OpenMP_CXX
)

# Add BLAS/LAPACK if found
if(BLAS_FOUND AND LAPACK_FOUND)
    target_link_libraries(chess_engine 
        PRIVATE 
            ${BLAS_LIBRARIES}
            ${LAPACK_LIBRARIES}
    )
endif()

# Enable warnings and optimizations
if(MSVC)
    target_compile_options(chess_engine PRIVATE /W4 /O2 /arch:AVX2)
else()
    target_compile_options(chess_engine PRIVATE -Wall -Wextra -O3 -march=native)
endif()

# Install rules
install(TARGETS chess_engine
    RUNTIME DESTINATION bin
)
