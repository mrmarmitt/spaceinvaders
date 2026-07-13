#pragma once

#include <cstdint>

// O dominio do Space Invaders (degrau 5 da PoC): simulacao completa de uma
// partida — jogador, horda, tiro, bombas, colisao AABB, ondas, pontuacao e
// vidas — em C++ puro, sem nenhuma dependencia de plataforma ou da cengine.
// A cena da plataforma traduz input em comandos, chama update(dt) com o
// passo fixo do engine e desenha lendo as consultas; os testes (CMake, sem
// The-Forge) dirigem a simulacao do mesmo jeito.
//
// Coordenadas: arena virtual de 224x256 (a resolucao interna do arcade
// original — os sprites de 8x8..13x8 texels ficam em escala 1:1). Origem no
// canto superior esquerdo, Y crescendo para baixo; quem projeta para a tela
// e a cena.

namespace si {

struct Rect
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    [[nodiscard]] bool intersects(const Rect& other) const
    {
        return x < other.x + other.w && other.x < x + w && y < other.y + other.h && other.y < y + h;
    }
};

enum class InvaderKind : uint8_t
{
    Squid,   // linha 0 — 30 pontos
    Crab,    // linhas 1-2 — 20 pontos
    Octopus, // linhas 3-4 — 10 pontos
};

struct WorldConfig
{
    uint32_t seed = 1;        // RNG das bombas (deterministico p/ testes)
    bool bombsEnabled = true; // testes de mira/marcha desligam as bombas
};

class World
{
public:
    static constexpr float    kArenaW = 224.0f;
    static constexpr float    kArenaH = 256.0f;
    static constexpr uint32_t kRows = 5;
    static constexpr uint32_t kCols = 11;
    static constexpr uint32_t kMaxBombs = 3;
    static constexpr int      kInitialLives = 3;

    enum class Outcome
    {
        Playing,
        GameOver,
    };

    explicit World(WorldConfig config = {});

    // --- comandos (fase input da cena) ---

    /// Direcao do canhao neste quadro: -1 esquerda, 0 parado, +1 direita.
    void setMoveAxis(float axis);

    /// Dispara se nao ha tiro em voo (regra do arcade: um por vez) e o
    /// jogador nao esta no cooldown pos-morte. Retorna se o tiro saiu.
    bool fire();

    // --- simulacao (fase update, dt fixo do engine) ---
    void update(double dt);

    // --- consultas (fase draw e testes) ---
    [[nodiscard]] Outcome  outcome() const { return m_outcome; }
    [[nodiscard]] int      score() const { return m_score; }
    [[nodiscard]] int      lives() const { return m_lives; }
    [[nodiscard]] uint32_t wave() const { return m_wave; }

    [[nodiscard]] Rect playerRect() const;
    [[nodiscard]] bool playerHitCooldown() const { return m_hitCooldown > 0.0; }

    [[nodiscard]] bool playerShotActive() const { return m_shotActive; }
    [[nodiscard]] Rect playerShotRect() const;

    [[nodiscard]] uint32_t bombCount() const { return m_bombCount; }
    [[nodiscard]] Rect     bombRect(uint32_t index) const;

    [[nodiscard]] bool                  invaderAlive(uint32_t row, uint32_t col) const;
    [[nodiscard]] Rect                  invaderRect(uint32_t row, uint32_t col) const;
    [[nodiscard]] static InvaderKind    invaderKind(uint32_t row);
    [[nodiscard]] uint32_t              aliveCount() const { return m_aliveCount; }

    /// Pose da animacao (troca a cada passo da marcha, como no arcade).
    [[nodiscard]] bool animFrame() const { return (m_steps & 1u) != 0; }

    /// Gancho de teste (e futura regra de tiro certeiro de debug).
    void killInvader(uint32_t row, uint32_t col);

private:
    void   resetHorde();
    void   stepHorde();
    void   updatePlayerShot(double dt);
    void   updateBombs(double dt);
    void   spawnBomb();
    double stepInterval() const;
    float  rand01();

    WorldConfig m_config;
    uint32_t    m_rng = 1;

    Outcome  m_outcome = Outcome::Playing;
    int      m_score = 0;
    int      m_lives = kInitialLives;
    uint32_t m_wave = 1;

    float  m_playerX = 0.0f;
    float  m_moveAxis = 0.0f;
    double m_hitCooldown = 0.0;

    bool  m_shotActive = false;
    float m_shotX = 0.0f;
    float m_shotY = 0.0f;

    bool     m_alive[kRows * kCols] = {};
    uint32_t m_aliveCount = kRows * kCols;
    float    m_hordeX = 0.0f;
    float    m_hordeY = 0.0f;
    float    m_dir = 1.0f;
    double   m_stepTimer = 0.0;
    uint32_t m_steps = 0;

    uint32_t m_bombCount = 0;
    float    m_bombX[kMaxBombs] = {};
    float    m_bombY[kMaxBombs] = {};
    double   m_bombTimer = 0.0;
};

} // namespace si
