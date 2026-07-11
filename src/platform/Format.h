#pragma once

#include <iomanip>
#include <sstream>
#include <string>

// Formata uma duração em milissegundos como hh:mm:ss.mmm (mesmo formato que a
// plataforma terminal usava no jogo). Compartilhado entre as plataformas
// (ftxui, theforge) — é std puro, sem dependência de biblioteca gráfica.
inline std::string formatMillis(const long long millis) {
    const int hours = static_cast<int>(millis / (1000 * 60 * 60));
    const int minutes = static_cast<int>((millis / (1000 * 60)) % 60);
    const int seconds = static_cast<int>((millis / 1000) % 60);
    const int milliseconds = static_cast<int>(millis % 1000);

    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << minutes
        << ":" << std::setw(2) << seconds << "." << std::setw(3) << milliseconds;
    return out.str();
}
