#pragma once

// O irmao do ForgeUi para sprites (degrau 3 da PoC): as cenas desenham via
// drawSprite() em modo imediato, e o batcher acumula os quads num vertex
// buffer dinamico (um trecho por frame in flight) e emite UM draw call por
// lote. O ciclo de vida (init/load/begin/flush) e do casco IApp; as cenas so
// enxergam drawSprite + as regioes do atlas.
//
// Camadas 2D = ordem de chamada, atravessando as duas pontes: o
// forgeui::drawText da flush no lote pendente antes de gravar texto, entao
// "sprites primeiro, texto depois" dentro do draw() da cena produz o texto
// por cima — a mesma semantica de um renderer 2D imediato de verdade.

#include <cstdint>

// The-Forge (o include path do projeto aponta para a raiz do The-Forge).
#include "Common_3/Graphics/Interfaces/IGraphics.h"

namespace forgesprite {

// Regiao do atlas em PIXELS (origem no canto superior esquerdo da textura).
// A tabela abaixo espelha o layout gerado por tools/make-atlas-dds.ps1 —
// mudou o atlas la, atualize aqui.
struct SpriteRegion
{
    float x;
    float y;
    float w;
    float h;
};

namespace sprites {
inline constexpr SpriteRegion kCrab0 = { 0.0f, 0.0f, 11.0f, 8.0f };
inline constexpr SpriteRegion kCrab1 = { 16.0f, 0.0f, 11.0f, 8.0f };
inline constexpr SpriteRegion kSquid0 = { 32.0f, 0.0f, 8.0f, 8.0f };
inline constexpr SpriteRegion kSquid1 = { 48.0f, 0.0f, 8.0f, 8.0f };
inline constexpr SpriteRegion kOcto0 = { 0.0f, 16.0f, 12.0f, 8.0f };
inline constexpr SpriteRegion kOcto1 = { 16.0f, 16.0f, 12.0f, 8.0f };
inline constexpr SpriteRegion kPlayer = { 32.0f, 16.0f, 13.0f, 8.0f };
inline constexpr SpriteRegion kShot = { 48.0f, 16.0f, 3.0f, 7.0f };
} // namespace sprites

// Contadores do quadro ANTERIOR (fechados no begin() seguinte) — e o que a
// cena mostra na tela sem depender da ordem draw-sprites/draw-texto.
struct Stats
{
    uint32_t sprites = 0;
    uint32_t drawCalls = 0;
};

// --- ciclo de vida (chamado pelo casco da plataforma) ---

// Init do app: cria textura do atlas + sampler + vertex buffer dinamico
// (frameCount trechos). Requer resource loader + root signature prontos;
// o app chama waitForAllResourceLoads() depois.
void init(Renderer* renderer, uint32_t frameCount);
void exit();

// Load/Unload do app: shader + descriptor set (RELOAD_TYPE_SHADER) e
// pipeline (SHADER|RENDERTARGET), como os recursos proprios do casco.
void load(const ReloadDesc* reloadDesc, TinyImageFormat colorFormat, SampleCount sampleCount, uint32_t sampleQuality);
void unload(const ReloadDesc* reloadDesc);

// Abre o lote do quadro: publica cmd + dimensoes, avanca o trecho do vertex
// buffer (frameIndex) e fecha os contadores do quadro anterior.
void begin(Cmd* cmd, float width, float height, uint32_t frameIndex);

// Desenha o lote acumulado (1 draw call) e reinicia a acumulacao. Chamado
// pelo forgeui::drawText (sprites pendentes ficam SOB o texto) e pelo casco
// apos o frame() da cengine (cauda sem texto). No-op com lote vazio.
void flush();

// --- consumo pelas cenas ---

// Enfileira um sprite: regiao do atlas, canto superior esquerdo em pixels,
// escala (multiplica o tamanho nativo da regiao) e tint ABGR (mesmo formato
// do forgeui::color; o atlas e branco, o tint E a cor).
void drawSprite(const SpriteRegion& region, float x, float y, float scale, uint32_t colorAbgr);

Stats lastFrameStats();

} // namespace forgesprite
