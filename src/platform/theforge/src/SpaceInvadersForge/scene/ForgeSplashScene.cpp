#include "ForgeSplashScene.h"

#include <cmath>
#include <utility>

#include "spaceinvaders/game/GameRouter.h"

#include "../ForgeSpriteUi.h"
#include "../ForgeUi.h"

ForgeSplashScene::ForgeSplashScene(std::shared_ptr<GameRouter> gameRouter): m_gameRouter(std::move(gameRouter)) {}

void ForgeSplashScene::update(const cengine::core::Seconds dt) { m_elapsed += dt.count(); }

void ForgeSplashScene::draw()
{
    const float w = forgeui::screenWidth();
    const float h = forgeui::screenHeight();

    // tabela de pontos classica (sprites primeiro; o texto forca o flush)
    const float scale = 3.0f;
    const bool  frame1 = std::fmod(m_elapsed, 1.0) >= 0.5;

    struct Row
    {
        const forgesprite::SpriteRegion& region;
        uint32_t                         tint;
        const char*                      label;
    };
    const Row rows[] = {
        { frame1 ? forgesprite::sprites::kSquid1 : forgesprite::sprites::kSquid0, forgeui::color::kText, "= 30 PONTOS" },
        { frame1 ? forgesprite::sprites::kCrab1 : forgesprite::sprites::kCrab0, forgeui::color::kAccent, "= 20 PONTOS" },
        { frame1 ? forgesprite::sprites::kOcto1 : forgesprite::sprites::kOcto0, forgeui::color::kTitle, "= 10 PONTOS" },
    };

    const float tableTop = h * 0.40f;
    const float rowH = 52.0f;
    for (uint32_t i = 0; i < 3; ++i)
    {
        const Row& row = rows[i];
        forgesprite::drawSprite(row.region, w * 0.5f - 90.0f - row.region.w * scale * 0.5f, tableTop + i * rowH, scale, row.tint);
    }

    forgeui::drawTextCentered("S P A C E   I N V A D E R S", h * 0.16f, 56.0f, forgeui::color::kTitle);

    for (uint32_t i = 0; i < 3; ++i)
    {
        forgeui::drawText(rows[i].label, w * 0.5f - 40.0f, tableTop + i * rowH, 24.0f, forgeui::color::kValue);
    }

    // piscar dirigido pelo tempo de simulacao, como no splash do 8puzzle
    if (std::fmod(m_elapsed, 1.2) < 0.8)
    {
        forgeui::drawTextCentered("Pressione ENTER para comecar", h * 0.78f, 24.0f, forgeui::color::kValue);
    }

    forgeui::drawHints("ENTER comecar");
}

void ForgeSplashScene::input()
{
    switch (forgeui::readKey().key)
    {
    case Key::Enter:
    case Key::Escape:
        m_gameRouter->menu(); // sair so existe a partir do menu
        break;
    default:
        break;
    }
}
