#pragma once

#include <memory>

#include "spaceinvaders/game/GameRouter.h"
#include "StateGameFlow.h"

// Fluxo do Space Invaders:
//
//   initial (splash) -> menu -> game -> gameOver -> menu
//                        |  \-> record -> menu
//                        \--> exit
//
// Codigos de estado = chaves das factories de cena (ForgeSceneFactory).
// "exit" casa com cengine::routing::kExitStateCode — o frame() devolve
// false e o host encerra.

class InitialSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "initial"; }
    [[nodiscard]] std::string getName() const override { return "Splash"; }

    void menu(GameRouter& game) const override; // unica transicao valida
    void game(GameRouter&) const override {}
    void gameOver(GameRouter&) const override {}
    void record(GameRouter&) const override {}
    void exit(GameRouter&) const override {}
};

class MenuSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "menu"; }
    [[nodiscard]] std::string getName() const override { return "Menu"; }

    void menu(GameRouter&) const override {}
    void game(GameRouter& game) const override;
    void gameOver(GameRouter&) const override {}
    void record(GameRouter& game) const override;
    void exit(GameRouter& game) const override;
};

class GameSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "game"; }
    [[nodiscard]] std::string getName() const override { return "Game"; }

    void menu(GameRouter& game) const override; // abandono via ESC
    void game(GameRouter&) const override {}
    void gameOver(GameRouter& game) const override;
    void record(GameRouter&) const override {}
    void exit(GameRouter&) const override {}
};

class GameOverSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "gameOver"; }
    [[nodiscard]] std::string getName() const override { return "GameOver"; }

    void menu(GameRouter& game) const override;
    void game(GameRouter&) const override {}
    void gameOver(GameRouter&) const override {}
    void record(GameRouter&) const override {}
    void exit(GameRouter&) const override {}
};

class RecordSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "record"; }
    [[nodiscard]] std::string getName() const override { return "Record"; }

    void menu(GameRouter& game) const override;
    void game(GameRouter&) const override {}
    void gameOver(GameRouter&) const override {}
    void record(GameRouter&) const override {}
    void exit(GameRouter&) const override {}
};

class ExitSG final: public StateGameFlow
{
public:
    [[nodiscard]] std::string getCode() const override { return "exit"; }
    [[nodiscard]] std::string getName() const override { return "Exit"; }

    void menu(GameRouter&) const override {}
    void game(GameRouter&) const override {}
    void gameOver(GameRouter&) const override {}
    void record(GameRouter&) const override {}
    void exit(GameRouter&) const override {}
};

inline void InitialSG::menu(GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }

inline void MenuSG::game(GameRouter& game) const { game.setNextState(std::make_unique<GameSG>()); }
inline void MenuSG::record(GameRouter& game) const { game.setNextState(std::make_unique<RecordSG>()); }
inline void MenuSG::exit(GameRouter& game) const { game.setNextState(std::make_unique<ExitSG>()); }

inline void GameSG::menu(GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }
inline void GameSG::gameOver(GameRouter& game) const { game.setNextState(std::make_unique<GameOverSG>()); }

inline void GameOverSG::menu(GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }

inline void RecordSG::menu(GameRouter& game) const { game.setNextState(std::make_unique<MenuSG>()); }
