#include "ForgeMenuScene.h"

#include <cmath>
#include <utility>

#include "spaceinvaders/game/GameRouter.h"

#include "../ForgeSpriteUi.h"
#include "../ForgeUi.h"

namespace {
constexpr const char* kOptions[] = { "JOGAR", "RECORDES", "SAIR" };
constexpr int         kOptionCount = 3;
} // namespace

ForgeMenuScene::ForgeMenuScene(std::shared_ptr<GameRouter> gameRouter): m_gameRouter(std::move(gameRouter)) {}

void ForgeMenuScene::update(const cengine::core::Seconds dt) { m_elapsed += dt.count(); }

void ForgeMenuScene::draw()
{
    const float w = forgeui::screenWidth();
    const float h = forgeui::screenHeight();

    const float top = h * 0.42f;
    const float rowH = 56.0f;

    // cursor-invasor ao lado da opcao selecionada (sprite antes do texto)
    const bool frame1 = std::fmod(m_elapsed, 1.0) >= 0.5;
    const forgesprite::SpriteRegion& cursor = frame1 ? forgesprite::sprites::kCrab1 : forgesprite::sprites::kCrab0;
    const float                      textW = forgeui::textWidth(kOptions[m_selected], 30.0f);
    forgesprite::drawSprite(cursor, (w - textW) * 0.5f - 64.0f, top + m_selected * rowH, 3.0f, forgeui::color::kAccent);

    forgeui::drawTextCentered("S P A C E   I N V A D E R S", h * 0.18f, 44.0f, forgeui::color::kTitle);

    for (int i = 0; i < kOptionCount; ++i)
    {
        const uint32_t color = (i == m_selected) ? forgeui::color::kAccent : forgeui::color::kText;
        forgeui::drawTextCentered(kOptions[i], top + i * rowH, 30.0f, color);
    }

    forgeui::drawHints("SETAS navegar   ENTER confirmar");
}

void ForgeMenuScene::input()
{
    switch (forgeui::readKey().key)
    {
    case Key::Up:
        m_selected = (m_selected + kOptionCount - 1) % kOptionCount;
        break;
    case Key::Down:
        m_selected = (m_selected + 1) % kOptionCount;
        break;
    case Key::Enter:
        if (m_selected == 0)
            m_gameRouter->game();
        else if (m_selected == 1)
            m_gameRouter->record();
        else
            m_gameRouter->exit();
        break;
    case Key::Escape:
        m_gameRouter->exit();
        break;
    default:
        break;
    }
}
