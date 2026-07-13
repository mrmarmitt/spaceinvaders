#include <gtest/gtest.h>

#include "spaceinvaders/game/World.h"

using si::World;
using si::WorldConfig;

namespace {

constexpr double kDt = 1.0 / 60.0;

// Avanca a simulacao por `seconds` em passos fixos de 1/60 (como o engine).
void simulate(World& world, const double seconds)
{
    const int steps = (int)(seconds / kDt);
    for (int i = 0; i < steps; ++i)
    {
        world.update(kDt);
    }
}

WorldConfig noBombs()
{
    WorldConfig config;
    config.bombsEnabled = false;
    return config;
}

} // namespace

TEST(WorldTest, EstadoInicial)
{
    const World world;

    EXPECT_EQ(world.outcome(), World::Outcome::Playing);
    EXPECT_EQ(world.score(), 0);
    EXPECT_EQ(world.lives(), World::kInitialLives);
    EXPECT_EQ(world.wave(), 1u);
    EXPECT_EQ(world.aliveCount(), World::kRows * World::kCols);
    EXPECT_FALSE(world.playerShotActive());
    EXPECT_EQ(world.bombCount(), 0u);
}

TEST(WorldTest, JogadorMoveERespeitaAsBordas)
{
    World world(noBombs());

    const float startX = world.playerRect().x;

    world.setMoveAxis(1.0f);
    simulate(world, 0.5);
    EXPECT_GT(world.playerRect().x, startX);

    // 10s para a direita: para na borda, nao sai da arena
    simulate(world, 10.0);
    const si::Rect right = world.playerRect();
    EXPECT_LE(right.x + right.w, World::kArenaW);

    // 20s para a esquerda: para na outra borda
    world.setMoveAxis(-1.0f);
    simulate(world, 20.0);
    EXPECT_GE(world.playerRect().x, 0.0f);
}

TEST(WorldTest, UmTiroPorVez)
{
    World world(noBombs());

    EXPECT_TRUE(world.fire());
    EXPECT_TRUE(world.playerShotActive());
    EXPECT_FALSE(world.fire()) << "segundo tiro com um em voo deve falhar";

    // o tiro sobe e (apos matar alguem ou sair) libera o gatilho
    simulate(world, 2.0);
    EXPECT_FALSE(world.playerShotActive());
    EXPECT_TRUE(world.fire());
}

TEST(WorldTest, TiroMataInvasorEPontua)
{
    World world(noBombs());

    // jogador nasce centrado sob a coluna 5; o tiro reto acerta o invasor
    // mais baixo dela (linha 4 = polvo, 10 pontos)
    EXPECT_TRUE(world.invaderAlive(4, 5));
    EXPECT_TRUE(world.fire());
    simulate(world, 2.0);

    EXPECT_FALSE(world.invaderAlive(4, 5));
    EXPECT_EQ(world.score(), 10);
    EXPECT_EQ(world.aliveCount(), World::kRows * World::kCols - 1);
}

TEST(WorldTest, PontuacaoPorTipo)
{
    EXPECT_EQ(World::invaderKind(0), si::InvaderKind::Squid);
    EXPECT_EQ(World::invaderKind(1), si::InvaderKind::Crab);
    EXPECT_EQ(World::invaderKind(2), si::InvaderKind::Crab);
    EXPECT_EQ(World::invaderKind(3), si::InvaderKind::Octopus);
    EXPECT_EQ(World::invaderKind(4), si::InvaderKind::Octopus);
}

TEST(WorldTest, OndaLimpaViraProximaOnda)
{
    World world(noBombs());

    const int scoreBefore = world.score();
    for (uint32_t row = 0; row < World::kRows; ++row)
    {
        for (uint32_t col = 0; col < World::kCols; ++col)
        {
            world.killInvader(row, col);
        }
    }
    EXPECT_EQ(world.aliveCount(), 0u);

    world.update(kDt); // fecha a onda

    EXPECT_EQ(world.wave(), 2u);
    EXPECT_EQ(world.aliveCount(), World::kRows * World::kCols) << "horda nova completa";
    EXPECT_EQ(world.score(), scoreBefore) << "pontuacao sobrevive a troca de onda";
    EXPECT_EQ(world.outcome(), World::Outcome::Playing);
}

TEST(WorldTest, HordaDesceAteInvadir)
{
    World world(noBombs());

    // um unico sobrevivente desce rapido (intervalo minimo da marcha);
    // sem ninguem atirando, a invasao e inevitavel e encerra a partida
    for (uint32_t row = 0; row < World::kRows; ++row)
    {
        for (uint32_t col = 0; col < World::kCols; ++col)
        {
            if (!(row == 0 && col == 0))
            {
                world.killInvader(row, col);
            }
        }
    }

    simulate(world, 300.0);

    EXPECT_EQ(world.outcome(), World::Outcome::GameOver);
    EXPECT_EQ(world.lives(), World::kInitialLives) << "invasao encerra sem gastar vidas";
}

TEST(WorldTest, BombasAtingemJogadorParadoAteOGameOver)
{
    // com bombas ligadas e o jogador parado no meio, cedo ou tarde as 3
    // vidas acabam (bem antes de a horda invadir: ela so desce ~8u a cada
    // travessia de ~50s no ritmo inicial)
    World world(WorldConfig{ 42, true });

    simulate(world, 240.0);

    EXPECT_EQ(world.outcome(), World::Outcome::GameOver);
    EXPECT_EQ(world.lives(), 0) << "game over por vidas, nao por invasao";
}

TEST(WorldTest, CooldownPosMorteSeguraOGatilho)
{
    World world(WorldConfig{ 7, true });

    // roda ate perder a primeira vida (max 120s de simulacao)
    bool hit = false;
    for (int i = 0; i < 120 * 60 && !hit; ++i)
    {
        world.update(kDt);
        hit = world.lives() < World::kInitialLives;
    }
    ASSERT_TRUE(hit) << "alguma bomba deveria ter acertado o jogador parado";

    EXPECT_TRUE(world.playerHitCooldown());
    EXPECT_FALSE(world.fire()) << "sem atirar durante o cooldown";

    simulate(world, 2.0);
    EXPECT_FALSE(world.playerHitCooldown());
}
