#pragma once

#include <memory>

#include <cengine/routing/IRouter.hpp>
#include <cengine/routing/IState.hpp>

class StateGameFlow;

// Fachada de navegacao de dominio do Space Invaders sobre o roteador da
// cengine (mesmo desenho do GameRouter do 8puzzle): as cenas chamam
// menu()/game()/gameOver()/... que delegam a maquina de estados
// (StateGameFlow do estado atual) e agendam a proxima cena via
// setNextState().
class GameRouter final
{
    std::shared_ptr<cengine::routing::IRouter> m_router;

    static const StateGameFlow& castIt(const cengine::routing::IState& state);

public:
    explicit GameRouter(std::shared_ptr<cengine::routing::IRouter> router);

    // Agenda o proximo estado/cena (chamado pelas transicoes de StateGameFlow).
    void setNextState(std::unique_ptr<cengine::routing::IState> state) const;

    void menu();
    void game();
    void gameOver();
    void record();
    void exit();
};
