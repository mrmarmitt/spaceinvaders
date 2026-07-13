#pragma once

#include <cstdint>

// Resultado da ultima partida, compartilhado entre a cena do jogo (escreve
// no game over) e a cena de game over (le para exibir e montar o Record).
// Papel analogo ao GamePlayService do 8puzzle, reduzido ao que o Space
// Invaders precisa.
class PlaySession
{
    int      m_score = 0;
    uint32_t m_wave = 1;

public:
    void setResult(const int score, const uint32_t wave)
    {
        m_score = score;
        m_wave = wave;
    }

    [[nodiscard]] int      score() const { return m_score; }
    [[nodiscard]] uint32_t wave() const { return m_wave; }
};
