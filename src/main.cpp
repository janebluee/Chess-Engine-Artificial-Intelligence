#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <stdexcept>
#include "../include/engine/engine.hpp"

namespace {
    void printEngineInfo() {
        std::cout << "id name Chess AI Engine" << std::endl;
        std::cout << "id author janebluee" << std::endl;
        std::cout << "uciok" << std::endl;
    }
}

int main() {
    try {
        ChessEngine engine;
        engine.init();
        
        std::cout << "Chess AI Engine initialized successfully!" << std::endl;
        std::cout << "Type 'quit' to exit" << std::endl;
        
        std::string command;
        while (std::getline(std::cin, command)) {
            if (command == "quit") {
                break;
            }
            
            try {
                if (command == "uci") {
                    printEngineInfo();
                }
                else if (command == "isready") {
                    std::cout << "readyok" << std::endl;
                }
                else if (command.substr(0, 8) == "position") {
                    engine.setPosition(command);
                }
                else if (command.substr(0, 2) == "go") {
                    std::string bestMove = engine.getBestMove(command);
                    std::cout << "bestmove " << bestMove << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error processing command '" << command << "': " << e.what() << std::endl;
            }
        }
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
