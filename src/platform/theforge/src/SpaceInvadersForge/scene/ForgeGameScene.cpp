#include "ForgeGameScene.h"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <utility>

#include "spaceinvaders/game/GameRouter.h"
#include "spaceinvaders/game/service/PlaySession.h"

#include "../ForgeSpriteUi.h"
#include "../ForgeUi.h"

namespace {

// Regiao + tint por tipo/pose (o atlas e branco; o tint e a cor).
const forgesprite::SpriteRegion& regionFor(const si::InvaderKind kind, const bool frame1)
{
    switch (kind)
    {
    case si::InvaderKind::Squid:
        return frame1 ? forgesprite::sprites::kSquid1 : forgesprite::sprites::kSquid0;
    case si::InvaderKind::Crab:
        return frame1 ? forgesprite::sprites::kCrab1 : forgesprite::sprites::kCrab0;
    case si::InvaderKind::Octopus:
    default:
        return frame1 ? forgesprite::sprites::kOcto1 : forgesprite::sprites::kOcto0;
    }
}

uint32_t tintFor(const si::InvaderKind kind)
{
    switch (kind)
    {
    case si::InvaderKind::Squid:
        return forgeui::color::kText;
    case si::InvaderKind::Crab:
        return forgeui::color::kAccent;
    case si::InvaderKind::Octopus:
    default:
        return forgeui::color::kTitle;
    }
}

uint32_t worldSeed()
{
    // partida diferente a cada execucao; os testes usam seed fixa
    return (uint32_t)std::time(nullptr);
}

si::WorldConfig playConfig()
{
    si::WorldConfig config;
    config.seed = worldSeed();
    return config;
}

} // namespace

ForgeGameScene::ForgeGameScene(std::shared_ptr<GameRouter> gameRouter, std::shared_ptr<PlaySession> session):
    m_gameRouter(std::move(gameRouter)), m_session(std::move(session)), m_world(playConfig())
{
}

void ForgeGameScene::update(const cengine::core::Seconds dt)
{
    m_elapsed += dt.count();
    m_world.update(dt.count());

    if (m_world.outcome() == si::World::Outcome::GameOver && !m_gameOverReported)
    {
        m_gameOverReported = true;
        m_session->setResult(m_world.score(), m_world.wave());
        m_gameRouter->gameOver();
    }
}

void ForgeGameScene::input()
{
    const KeyEvent event = forgeui::readKey();
    if (event.key == Key::Escape)
    {
        m_gameRouter->menu(); // abandona a partida
        return;
    }

    m_world.setMoveAxis(forgeui::moveAxis());

    // gatilho na borda de subida do ESPACO (segurar nao metralha; o World
    // ainda limita a um tiro em voo, como no arcade)
    const bool fireNow = forgeui::fireHeld();
    if (fireNow && !m_firePrev)
    {
        m_world.fire();
    }
    m_firePrev = fireNow;
}

void ForgeGameScene::draw()
{
    using si::World;

    const float w = forgeui::screenWidth();
    const float h = forgeui::screenHeight();

    // arena projetada com escala INTEIRA (texel = bloco exato de pixels) e
    // centrada; sobra vira moldura
    const float scale = std::max(1.0f, std::floor(std::min(w * 0.95f / World::kArenaW, h * 0.92f / World::kArenaH)));
    const float ox = (w - World::kArenaW * scale) * 0.5f;
    const float oy = (h - World::kArenaH * scale) * 0.5f;

    auto toScreenX = [&](float x) { return ox + x * scale; };
    auto toScreenY = [&](float y) { return oy + y * scale; };

    // --- sprites (tudo em UM draw call; o texto do HUD vem por cima) ---
    const bool frame1 = m_world.animFrame();
    for (uint32_t row = 0; row < World::kRows; ++row)
    {
        const si::InvaderKind kind = World::invaderKind(row);
        for (uint32_t col = 0; col < World::kCols; ++col)
        {
            if (!m_world.invaderAlive(row, col))
                continue;
            const si::Rect rect = m_world.invaderRect(row, col);
            forgesprite::drawSprite(regionFor(kind, frame1), toScreenX(rect.x), toScreenY(rect.y), scale, tintFor(kind));
        }
    }

    // jogador (pisca durante o cooldown pos-morte)
    if (!m_world.playerHitCooldown() || std::fmod(m_elapsed, 0.2) < 0.1)
    {
        const si::Rect player = m_world.playerRect();
        forgesprite::drawSprite(forgesprite::sprites::kPlayer, toScreenX(player.x), toScreenY(player.y), scale,
                                forgeui::color::kSuccess);
    }

    if (m_world.playerShotActive())
    {
        const si::Rect shot = m_world.playerShotRect();
        forgesprite::drawSprite(forgesprite::sprites::kShot, toScreenX(shot.x), toScreenY(shot.y), scale, forgeui::color::kText);
    }

    for (uint32_t i = 0; i < m_world.bombCount(); ++i)
    {
        const si::Rect bomb = m_world.bombRect(i);
        forgesprite::drawSprite(forgesprite::sprites::kShot, toScreenX(bomb.x), toScreenY(bomb.y), scale, forgeui::color::kValue);
    }

    // vidas restantes como mini-canhoes no rodape da arena
    const float lifeScale = std::max(1.0f, scale * 0.66f);
    for (int i = 0; i < m_world.lives(); ++i)
    {
        forgesprite::drawSprite(forgesprite::sprites::kPlayer, toScreenX(4.0f) + i * (16.0f * lifeScale),
                                toScreenY(World::kArenaH) + 8.0f, lifeScale, forgeui::color::kSuccess);
    }

    // --- HUD em texto (forca o flush; fica por cima dos sprites) ---
    char hud[96];
    snprintf(hud, sizeof(hud), "PONTOS  %05d", m_world.score());
    forgeui::drawText(hud, toScreenX(4.0f), oy - 34.0f, 24.0f, forgeui::color::kValue);

    char waveText[32];
    snprintf(waveText, sizeof(waveText), "ONDA  %u", m_world.wave());
    forgeui::drawText(waveText, toScreenX(World::kArenaW) - forgeui::textWidth(waveText, 24.0f), oy - 34.0f, 24.0f,
                      forgeui::color::kText);

    forgeui::drawHints("SETAS mover   ESPACO atirar   ESC abandonar");
}
