#pragma once

#include <memory>

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

class GameRouter;
class RecordService;

// Recordes (estado "record"): top-10 por pontuacao. ENTER/ESC -> menu.
class ForgeRecordScene final: public cengine::core::IScene
{
    std::shared_ptr<GameRouter>    m_gameRouter;
    std::shared_ptr<RecordService> m_recordService;

public:
    ForgeRecordScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<RecordService> recordService);

    void onEnter() override {}
    void update(cengine::core::Seconds) override {}
    void draw() override;
    void input() override;
    void onExit() override {}
};
