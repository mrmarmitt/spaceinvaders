#pragma once

// Ponte entre o IApp (SpaceInvadersForge.cpp) e as cenas da plataforma
// The-Forge (copiado do 8puzzle — decisao registrada na task 01):
// o app publica aqui o snapshot de input de cada Update e o alvo de desenho de
// cada Draw; as cenas consomem via readKey()/drawText*() sem conhecer o IApp.
// Equivale ao par Keyboard.h + "present()" da plataforma FTXUI — teclado em
// fila (no maximo um evento consumido por input()) e desenho em modo imediato.

#include <cstdint>
#include <string>

// The-Forge (o include path do projeto aponta para a raiz do The-Forge).
#include "Common_3/Application/Interfaces/IFont.h"
#include "Common_3/Graphics/Interfaces/IGraphics.h"

// Mesmo vocabulario de teclas da plataforma FTXUI (Keyboard.h).
enum class Key {
    None,      // nenhuma tecla pendente
    Up,
    Down,
    Left,
    Right,
    Enter,
    Escape,
    Backspace,
    Char,      // caractere imprimivel (ver KeyEvent::character)
    Other
};

struct KeyEvent {
    Key key = Key::None;
    char character = '\0'; // valido quando key == Key::Char
};

namespace forgeui {

// Paleta (ABGR, formato do FontDrawDesc::mFontColor): mesma intencao de cores
// das cenas FTXUI (ciano para titulos, ambar para destaques).
namespace color {
inline constexpr uint32_t kTitle = 0xffffb300;   // ciano
inline constexpr uint32_t kAccent = 0xff00b3ff;  // ambar
inline constexpr uint32_t kValue = 0xff00d7ff;   // dourado (valores/stats)
inline constexpr uint32_t kSuccess = 0xff00c800; // verde
inline constexpr uint32_t kText = 0xffffffff;
inline constexpr uint32_t kDim = 0xff9a9a9a;
inline constexpr uint32_t kFaint = 0xff5a5a5a;
} // namespace color

// --- ciclo de vida (chamado pelo casco da plataforma) ---

// Enfileira um evento de tecla vindo do WndProc proprio (WM_KEYDOWN/
// WM_CHAR) — modo biblioteca: o TheForgeWindowManager alimenta, as cenas
// consomem sem saber quem alimentou. (No modo hospedado da fase 1, o
// equivalente era o beginInput lendo o input system do framework — ver
// git para a versao IApp.)
void pushKey(KeyEvent event);

// Publica o estado segurado do quadro (WM_KEYDOWN/WM_KEYUP rastreados pelo
// WndProc): eixo de movimento (-1..+1) e gatilho. Chamado pelo window
// manager a cada update(), antes das fases do jogo.
void pushHeldState(float moveAxis, bool fireHeld);

// Publica o alvo de desenho do quadro (chamado no Draw, antes do render()).
void beginDraw(Cmd* cmd, float width, float height, uint32_t fontID);

// --- consumo pelas cenas ---

// Consome no maximo um evento de tecla por chamada (fila esvazia 1/quadro).
KeyEvent readKey();

// Estado continuo (teclas SEGURADAS), publicado por pushHeldState — a fila
// de edges acima nao serve para movimento: mover o canhao exige saber se a
// seta esta pressionada AGORA, todo quadro (degrau 5).
float moveAxis(); // -1 esquerda .. +1 direita (setas)
bool  fireHeld(); // ESPACO pressionado (a cena detecta a borda de subida)

float screenWidth();
float screenHeight();

[[nodiscard]] float textWidth(const std::string& text, float fontSize);

void drawText(const std::string& text, float x, float y, float fontSize, uint32_t colorAbgr);
void drawTextCentered(const std::string& text, float y, float fontSize, uint32_t colorAbgr);

// Rodape padrao com as dicas de tecla da cena (equivale ao hints() FTXUI).
void drawHints(const std::string& text);

} // namespace forgeui
