#pragma once

#include <cengine/routing/IState.hpp>

class GameRouter;

// Estado da maquina de fluxo do jogo (mesmo desenho do 8puzzle): estados
// sem dados, cada transicao valida despacha a proxima cena via GameRouter;
// transicoes invalidas sao no-ops explicitos.
class StateGameFlow: public cengine::routing::IState
{
public:
    StateGameFlow() = default;
    ~StateGameFlow() override = default;

    [[nodiscard]] std::string getCode() const override = 0;
    [[nodiscard]] std::string getName() const override = 0;

    virtual void menu(GameRouter& game) const = 0;
    virtual void game(GameRouter& game) const = 0;
    virtual void gameOver(GameRouter& game) const = 0;
    virtual void record(GameRouter& game) const = 0;
    virtual void exit(GameRouter& game) const = 0;
};
