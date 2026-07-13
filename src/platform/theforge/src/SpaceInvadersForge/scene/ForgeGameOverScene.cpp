#include "ForgeGameOverScene.h"

#include <utility>

#include "spaceinvaders/game/GameRouter.h"
#include "spaceinvaders/game/service/PlaySession.h"
#include "spaceinvaders/game/service/RecordService.h"

#include "../ForgeUi.h"

ForgeGameOverScene::ForgeGameOverScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<PlaySession> session,
                                       std::shared_ptr<RecordService> recordService):
    m_gameRouter(std::move(gameRouter)), m_session(std::move(session)), m_recordService(std::move(recordService))
{
}

void ForgeGameOverScene::onEnter() { m_isRecord = m_recordService->isNewRecord(buildRecord(), 10); }

void ForgeGameOverScene::draw()
{
    const float h = forgeui::screenHeight();

    forgeui::drawTextCentered("FIM DE JOGO", h * 0.18f, 48.0f, forgeui::color::kTitle);

    const std::string stats =
        "Pontuacao: " + std::to_string(m_session->score()) + "     Onda alcancada: " + std::to_string(m_session->wave());
    forgeui::drawTextCentered(stats, h * 0.18f + 72.0f, 26.0f, forgeui::color::kValue);

    std::string hintText;
    if (m_isRecord)
    {
        forgeui::drawTextCentered("* NOVO RECORDE! *", h * 0.50f, 30.0f, forgeui::color::kValue);
        forgeui::drawTextCentered("Digite seu nome:", h * 0.50f + 48.0f, 22.0f, forgeui::color::kText);
        forgeui::drawTextCentered("[ " + m_name + "_ ]", h * 0.50f + 84.0f, 26.0f, forgeui::color::kAccent);
        hintText = "digite o nome   BACKSPACE apagar   ENTER confirmar";
    }
    else
    {
        forgeui::drawTextCentered("Sem recorde desta vez - tente de novo!", h * 0.53f, 22.0f, forgeui::color::kDim);
        hintText = "ENTER voltar ao menu";
    }

    forgeui::drawHints(hintText);
}

void ForgeGameOverScene::input()
{
    const KeyEvent event = forgeui::readKey();

    if (!m_isRecord)
    {
        if (event.key == Key::Enter || event.key == Key::Escape)
        {
            m_gameRouter->menu();
        }
        return;
    }

    switch (event.key)
    {
    case Key::Enter:
    {
        Record record = buildRecord();
        record.assignRecord(m_name.empty() ? "Anonimo" : m_name, nowAsString());
        m_recordService->addRecord(record);
        m_gameRouter->menu();
        break;
    }
    case Key::Backspace:
        if (!m_name.empty())
        {
            m_name.pop_back();
        }
        break;
    case Key::Char:
        if (m_name.size() < 20)
        {
            m_name.push_back(event.character);
        }
        break;
    default:
        break;
    }
}

Record ForgeGameOverScene::buildRecord() const { return { m_session->score(), m_session->wave() }; }
