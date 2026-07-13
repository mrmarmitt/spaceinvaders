// main_theforge.cpp — por onde tudo comeca: entry point da plataforma
// The-Forge em MODO BIBLIOTECA + composition root do jogo (mesmo padrao do
// main_theforge.cpp do 8puzzle, fase 2): o main vive fora da plataforma e
// do pacote do jogo, e e onde todas as instancias e injecoes principais
// acontecem.
//
// A CENGINE e dona do loop: main() monta o jogo e chama engine.start(), que
// bloqueia ate o jogo rotear para "exit". O The-Forge entra como BIBLIOTECA
// atras do port IWindowManager (TheForgeWindowManager: janela Win32 propria,
// GPU no par update()/present() da cengine 0.5.0). E a evolucao do casco
// IApp da fase 1 (modo hospedado, ver git ate o degrau 5): sai a macro
// DEFINE_APPLICATION_MAIN e a inversao de controle do framework; o main()
// e nosso.
//
//   main()
//     -> initMemAlloc/initFileSystem/initLog   subsistemas de processo (na
//                                              ordem do WindowsMain)
//     -> composicao: repositorio de cenas -> router (estado inicial =
//        splash) -> GameRouter (fachada de dominio) -> PlaySession/
//        RecordService (records.tsv ao lado do exe) -> factories de cena
//     -> EngineManager{ TheForgeWindowManager, GameManager }
//     -> engine.start()   loop da cengine: window.update() -> fases do jogo
//                         (input -> update(dt fixo) 0..N -> render) ->
//                         window.present(); shouldExit() -> cleanup()
//
// O dominio (World, regras) vive em src/spaceinvaders/game/ e as cenas em
// src/platform/theforge/src/SpaceInvadersForge/scene/ — nenhuma cena mudou
// na migracao IApp -> IWindowManager: elas so falam com forgeui/forgesprite.
//
// Este main NAO e um alvo CMake: compila no SpaceInvadersForge.vcxproj
// (MSBuild), que reutiliza a cadeia de build do The-Forge — dai os
// subsistemas de processo e os exports do Agility SDK aqui dentro.

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
#include "platform/theforge/src/SpaceInvadersForge/TheForgeWindowManager.h"

#include <memory>

#include "Common_3/OS/Interfaces/IOperatingSystem.h" // UINT dos exports do Agility
#include "Common_3/Utilities/Interfaces/IFileSystem.h"
#include "Common_3/Utilities/Interfaces/ILog.h"

#include "Common_3/Utilities/Interfaces/IMemory.h" // deve ser o ultimo include

// O DEFINE_APPLICATION_MAIN exportava estes simbolos para ativar o Agility
// SDK do D3D12 (D3D12Core.dll copiada para a pasta do exe). Sem IApp,
// exportamos por conta propria — D3D12_AGILITY_SDK_VERSION vem do
// TF_Shared.props.
extern "C"
{
    __declspec(dllexport) extern const UINT  D3D12SDKVersion = D3D12_AGILITY_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = "";
}

int main()
{
    constexpr const char* kAppName = "SpaceInvadersForge";

    // Subsistemas de processo na ordem do WindowsMain; o resto (janela, GPU,
    // fontes, batcher) e responsabilidade do TheForgeWindowManager, dentro
    // da cengine.
    if (!initMemAlloc(kAppName))
        return EXIT_FAILURE;

    FileSystemInitDesc fsDesc = {};
    fsDesc.pAppName = kAppName;
    if (!initFileSystem(&fsDesc))
        return EXIT_FAILURE;

    initLog(kAppName, DEFAULT_LOG_LEVEL);

    {
        // composicao do jogo: quem conhece o grafo inteiro (dominio +
        // servicos + cenas + engine) e so este arquivo
        auto sceneRepository = std::make_unique<cengine::routing::SceneRepository>();
        cengine::routing::ISceneRepository& sceneRepositoryRef = *sceneRepository;

        const auto router =
            std::make_shared<cengine::routing::RouterInMemory>(std::move(sceneRepository), std::make_unique<InitialSG>());

        const auto gameRouter = std::make_shared<GameRouter>(router);
        const auto session = std::make_shared<PlaySession>();
        // recordes relativos ao diretorio de trabalho (o diretorio do exe)
        const auto recordRepository = std::make_shared<FileRecordRepository>("records.tsv");
        const auto recordService = std::make_shared<RecordService>(recordRepository);

        ForgeSceneFactory::populateForgeScenes(sceneRepositoryRef, gameRouter, session, recordService);

        // Modo PROPRIO: a cengine dirige o loop e o The-Forge entra como
        // biblioteca atras do IWindowManager.
        cengine::core::EngineManager engine{
            std::make_unique<TheForgeWindowManager>(kAppName, 1280, 720),
            std::make_unique<cengine::routing::GameManager>(router),
        };

        engine.start(); // bloqueia ate o jogo rotear para "exit"; o
                        // cleanup() (jogo + janela/GPU) roda no fim do start()
    }

    LOGF(eINFO, "[spaceinvaders] loop da cengine encerrado");

    exitLog();
    exitFileSystem();
    exitMemAlloc();
    return 0;
}
