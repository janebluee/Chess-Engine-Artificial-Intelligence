#include "../../include/engine/engine.hpp"
#include <random>

class PersonalityManager {
public:
    enum Style {
        AGGRESSIVE,
        POSITIONAL,
        TACTICAL,
        DYNAMIC,
        SOLID
    };

    PersonalityManager() {
        loadOpeningBooks();
        initializeStyles();
    }

    void setStyle(Style style) {
        currentStyle = style;
        updateParameters();
    }

    void adaptToOpponent(const std::string& opponentHistory) {
        analyzePlayingStyle(opponentHistory);
        adjustCounterStrategy();
    }

    float getAggressionFactor() const {
        return styleParams.aggressionFactor;
    }

    float getPositionalWeight() const {
        return styleParams.positionalWeight;
    }

    bool shouldSacrifice(int materialDiff, float compensation) {
        return materialDiff <= 3 && compensation > 0.8f &&
               (currentStyle == AGGRESSIVE || currentStyle == TACTICAL);
    }

    std::string getNextBookMove(const std::string& fen) {
        if (openingBook.contains(fen)) {
            auto moves = openingBook[fen];
            if (currentStyle == AGGRESSIVE) {
                return selectAggressiveMove(moves);
            } else if (currentStyle == POSITIONAL) {
                return selectPositionalMove(moves);
            }
            return selectRandomMove(moves);
        }
        return "";
    }

private:
    Style currentStyle;
    std::unordered_map<std::string, std::vector<std::string>> openingBook;
    
    struct StyleParameters {
        float aggressionFactor;
        float positionalWeight;
        float tacticalThreshold;
        float riskTolerance;
        int searchDepthBonus;
    } styleParams;

    void loadOpeningBooks() {
        loadMainBook();
        loadSpecializedBooks();
    }

    void initializeStyles() {
        styleProfiles[AGGRESSIVE] = {1.2f, 0.7f, 0.6f, 1.3f, 1};
        styleProfiles[POSITIONAL] = {0.8f, 1.3f, 0.8f, 0.7f, 2};
        styleProfiles[TACTICAL] = {1.1f, 0.8f, 1.2f, 1.1f, 1};
        styleProfiles[DYNAMIC] = {1.0f, 1.0f, 1.0f, 1.0f, 0};
        styleProfiles[SOLID] = {0.7f, 1.2f, 0.9f, 0.6f, 2};
    }

    void updateParameters() {
        styleParams = styleProfiles[currentStyle];
        adjustForTimeControl();
        adjustForTournamentSituation();
    }

    void analyzePlayingStyle(const std::string& history) {
        std::vector<GameAnalysis> games = parseGames(history);
        OpponentProfile profile = createProfile(games);
        weaknesses = findWeaknesses(profile);
    }

    void adjustCounterStrategy() {
        if (weaknesses.pawnStructure) {
            styleParams.positionalWeight *= 1.2f;
        }
        if (weaknesses.tacticalAwareness) {
            styleParams.tacticalThreshold *= 0.8f;
        }
        if (weaknesses.timeManagement) {
            styleParams.searchDepthBonus += 1;
        }
    }

    std::string selectAggressiveMove(const std::vector<std::string>& moves) {
        auto scored_moves = scoreMovesByAggression(moves);
        return selectTopMove(scored_moves);
    }

    std::string selectPositionalMove(const std::vector<std::string>& moves) {
        auto scored_moves = scoreMovesByPosition(moves);
        return selectTopMove(scored_moves);
    }
};
