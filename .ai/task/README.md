# Plano de trabalho — Space Invaders

Tasks do projeto, no mesmo formato dos planos da
[cengine](https://github.com/mrmarmitt/cengine) e do 8puzzle (`.ai/task/`):
um arquivo por task, com contexto, degraus e critérios de aceite. Status:
`todo` → `in-progress` → `done`.

## Índice

| # | Task | Status | Categoria |
|---|------|--------|-----------|
| 01 | [PoC sprites — Space Invaders no The-Forge](01-poc-sprites.md) | todo | Plataforma |

## Contexto do projeto

O Space Invaders é a segunda PoC da trilha de renderização: onde o 8puzzle
provou que a cengine roda sob o loop do The-Forge (task 01 daquele repo, fase
1), este projeto prova a **renderização 2D de verdade** — shader FSL próprio,
texturas, sprite batcher com atlas, animação e movimento contínuo com dt. O
domínio (entidades/colisão/ondas/pontuação) segue o mesmo desenho do 8puzzle:
C++ puro testável sem plataforma, cenas via `GameRouter`/factory da cengine,
plataforma como detalhe em `src/platform/theforge/`.

O jogo foi escolhido por exercitar exatamente as peças novas sem inflar o
gameplay: ~55 invasores simultâneos justificam o batcher, sprites pequenos
cabem num atlas único, a animação é o caso mínimo (2 frames por troca de UV)
e a horda acelerando testa o timestep — conectando com a task 15 da cengine
(passo fixo hospedado).
