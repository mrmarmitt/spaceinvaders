# 02 — Modo biblioteca: cengine dona do loop (IWindowManager)

- **Status:** done ✅ (2026-07-13)
- **Prioridade:** média — evolução do ponto anterior: o casco `IApp` já era
  um ponto superado na fase 2 do 8puzzle.
- **Categoria:** Plataforma
- **Pré-requisitos:** ✅ task 01 (PoC sprites completa no modo hospedado);
  ✅ fase 2 do 8puzzle (task 02 daquele repo — a receita do
  `TheForgeWindowManager` sobre a cengine 0.5.0).

## Objetivo

Substituir o casco `IApp`/`DEFINE_APPLICATION_MAIN` (modo hospedado, fase 1)
pelo port `IWindowManager` da cengine (modo próprio): o `main()` é nosso, a
cengine dirige o loop via `EngineManager::start()`, e o The-Forge vira
biblioteca atrás do `TheForgeWindowManager`. Diferente do 8puzzle (que
manteve os dois alvos), aqui o alvo único **evolui** — a fase 1 vive no git
(até o commit do degrau 5 + main.cpp).

## O que mudou

- **`src/main_theforge.cpp`**: agora é um `main()` de verdade, convergido
  por inteiro com o formato do 8puzzle — subsistemas de processo
  (initMemAlloc/initFileSystem/initLog), exports do Agility SDK, composição
  do jogo e `engine.start()`. Saíram `SpaceInvadersForge.h/.cpp` (casco
  IApp) e `GameComposition.h` (o contrato só existia porque o boot era do
  framework).
- **`TheForgeWindowManager`**: a receita do 8puzzle + as duas peças que ele
  não tinha — o **batcher de sprites** (`forgesprite::begin` no `update()`,
  `flush` da cauda no `present()`, load/unload no ciclo) e o **input de
  tecla segurada** (WM_KEYDOWN/WM_KEYUP rastreiam setas/ESPAÇO;
  `forgeui::pushHeldState` publica a cada quadro; WM_KILLFOCUS solta tudo
  para não travar tecla no alt-tab).
- **`ForgeUi`**: saiu o caminho do input system do framework
  (`initBindings`/`beginInput` — bindings `released`/`pressed`); a fila é
  alimentada por `pushKey` (WndProc, ignorando auto-repeat via bit 30 do
  lParam) e o estado segurado por `pushHeldState`. As funções de texto e o
  contrato das cenas não mudaram.
- **Zero mudança em cenas e domínio** — a prova de que as pontes
  (forgeui/forgesprite) isolam de verdade: a migração de casco inteira não
  tocou uma linha de `scene/` nem de `src/spaceinvaders/`.

## Aceite

- ✅ Build limpo; splash com sprites+texto renderizando (sem os widgets do
  UI middleware do The-Forge — janela limpa de graça).
- ✅ Resize recriando o swapchain via WM_SIZE → `applyPendingResize`
  (1280×720 → 984×861 no log).
- ✅ Fechamento como decisão do jogo: X → ESC na fila → splash roteou para
  o MENU (primeira validação automatizada que enxerga o menu!); segundo X →
  exit → `start()` retornou → exit code 0, log limpo.
- ✅ Partida completa segue com o jogador (cenas intocadas); vale a
  checagem manual de gameplay (setas/ESPAÇO agora via WndProc).

## Aprendizados

- O contrato `update()`/`present()` da cengine 0.5.0 absorveu o batcher sem
  atrito: `begin` no preparo do quadro, `flush` da cauda no present — o
  mesmo lugar que o casco IApp usava, só que agora do lado de dentro do
  window manager.
- Input segurado sem o input system do framework é trivial no Win32:
  WM_KEYDOWN/WM_KEYUP + WM_KILLFOCUS cobrem tudo que os bindings `pressed`
  davam; o auto-repeat (bit 30) é o único cuidado para a fila de edges.
- O X da janela roteando pelo jogo (WM_CLOSE → ESC na fila) deixa o
  encerramento sempre limpo — o `cleanup()` roda depois do último quadro
  apresentado, nunca no meio de um.

## Relacionado

- Task 01 — a PoC que este modo substitui como casco (degraus preservados
  no git).
- 8puzzle task 02 (fase 2) — a origem da receita.
- cengine task 21 — proposta de tornar o `IWindowManager` obrigatório
  (remover a hipótese do `nullptr`), agora que os dois consumidores reais
  usam o modo próprio.
