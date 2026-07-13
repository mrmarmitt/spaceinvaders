#include "World.h"

#include <algorithm>

namespace si {

namespace {

constexpr float kPlayerW = 13.0f;
constexpr float kPlayerH = 8.0f;
constexpr float kPlayerY = World::kArenaH - 24.0f; // 232
constexpr float kPlayerSpeed = 75.0f;              // unidades/s
constexpr float kPlayerMarginX = 4.0f;

constexpr float kShotW = 3.0f;
constexpr float kShotH = 7.0f;
constexpr float kShotSpeed = 150.0f;

constexpr float kBombW = 3.0f;
constexpr float kBombH = 7.0f;
constexpr float kBombSpeed = 55.0f;

constexpr float kCellW = 16.0f;
constexpr float kCellH = 12.0f;
constexpr float kHordeLeft0 = (World::kArenaW - World::kCols * kCellW) * 0.5f; // 24
constexpr float kHordeTop0 = 48.0f;
constexpr float kHordeTopPerWave = 8.0f; // cada onda comeca mais baixa...
constexpr float kHordeTopMax = 88.0f;    // ...ate um teto (como no arcade)
constexpr float kHordeStepX = 2.0f;
constexpr float kHordeDropY = 8.0f;
constexpr float kHordeMarginX = 8.0f;

constexpr double kHitCooldownSeconds = 1.5;

constexpr float kInvaderH = 8.0f;

float invaderWidth(const InvaderKind kind)
{
    switch (kind)
    {
    case InvaderKind::Squid:
        return 8.0f;
    case InvaderKind::Crab:
        return 11.0f;
    case InvaderKind::Octopus:
    default:
        return 12.0f;
    }
}

int invaderPoints(const InvaderKind kind)
{
    switch (kind)
    {
    case InvaderKind::Squid:
        return 30;
    case InvaderKind::Crab:
        return 20;
    case InvaderKind::Octopus:
    default:
        return 10;
    }
}

} // namespace

World::World(WorldConfig config): m_config(config), m_rng(config.seed != 0 ? config.seed : 1)
{
    m_playerX = (kArenaW - kPlayerW) * 0.5f;
    resetHorde();
}

void World::setMoveAxis(const float axis) { m_moveAxis = std::clamp(axis, -1.0f, 1.0f); }

bool World::fire()
{
    if (m_outcome != Outcome::Playing || m_shotActive || m_hitCooldown > 0.0)
    {
        return false;
    }
    m_shotActive = true;
    m_shotX = m_playerX + (kPlayerW - kShotW) * 0.5f;
    m_shotY = kPlayerY - kShotH;
    return true;
}

void World::update(const double dt)
{
    if (m_outcome != Outcome::Playing)
    {
        return;
    }

    // jogador (pode se mover durante o cooldown pos-morte; so nao atira)
    m_hitCooldown = std::max(0.0, m_hitCooldown - dt);
    m_playerX = std::clamp(m_playerX + m_moveAxis * kPlayerSpeed * (float)dt, kPlayerMarginX, kArenaW - kPlayerMarginX - kPlayerW);

    updatePlayerShot(dt);

    // marcha em passos discretos (mesma mecanica provada no degrau 4)
    m_stepTimer += dt;
    while (m_aliveCount > 0 && m_stepTimer >= stepInterval())
    {
        m_stepTimer -= stepInterval();
        stepHorde();
    }

    updateBombs(dt);

    // invasao: qualquer invasor vivo alcancando a linha do jogador encerra
    // a partida na hora, independente das vidas (regra do arcade)
    for (uint32_t row = 0; row < kRows; ++row)
    {
        for (uint32_t col = 0; col < kCols; ++col)
        {
            if (m_alive[row * kCols + col] && m_hordeY + row * kCellH + kCellH >= kPlayerY)
            {
                m_outcome = Outcome::GameOver;
                return;
            }
        }
    }

    // onda limpa: proxima horda (mais baixa), pontuacao e vidas seguem
    if (m_aliveCount == 0)
    {
        ++m_wave;
        resetHorde();
    }
}

Rect World::playerRect() const { return { m_playerX, kPlayerY, kPlayerW, kPlayerH }; }

Rect World::playerShotRect() const { return { m_shotX, m_shotY, kShotW, kShotH }; }

Rect World::bombRect(const uint32_t index) const { return { m_bombX[index], m_bombY[index], kBombW, kBombH }; }

bool World::invaderAlive(const uint32_t row, const uint32_t col) const { return m_alive[row * kCols + col]; }

Rect World::invaderRect(const uint32_t row, const uint32_t col) const
{
    const float w = invaderWidth(invaderKind(row));
    return { m_hordeX + col * kCellW + (kCellW - w) * 0.5f, m_hordeY + row * kCellH + (kCellH - kInvaderH) * 0.5f, w, kInvaderH };
}

InvaderKind World::invaderKind(const uint32_t row) { return (row == 0) ? InvaderKind::Squid : (row < 3) ? InvaderKind::Crab : InvaderKind::Octopus; }

void World::killInvader(const uint32_t row, const uint32_t col)
{
    if (m_alive[row * kCols + col])
    {
        m_alive[row * kCols + col] = false;
        --m_aliveCount;
    }
}

void World::resetHorde()
{
    for (bool& alive : m_alive)
        alive = true;
    m_aliveCount = kRows * kCols;
    m_hordeX = kHordeLeft0;
    m_hordeY = std::min(kHordeTop0 + (m_wave - 1) * kHordeTopPerWave, kHordeTopMax);
    m_dir = 1.0f;
    m_stepTimer = 0.0;
    m_steps = 0;
    m_shotActive = false;
    m_bombCount = 0;
    m_bombTimer = 1.0;
}

void World::stepHorde()
{
    ++m_steps;

    // bordas medidas pela extensao VIVA da horda (colunas mortas nao contam)
    uint32_t minCol = kCols;
    uint32_t maxCol = 0;
    for (uint32_t col = 0; col < kCols; ++col)
    {
        for (uint32_t row = 0; row < kRows; ++row)
        {
            if (m_alive[row * kCols + col])
            {
                minCol = std::min(minCol, col);
                maxCol = std::max(maxCol, col);
                break;
            }
        }
    }

    const float left = m_hordeX + minCol * kCellW;
    const float right = m_hordeX + (maxCol + 1) * kCellW;

    const bool atEdge = (m_dir > 0.0f && right + kHordeStepX > kArenaW - kHordeMarginX) || //
                        (m_dir < 0.0f && left - kHordeStepX < kHordeMarginX);
    if (atEdge)
    {
        m_dir = -m_dir;
        m_hordeY += kHordeDropY;
    }
    else
    {
        m_hordeX += m_dir * kHordeStepX;
    }
}

void World::updatePlayerShot(const double dt)
{
    if (!m_shotActive)
    {
        return;
    }

    m_shotY -= kShotSpeed * (float)dt;
    if (m_shotY + kShotH < 0.0f)
    {
        m_shotActive = false;
        return;
    }

    const Rect shot = playerShotRect();
    for (uint32_t row = 0; row < kRows; ++row)
    {
        for (uint32_t col = 0; col < kCols; ++col)
        {
            if (m_alive[row * kCols + col] && shot.intersects(invaderRect(row, col)))
            {
                m_alive[row * kCols + col] = false;
                --m_aliveCount;
                m_score += invaderPoints(invaderKind(row));
                m_shotActive = false;
                return;
            }
        }
    }
}

void World::updateBombs(const double dt)
{
    if (m_config.bombsEnabled && m_aliveCount > 0)
    {
        m_bombTimer -= dt;
        if (m_bombTimer <= 0.0 && m_bombCount < kMaxBombs)
        {
            spawnBomb();
            // cadencia encurta com as ondas; o jitter evita chuva sincronizada
            const double base = std::max(0.35, 1.1 - 0.1 * (m_wave - 1));
            m_bombTimer = base * (0.6 + 0.8 * rand01());
        }
    }

    const Rect player = playerRect();
    for (uint32_t i = 0; i < m_bombCount;)
    {
        m_bombY[i] += kBombSpeed * (float)dt;

        const bool offScreen = m_bombY[i] > kArenaH;
        const bool hitPlayer = m_hitCooldown <= 0.0 && bombRect(i).intersects(player);
        if (hitPlayer)
        {
            --m_lives;
            m_hitCooldown = kHitCooldownSeconds;
            if (m_lives <= 0)
            {
                m_outcome = Outcome::GameOver;
                return;
            }
        }

        if (offScreen || hitPlayer)
        {
            // remove trocando com a ultima (ordem nao importa)
            --m_bombCount;
            m_bombX[i] = m_bombX[m_bombCount];
            m_bombY[i] = m_bombY[m_bombCount];
        }
        else
        {
            ++i;
        }
    }
}

void World::spawnBomb()
{
    // coluna viva aleatoria; a bomba cai do invasor mais baixo dela
    uint32_t liveCols[kCols];
    uint32_t liveColCount = 0;
    for (uint32_t col = 0; col < kCols; ++col)
    {
        for (uint32_t row = 0; row < kRows; ++row)
        {
            if (m_alive[row * kCols + col])
            {
                liveCols[liveColCount++] = col;
                break;
            }
        }
    }
    if (liveColCount == 0)
    {
        return;
    }

    const uint32_t col = liveCols[std::min((uint32_t)(rand01() * liveColCount), liveColCount - 1)];
    uint32_t       bottomRow = 0;
    for (uint32_t row = 0; row < kRows; ++row)
    {
        if (m_alive[row * kCols + col])
        {
            bottomRow = row;
        }
    }

    const Rect from = invaderRect(bottomRow, col);
    m_bombX[m_bombCount] = from.x + (from.w - kBombW) * 0.5f;
    m_bombY[m_bombCount] = from.y + from.h;
    ++m_bombCount;
}

double World::stepInterval() const
{
    // ~0.55s com os 55 vivos ate ~0.06s com o ultimo (aceleracao do arcade)
    return 0.05 + 0.5 * ((double)m_aliveCount / (kRows * kCols));
}

float World::rand01()
{
    // xorshift32 — deterministico por seed, suficiente para as bombas
    m_rng ^= m_rng << 13;
    m_rng ^= m_rng >> 17;
    m_rng ^= m_rng << 5;
    return (float)(m_rng & 0xFFFFFFu) / 16777216.0f;
}

} // namespace si
