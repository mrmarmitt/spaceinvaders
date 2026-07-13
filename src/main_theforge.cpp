// main_theforge.cpp — por onde tudo comeca: entry point da plataforma
// The-Forge + COMPOSITION ROOT do jogo (mesmo padrao dos mains do 8puzzle:
// o main vive fora da plataforma e do pacote do jogo, e e onde todas as
// instancias e injecoes principais acontecem).
//
// ENTRY POINT (modo hospedado — fase 1): o main() de verdade e gerado pela
// macro DEFINE_APPLICATION_MAIN (no fim deste arquivo), que entrega uma
// instancia estatica do casco ao WindowsMain do The-Forge
// (Common_3/OS/Windows/WindowsBase.cpp) — dono do processo, da janela e do
// loop. A partir dai o controle e invertido, e o framework chama o casco:
//
//   WindowsMain
//     -> SpaceInvadersForge::Init()   subsistemas + batcher + fontes; ao
//                                     final chama composeSpaceInvadersGame()
//                                     (abaixo) para montar o jogo
//     -> Load()                       swapchain, pipelines
//     -> loop:  Update(dt)            captura input (forgeui::beginInput)
//               Draw()                grava o quadro e roda
//                                     engine.frame(dt) — o quadro da
//                                     CENGINE (input -> update(dt fixo) ->
//                                     render das cenas); false = o jogo
//                                     roteou para "exit" -> shutdown
//     -> Unload() / Exit()            teardown na ordem inversa
//
// COMPOSICAO: composeSpaceInvadersGame() monta o grafo de objetos que os
// mains do 8puzzle montam — repositorio de cenas -> router (estado inicial
// = splash) -> GameRouter (fachada de dominio) -> servicos -> factories de
// cena -> EngineManager em modo hospedado. O dominio (World, regras) vive
// em src/spaceinvaders/game/ e as cenas em
// src/platform/theforge/src/SpaceInvadersForge/scene/.
//
// Este main NAO e um alvo CMake: compila no SpaceInvadersForge.vcxproj
// (MSBuild), que reutiliza a cadeia de build do The-Forge.

// cengine + jogo — C++ puro, ANTES dos headers do The-Forge (IMemory.h por ultimo).
#include <cengine/core/EngineManager.hpp>
#include <cengine/routing/GameManager.hpp>
#include <cengine/routing/RouterInMemory.hpp>
#include <cengine/routing/SceneRepository.hpp>

#include "spaceinvaders/game/GameRouter.h"
#include "spaceinvaders/game/service/PlaySession.h"
#include "spaceinvaders/game/service/RecordService.h"
#include "spaceinvaders/game/service/repository/FileRecordRepository.h"
#include "spaceinvaders/game/state/StateGame.h"

#include "platform/theforge/src/SpaceInvadersForge/ForgeSceneFactory.h"
#include "platform/theforge/src/SpaceInvadersForge/GameComposition.h"
#include "platform/theforge/src/SpaceInvadersForge/SpaceInvadersForge.h"

#include <memory>

#include "Common_3/Utilities/Interfaces/IMemory.h" // deve ser o ultimo include

GameComposition composeSpaceInvadersGame()
{
    // fiacao identica em espirito aos mains do 8puzzle: quem conhece o
    // grafo inteiro (dominio + servicos + cenas + engine) e so este arquivo
    auto sceneRepository = std::make_unique<cengine::routing::SceneRepository>();
    cengine::routing::ISceneRepository& sceneRepositoryRef = *sceneRepository;

    auto router = std::make_shared<cengine::routing::RouterInMemory>(std::move(sceneRepository), std::make_unique<InitialSG>());

    const auto gameRouter = std::make_shared<GameRouter>(router);
    const auto session = std::make_shared<PlaySession>();
    // recordes relativos ao diretorio de trabalho (o diretorio do exe)
    const auto recordRepository = std::make_shared<FileRecordRepository>("records.tsv");
    const auto recordService = std::make_shared<RecordService>(recordRepository);

    ForgeSceneFactory::populateForgeScenes(sceneRepositoryRef, gameRouter, session, recordService);

    // Modo hospedado: sem window manager (janela e do The-Forge) e sem
    // start() — o Draw() do casco dirige via frame(dt).
    auto engine =
        std::make_unique<cengine::core::EngineManager>(nullptr, std::make_unique<cengine::routing::GameManager>(router));

    return { std::move(router), std::move(engine) };
}

DEFINE_APPLICATION_MAIN(SpaceInvadersForge)
