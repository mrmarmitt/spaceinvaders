#include "ForgeRecordScene.h"

#include <cstdio>
#include <utility>

#include "spaceinvaders/game/GameRouter.h"
#include "spaceinvaders/game/service/RecordService.h"

#include "../ForgeUi.h"

ForgeRecordScene::ForgeRecordScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<RecordService> recordService):
    m_gameRouter(std::move(gameRouter)), m_recordService(std::move(recordService))
{
}

void ForgeRecordScene::draw()
{
    const float h = forgeui::screenHeight();

    forgeui::drawTextCentered("RECORDES", h * 0.12f, 44.0f, forgeui::color::kTitle);

    const auto records = m_recordService->listByScore();
    if (records.empty())
    {
        forgeui::drawTextCentered("Nenhum recorde ainda - va derrubar uns invasores!", h * 0.45f, 24.0f, forgeui::color::kDim);
    }
    else
    {
        const float top = h * 0.12f + 80.0f;
        const float rowH = 38.0f;
        const size_t count = records.size() < 10 ? records.size() : 10;
        for (size_t i = 0; i < count; ++i)
        {
            const Record& record = records[i];
            char          line[160];
            snprintf(line, sizeof(line), "%2u.  %-20s %6d pts   onda %u   %s", (unsigned)(i + 1), record.getName().c_str(),
                     record.getScore(), record.getWave(), record.getPlayedAt().c_str());
            const uint32_t color = (i == 0) ? forgeui::color::kValue : forgeui::color::kText;
            forgeui::drawTextCentered(line, top + (float)i * rowH, 22.0f, color);
        }
    }

    forgeui::drawHints("ENTER/ESC voltar ao menu");
}

void ForgeRecordScene::input()
{
    switch (forgeui::readKey().key)
    {
    case Key::Enter:
    case Key::Escape:
        m_gameRouter->menu();
        break;
    default:
        break;
    }
}
