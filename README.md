# Space Invaders

Um Space Invaders completo e jogável, construído como a **segunda PoC da
trilha de renderização**: onde o [8puzzle](https://github.com/mrmarmitt)
provou que a [cengine](https://github.com/mrmarmitt/cengine) roda sobre o
[The-Forge](https://github.com/ConfettiFX/The-Forge), este projeto prova a
**renderização 2D de verdade** — shader FSL próprio, texturas com alpha,
sprite batcher com atlas (a horda inteira em **1 draw call**), animação e
movimento contínuo com passo fixo.

A cengine é dona do game loop (`EngineManager::start()`); o The-Forge entra
como **biblioteca** atrás do port `IWindowManager` (D3D12 no Windows).

## Controles

| Tecla | Ação |
|---|---|
| ← → | mover o canhão |
| ESPAÇO | atirar (um tiro em voo por vez, como no arcade) |
| ↑ ↓ / ENTER | navegar / confirmar nos menus |
| ESC | voltar / abandonar a partida / sair (do menu) |

Pontuação: lula = 30, caranguejo = 20, polvo = 10. A horda acelera conforme
morre e desce a cada borda; alcançar a linha do canhão é fim de jogo
imediato. Recordes (top-10 por pontos) persistem em `records.tsv` ao lado
do exe.

## Estrutura

```
src/
  main_theforge.cpp        entry point + composition root (todas as
                           instâncias e injeções principais)
  spaceinvaders/game/      DOMÍNIO em C++ puro: World (horda, tiros,
                           bombas, colisão AABB, ondas, vidas), GameRouter
                           + máquina de estados, serviços de recorde —
                           zero dependência de plataforma
  platform/theforge/
    src/SpaceInvadersForge/
      TheForgeWindowManager.*   janela Win32 + GPU atrás do IWindowManager
      ForgeUi.*                 ponte de teclado + texto (fontstash)
      ForgeSpriteUi.*           sprite batcher (atlas, 1 draw call/lote)
      scene/                    splash, menu, jogo, game over, recordes
      Shaders/FSL/              shaders próprios (sprite.vert/frag + SRT)
    PC_VS2019/                  projeto MSBuild (a pasta PRECISA ter esse nome)
test/                      suíte GoogleTest do domínio (roda sem The-Forge)
tools/make-atlas-dds.ps1   gera assets/textures/atlas.dds (DDS RGBA8)
.ai/task/                  plano de trabalho, degraus e aprendizados
```

## Build do jogo (Windows)

Pré-requisitos:

- Visual Studio 2019 (toolset v142);
- checkouts **irmãos** deste repo: [`The-Forge`](https://github.com/ConfettiFX/The-Forge)
  v1.63 e [`cengine`](https://github.com/mrmarmitt/cengine) —
  o workspace fica `The-Forge/`, `cengine/` e `spaceinvaders/` lado a lado;
- no The-Forge: rodar `PRE_BUILD.bat` (baixa os assets de arte) e buildar a
  solution `Examples_3/Unit_Tests/PC_VS2019` em **Release x64** (gera
  `OS.lib`/`Renderer.lib` que o jogo linka).

Depois:

```powershell
msbuild src\platform\theforge\PC_VS2019\SpaceInvadersForge.sln /p:Configuration=Release /p:Platform=x64
out\theforge\x64\Release\SpaceInvadersForge\SpaceInvadersForge.exe
```

## Testes do domínio (sem The-Forge)

O domínio compila e testa sozinho via CMake (busca cengine e GoogleTest por
FetchContent — precisa de rede no configure):

```powershell
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Debug
ctest --test-dir build -C Debug
```

## Plano de trabalho

O projeto foi construído em degraus documentados — um risco novo por
degrau, cada um validado com evidência antes do próximo. A história
completa (decisões, aceites e aprendizados de The-Forge/FSL/batcher/
timestep) está em [`.ai/task/`](.ai/task/README.md).
