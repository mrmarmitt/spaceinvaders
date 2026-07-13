# 01 — PoC sprites: Space Invaders no The-Forge

- **Status:** in-progress (degraus 0 e 1 ✅ em 2026-07-11; degrau 2 ✅ em
  2026-07-13)
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

3. **Sprite batcher + atlas** (risco: buffer dinâmico + sincronização GPU):
   a peça central. Spritesheet única com todos os sprites do jogo; API
   `drawSprite(região, x, y, escala, cor)` em modo imediato (as cenas
   chamam, o batcher acumula); vertex buffer dinâmico preenchido por quadro
   e **um draw call para tudo**. Risco escondido: frames in flight exigem um
   buffer por frame do swapchain (ou ring buffer) — escrever no buffer que a
   GPU ainda lê é o bug clássico deste degrau. Referência: o middleware de
   fonte (`Common_3/Application/Fonts/`) já resolve isso.
   - **Aceite:** grade 5×11 de invasores + jogador + tiros fake em 1 draw
     call, movendo em bloco, 60 fps estáveis; contador de sprites/draw calls
     na tela (via texto).

4. **Animação + tempo** (risco: dt e timestep): animação de 2 frames por
   troca de região de UV com timer; horda marchando com aceleração conforme
   "morre" (mortes simuladas por tecla). Decidir o timestep: replicar o
   acumulador de passo fixo da cengine no adaptador, ou usar o `frame(dt)`
   hospedado se a task 15 da cengine tiver andado. Este degrau é o caso de
   teste real daquela task — registrar o veredito.
   - **Aceite:** horda anima e acelera suavemente; comportamento idêntico
     com vsync ligado/desligado (lógica independente do framerate).

5. **O jogo de verdade** (risco: nenhum novo — é integração): domínio Space
   Invaders em C++ puro (entidades, colisão AABB, ondas, pontuação, vidas),
   testável sem plataforma como no 8puzzle. Cenas
   splash/menu/jogo/gameOver/recordes nos mesmos códigos de estado,
   `GameRouter` + factory no padrão provado em três plataformas, recordes em
   TSV relativo ao diretório do exe.
   - **Aceite:** jogável de ponta a ponta — onda completa, morte, game over
     com recorde nomeado e persistido; domínio com testes rodando no CMake
     sem The-Forge.

## Critérios de aceite

- [ ] Degraus 0–5 validados (aceites individuais acima).
- [ ] Registro do aprendizado (seção a preencher ao fim): o que o batcher
      exigiu da plataforma, o que o `ForgeSpriteUi` precisa expor para ser
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
