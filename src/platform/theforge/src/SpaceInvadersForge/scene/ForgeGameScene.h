#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

#include "spaceinvaders/game/World.h"

class GameRouter;
class PlaySession;

// A partida (estado "game"): traduz input em comandos do World, avanca a
// simulacao com o dt fixo do engine e desenha a arena 224x256 projetada na
// tela (escala inteira, pixels crisp). No game over do World, grava o
// resultado na PlaySession e navega para a cena de game over.
class ForgeGameScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter>  m_gameRouter;
    std::shared_ptr<PlaySession> m_session;

    si::World m_world;
    bool      m_firePrev = false;
    bool      m_gameOverReported = false;
    double    m_elapsed = 0.0;

public:
    ForgeGameScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<PlaySession> session);

    void onEnter() override {}
    void update(cengine::core::Seconds dt) override;
    void draw() override;
    void input() override;
    void onExit() override {}
};
