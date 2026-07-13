#pragma once

#include <cstdint>
#include <ctime>
#include <string>
#include <utility>

// Recorde de uma partida (nome, pontuacao, onda alcancada, data) — mesmo
// papel do Record do 8puzzle, com a metrica do Space Invaders (pontos).

class Record
{
    std::string m_name;
    int         m_score = 0;
    uint32_t    m_wave = 1;
    std::string m_playedAt;

public:
    Record() = default;

    Record(std::string name, const int score, const uint32_t wave, std::string playedAt):
        m_name(std::move(name)), m_score(score), m_wave(wave), m_playedAt(std::move(playedAt))
    {
    }

    Record(const int score, const uint32_t wave): m_score(score), m_wave(wave) {}

    void assignRecord(std::string name, std::string playedAt)
    {
        m_name = std::move(name);
        m_playedAt = std::move(playedAt);
    }

    [[nodiscard]] const std::string& getName() const { return m_name; }
    [[nodiscard]] int                getScore() const { return m_score; }
    [[nodiscard]] uint32_t           getWave() const { return m_wave; }
    [[nodiscard]] const std::string& getPlayedAt() const { return m_playedAt; }

    [[nodiscard]] bool scoresHigherThan(const Record& other) const { return m_score > other.m_score; }
};

/// Data/hora local "YYYY-MM-DD HH:MM" para carimbar o recorde.
inline std::string nowAsString()
{
    const std::time_t now = std::time(nullptr);
    std::tm           tm = {};
#if defined(_WIN32)
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &tm);
    return buffer;
}
