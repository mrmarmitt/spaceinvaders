#pragma once

#include <memory>
#include <string>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

#include "spaceinvaders/game/Record.h"

class GameRouter;
class PlaySession;
class RecordService;

// Game over (estado "gameOver"): mostra o resultado da PlaySession; se
// entrou no top-10, pede o nome e persiste o recorde (mesmo fluxo do
// ForgeGameOverScene do 8puzzle).
class ForgeGameOverScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter>    m_gameRouter;
    std::shared_ptr<PlaySession>   m_session;
    std::shared_ptr<RecordService> m_recordService;

    bool        m_isRecord = false;
    std::string m_name;

    [[nodiscard]] Record buildRecord() const;

public:
    ForgeGameOverScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<PlaySession> session,
                       std::shared_ptr<RecordService> recordService);

    void onEnter() override;
    void update(cengine::core::Seconds) override {}
    void draw() override;
    void input() override;
    void onExit() override {}
};
