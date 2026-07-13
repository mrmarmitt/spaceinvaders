// main.cpp — por onde tudo comeca.
//
// No modo hospedado do The-Forge o main() de verdade NAO e nosso: a macro
// DEFINE_APPLICATION_MAIN (IApp.h) gera um main() minimo que entrega uma
// instancia estatica do nosso app ao WindowsMain do framework
// (Common_3/OS/Windows/WindowsBase.cpp), que e o dono do processo, da
// janela e do loop. A partir dai o controle e invertido — o framework
// chama a gente:
//
//   WindowsMain
//     -> SpaceInvadersForge::Init()   subsistemas + batcher + fiacao do
//                                     jogo (dominio, GameRouter, cenas,
//                                     EngineManager da cengine hospedado)
//     -> Load()                       swapchain, pipelines, fontes
//     -> loop:  Update(dt)            captura input (forgeui::beginInput)
//               Draw()                grava o quadro e roda
//                                     gEngine->frame(dt) — o quadro da
//                                     CENGINE (input -> update(dt fixo)
//                                     -> render das cenas); false =
//                                     jogo roteou para "exit" ->
//                                     requestShutdown()
//     -> Unload() / Exit()            teardown na ordem inversa
//
// O jogo em si nao esta aqui: o dominio (World, regras) vive em
// src/spaceinvaders/game/ e as cenas em scene/ — este diretorio e so a
// plataforma. A macro tambem exporta os simbolos do Agility SDK do D3D12.

#include "SpaceInvadersForge.h"

DEFINE_APPLICATION_MAIN(SpaceInvadersForge)
