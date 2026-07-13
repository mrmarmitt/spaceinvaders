# 01 — PoC sprites: Space Invaders no The-Forge

- **Status:** done ✅ (degraus 0 e 1 em 2026-07-11; degraus 2 a 5 em
  2026-07-13; partida manual de ponta a ponta validada pelo jogador em
  2026-07-13 — PoC encerrada)
- **Prioridade:** exploratória (aprendizado de renderização — continuação da
  trilha iniciada na task 01 do 8puzzle)
- **Categoria:** Plataforma
- **Pré-requisitos:** ✅ PoC The-Forge do 8puzzle concluída (fase 1) — casco
  `IApp`, cadeia FSL/rootsig, `PathStatement.txt` e ponte de plataforma
  (`ForgeUi`) já dominados.
- **Relacionada:** task 15 da cengine (modo hospedado do loop) — o degrau 4
  é o caso de teste real dela; task 02 do 8puzzle (fase 2, modo biblioteca)
  segue ortogonal — esta PoC vive inteira no modelo `IApp` da fase 1.

## Visão e decisão de integração

O 8puzzle coube no The-Forge sem renderização de verdade: o jogo inteiro é
texto via middleware de fonte — que, internamente, já é um batcher de quads
texturizados. Esta PoC constrói **o nosso próprio batcher**: shader FSL
próprio, pipeline montado à mão, texturas via IResourceLoader, atlas e
animação. O princípio dos degraus é o mesmo da PoC anterior: **um risco novo
por degrau**, e cada degrau termina com algo rodando na tela.

A arquitetura não muda: `GameManager`/cenas/router intocados; o que cresce é
a ponte de plataforma — o análogo do `ForgeUi` ganha um irmão
(`ForgeSpriteUi` ou similar) com `drawSprite(região, x, y, escala, cor)` em
modo imediato, alimentado pelo batcher. Ao fim, essa ponte deve ter a API de
um renderizador 2D genérico, reutilizável pelo próximo jogo.

### Decisões prévias

- **Onde vive o projeto:** ✅ repo irmão próprio (`spaceinvaders/`, este
  repo), espelhando o layout do 8puzzle (`src/spaceinvaders/game/` +
  `src/platform/theforge/`), com cengine e The-Forge como dependências
  irmãs.
- **`ForgeUi`/`Format.h` (hoje no repo do 8puzzle):** **copiar** para esta
  PoC e aceitar a duplicação. Promover a um lugar compartilhado (cengine ou
  um `platform-common`) é trabalho de infraestrutura — vira task própria se
  a duplicação incomodar depois da PoC.

## Inventário do ambiente

| Item | Estado |
|------|--------|
| The-Forge clonado | `The-Forge/` no workspace, v1.63 (abr/2025) |
| Assets de arte do The-Forge | Baixados (`PRE_BUILD.bat` já rodou) |
| Toolchain | VS 2019 Community 16.11 (solutions `PC_VS2019`) |
| Aprendizados da PoC 1 | Registrados na task 01 do 8puzzle (FSL/rootsig por app, `PathStatement.txt` fora da árvore, `/Zc:char8_t-`, RTTI/exceções, bindings de input) |
| Sprites do jogo | A produzir/obter — PNG e converter (ver degrau 2) |

## Degraus

0. **Casco revalidado** ✅ (2026-07-11): reinstanciar o esqueleto
   provado na PoC 1, agora neste repo: vcxproj em `PC_VS2019/` (a pasta
   PRECISA ter esse nome — `TF_Shared.props`), `PathStatement.txt` próprio,
   `Shaders.list` só com os rootsigs, `IApp` desenhando texto via `ForgeUi`
   copiado, cenas de teste A↔B com ESC saindo. Objetivo: provar que o
   conhecimento da PoC 1 é reproduzível fora do repo do 8puzzle.
   - **Aceite:** ✅ janela abre, texto desenha, navegação A↔B funciona, ESC
     encerra limpo — validado em 2026-07-11 (build Release x64; log com init
     + swapchain + ciclo onEnter/onExit; A↔B/ESC verificados manualmente).
     Duas descobertas no caminho: a task 15 da cengine JÁ estava entregue
     (0.4.0 — o casco usa `EngineManager::frame(dt)` hospedado, não o
     mapeamento manual do CengineAdapter), e o layout espelhado preservou
     todos os caminhos relativos dos vcxproj/PathStatement/Shaders.list
     sem alteração.

1. **Primeiro quad próprio** ✅ (2026-07-11): o primeiro shader FSL
   de verdade do projeto — vertex + pixel desenhando **um retângulo
   colorido** (sem textura). Puxa a cadeia que o middleware escondia: shader
   no `Shaders.list`, root signature, pipeline com blend/depth configurados,
   vertex buffer, **projeção ortográfica**, ciclo `Load`/`Unload` com
   `ReloadDesc` recriando o pipeline. Sem textura de propósito: se falhar, o
   suspeito é o pipeline, não o loader de imagem.
   - **Aceite:** ✅ retângulo colorido na tela; sobrevive a resize e
     alt-tab/fullscreen; coexiste com o texto do `ForgeUi` no mesmo quadro
     (ordem dos passes correta) — validado em 2026-07-11 com screenshots
     (quad gradiente sob o texto fontstash) e resize programático (swapchain
     recriado 1680×720 → 1084×861; o quad manteve os 320×180 px, provando a
     projeção CPU-side; estado da cena preservado). Fullscreen/alt-tab usa o
     mesmo caminho de reload — vale um olho manual, mas sem risco novo.
     Aprendizados: na v1.63 o pipeline usa `PIPELINE_LAYOUT_DESC` com SRT
     sets (shader sem recursos = todos NULL, sobra só o static-sampler
     layout) e `ROOT_SIGNATURE(DefaultRootSignature)` no FSL; a partição do
     Load/Unload segue o `01_Transformations` (shader ← FSL, pipeline ←
     formato do swapchain, conteúdo do VB ← dimensões da tela).

2. **Quad texturizado com alpha** ✅ (2026-07-13): carregar textura via
   `addResource(TextureLoadDesc)`, sampler point/nearest (pixel art),
   descriptor set, alpha blending. Inclui o tooling: gerar o `.dds` e montar
   o mount de texturas no `PathStatement.txt`.
   - **Aceite:** ✅ um sprite com transparência sobre fundo colorido, bordas
     limpas (sem franja de alpha) — validado em 2026-07-13 com screenshots:
     invasor 16×16 desenhado a 10x cavalgando a borda do quad gradiente (o
     alpha compõe contra o gradiente E contra o fundo escuro no mesmo draw),
     bordas de pixel nítidas (sampler nearest), resize programático
     1680×720 → 1084×841 preservando os 160×160 px do sprite e o estado da
     cena; log sem erros e saída limpa (exit code 0).
   - **Aprendizados:**
     - A v1.63 NÃO traz conversor de imagem em `The-Forge/Tools/` (os
       `.tex` de `Art/Textures/dds` são DDS renomeados). O tooling virou
       nosso: `tools/make-invader-dds.ps1` gera DDS RGBA8 **sem compressão**
       com header DX10, que o tinydds do ResourceLoader lê direto — para
       pixel art é o formato certo (texel exato, sem blocos BC).
     - O SRT de verdade: um único `sprite.srt.h`
       (`BEGIN_SRT_NO_AB`/`DECL_TEXTURE`/`DECL_SAMPLER`) incluído pelos
       `.fsl` E pelo C++ (após `defaults.h`) — as macros
       `SRT_SET_DESC`/`SRT_LAYOUT_DESC`/`SRT_RES_IDX` derivam dele, mantendo
       shader e descriptor set em acordo por construção.
     - Ciclo do descriptor set espelha o `01_Transformations`: add/remove
       junto com o shader (`RELOAD_TYPE_SHADER`), `updateDescriptorSet` a
       cada Load (após os adds), `cmdBindDescriptorSet(cmd, 0, ...)` casando
       com o set no primeiro slot do `PIPELINE_LAYOUT_DESC`.
     - Alpha blending é a receita do FontSystem/UI: `BC_SRC_ALPHA` /
       `BC_ONE_MINUS_SRC_ALPHA` (cor e alpha), sem depth — em 2D a ordem de
       draw é a ordem das camadas (gradiente → sprite → texto).
     - `RD_TEXTURES` remontado para `assets/textures/` do repo (5 níveis
       acima do exe) — `pFileName` vai com extensão (`"invader.dds"`), como
       os samples fazem com `.tex`.

3. **Sprite batcher + atlas** ✅ (2026-07-13): a peça central. Spritesheet
   única com todos os sprites do jogo; API `drawSprite(região, x, y, escala,
   cor)` em modo imediato (as cenas chamam, o batcher acumula); vertex
   buffer dinâmico preenchido por quadro e **um draw call para tudo**.
   - **Aceite:** ✅ grade 5×11 (lulas/caranguejos/polvos com tints
     distintos) + jogador + 3 tiros fake em **1 draw call**, movendo em
     bloco, ~60 fps (302 updates/5.0s), contador de sprites/draw calls na
     tela — validado em 2026-07-13 com screenshots ("sprites: 59 | draw
     calls: 1") e resize programático preservando tamanho de pixel, estado
     e o único draw call. Log limpo, saída com exit code 0.
   - **Entregas:** `ForgeSpriteUi.h/.cpp` (a ponte-irmã do ForgeUi),
     `tools/make-atlas-dds.ps1` + `assets/textures/atlas.dds` (64×32,
     células 16×16, sprites BRANCOS — o tint por vértice é a cor, como
     overlay de arcade; 2 frames por invasor já prontos para o degrau 4).
     Os caminhos inline dos degraus 1–2 foram absorvidos pelo batcher
     (quad.fsl removidos; o git guarda as versões didáticas).
   - **Aprendizados:**
     - O risco central (escrever no buffer que a GPU lê) resolvido com a
       receita do UI middleware: UM buffer `CPU_TO_GPU` +
       `BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT` com um trecho por frame in
       flight; a fence do cmd ring já garante que o trecho do frame N foi
       consumido há `gDataBufferCount` quadros — nenhuma sincronização
       extra.
     - Vértice de 20 bytes: pos float2 (NDC na CPU) + uv float2 + cor
       `R8G8B8A8_UNORM` (uint32 ABGR == bytes R,G,B,A little-endian — os
       mesmos valores do `forgeui::color` servem de tint direto); o input
       assembler entrega float4 0..1 ao shader via semântica TEXCOORD1.
     - Camadas 2D atravessando as pontes: `forgeui::drawText` dá flush no
       lote pendente antes de gravar texto → "sprites primeiro, texto
       depois" na cena produz texto por cima, com 1 flush por lote contíguo
       (o casco dá o flush final após o `frame()` para a cauda sem texto).
     - Contadores expostos são do quadro ANTERIOR (fechados no `begin()`
       seguinte) — o texto desenha no meio do quadro corrente e mostraria
       um lote incompleto.
     - O `LNK1103` (intermediário LTCG corrompido) reincide a cada
       rebuild incremental — limpar `out/.../Intermediate/SpaceInvadersForge`
       é a cura; investigar `/LTCG:INCREMENTAL` se virar rotina.

4. **Animação + tempo** ✅ (2026-07-13): animação de 2 frames por troca de
   região de UV a cada passo da marcha; horda marchando em passos discretos
   (anda, borda → desce e inverte) com aceleração conforme "morre" (K
   simula a morte; o intervalo entre passos encolhe de ~0.6s para ~0.07s).
   - **Aceite:** ✅ horda anima e acelera; comportamento idêntico com vsync
     ligado/desligado — validado em 2026-07-13 com capturas em ~10s nas
     duas condições: vsync OFF rendeu **56.106 draws** (~5700 fps na iGPU)
     e vsync ON **592 draws** (60 fps), e nas duas: updates = 60Hz
     cravados, **passos: 16** iguais, posição e pose da horda idênticas.
     Os contadores `updates (passo fixo 60Hz)` vs `draws (taxa de render)`
     ficam na tela expondo a separação. A morte por tecla (K) foi validada
     manualmente em 2026-07-13: a marcha acelera conforme os invasores
     morrem (a injeção de tecla é bloqueada pelo Windows, então essa parte
     é sempre checagem manual).
   - **VEREDITO DO TIMESTEP (alimenta a task 15 da cengine):** o
     `frame(dt)` hospedado da cengine 0.4.0 **resolve o problema por
     inteiro** — o acumulador interno de passo fixo (1/60s) entrega
     `update(dt)` determinístico independente da taxa de render, sem
     nenhum acumulador no adaptador. A receita que funciona: movimento
     SÓ no `update(dt)` (timer de marcha acumulando o dt fixo), `draw()`
     só lê estado. Nada a mudar na cengine.
   - **Aprendizado extra:** o default do The-Forge é vsync OFF
     (`mSettings.mVSyncEnabled = false`) — na iGPU isso queima GPU a
     ~5700 fps; o casco agora liga vsync no `Init()`. O toggle em runtime
     já existia de graça no casco (o `Draw()` compara swapchain vs
     mSettings e recria).

5. **O jogo de verdade** ✅ (2026-07-13): domínio Space Invaders em C++ puro
   (`src/spaceinvaders/game/` — `World` com jogador, horda, tiro único,
   bombas, colisão AABB, ondas, pontuação, vidas, invasão), testável sem
   plataforma. Cenas splash/menu/jogo/gameOver/recordes em `scene/`,
   `GameRouter` + `StateGameFlow` + `ForgeSceneFactory` no padrão do
   8puzzle, recordes em `records.tsv` relativo ao diretório do exe.
   - **Aceite:** ✅ domínio com **13 testes GoogleTest passando no CMake sem
     The-Forge** (estado inicial, bordas, tiro único, mira/pontos por tipo,
     troca de onda, invasão, bombas até o game over, cooldown pós-morte,
     ranking/TSV/top-10). Validação visual no binário real em 2026-07-13:
     gameplay (horda colorida marchando e descendo, bombas, HUD
     pontos/onda, vidas como mini-canhões), transição real
     jogo → game over por bombas (tela FIM DE JOGO + entrada de nome de
     recorde) e splash com a tabela de pontos clássica animada. A partida
     de ponta a ponta (mover/atirar/onda completa/recorde nomeado) foi
     validada manualmente pelo jogador em 2026-07-13: **funcionou tudo**.
   - **Aprendizados:**
     - Arena virtual de **224×256 (a resolução interna do arcade)** no
       domínio: os sprites do atlas ficam 1:1 com as unidades do mundo, e a
       cena projeta com escala inteira (`floor`) — pixels crisp de graça e
       o domínio 100% ignorante de tela.
     - Movimento contínuo exigiu **input de tecla segurada**: bindings com
       fase `pressed` (valor enquanto pressionada) ao lado dos `released`
       (edge) existentes; o forgeui ganhou `moveAxis()`/`fireHeld()`
       amostrados no beginInput, e a cena detecta a borda de subida do
       ESPAÇO para o gatilho.
     - Validação sem teclado: começar o router direto no estado alvo
       (`GameSG` no lugar de `InitialSG`) exercita cena e transições reais
       sem navegar menus — o truque vale para qualquer cena.
     - A versão CMake usa cengine 0.5.0 (FetchContent) e o vcxproj compila
       a cengine irmã (0.4.0) — mesma dualidade do 8puzzle, sem atrito.

## Registro do aprendizado (fechamento da PoC)

- **O que o batcher exigiu da plataforma:** um vertex buffer CPU_TO_GPU
  persistentemente mapeado com um trecho por frame in flight (a fence do cmd
  ring é a única sincronização), pipeline com alpha blending e sampler
  nearest, e o contrato de camadas com o texto (drawText dá flush no lote
  pendente). Nada além disso — o resto é CPU pura.
- **O que o `ForgeSpriteUi` expõe para ser reutilizável:** `drawSprite(
  região-em-px-do-atlas, x, y, escala, tint)` em modo imediato + tabela de
  regiões. Para o próximo jogo falta parametrizar o atlas (hoje `atlas.dds`
  fixo) e, se houver rotação/câmera, entrar uma transform — fora disso a API
  se provou suficiente para um jogo completo.
- **Veredito do timestep:** registrado no degrau 4 — o `frame(dt)` hospedado
  da cengine (task 15) resolve por inteiro; nenhum acumulador no adaptador,
  comportamento idêntico com vsync on/off.

## Critérios de aceite

- [x] Degraus 0–5 validados (aceites individuais acima, incluindo a partida
      manual de ponta a ponta em 2026-07-13).
- [x] Registro do aprendizado (seção acima): o que o batcher exigiu da
      plataforma, o que o `ForgeSpriteUi` precisa expor para ser
      reutilizável pelo próximo jogo, e o veredito sobre timestep — alimenta
      a task 15 da cengine.

## Riscos

- **Sincronização do buffer dinâmico** (degrau 3): mitigado copiando o
  padrão do middleware de fonte, que mantém buffers por frame in flight.
- **Depuração de shader FSL**: erros do cross-compiler são crípticos;
  mitigar começando do shader mais simples possível e evoluindo por diffs
  pequenos (nunca dois problemas novos no mesmo build).
- **Coexistência com o middleware de fonte** (degrau 1): ordem de render
  targets/passes; mitigar desenhando os quads antes do texto no mesmo render
  pass.

## Fora do escopo

- **Som** — o The-Forge não tem subsistema de áudio; Space Invaders mudo é
  aceitável para a PoC (integrar miniaudio/SoLoud seria task própria).
- Rotação de sprite e câmera com scroll — são o "segundo jogo"
  (Asteroids/runner), por cima do batcher pronto.
- Promover `ForgeUi`/`Format.h` a código compartilhado entre repos.
- Modo biblioteca (fase 2 do 8puzzle) e integração de build definitiva.
