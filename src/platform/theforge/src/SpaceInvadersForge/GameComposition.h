#pragma once

#include <memory>

// Contrato entre o casco da plataforma e o composition root: o casco
// (SpaceInvadersForge::Init) pede o jogo montado no momento certo do boot;
// QUEM monta — todas as instancias e injecoes (dominio, servicos, cenas,
// engine) — e o src/main_theforge.cpp, fora da plataforma e do pacote do
// jogo, no mesmo padrao dos mains do 8puzzle.

namespace cengine::core {
class EngineManager;
}
namespace cengine::routing {
class RouterInMemory;
}

struct GameComposition
{
    std::shared_ptr<cengine::routing::RouterInMemory> router;
    std::unique_ptr<cengine::core::EngineManager>     engine;
};

GameComposition composeSpaceInvadersGame();
