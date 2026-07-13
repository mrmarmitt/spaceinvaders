#include "ForgeSceneFactory.h"

#include <cengine/routing/ISceneRepository.hpp>

#include "spaceinvaders/game/GameRouter.h"

#include "scene/ForgeExitScene.h"
#include "scene/ForgeGameOverScene.h"
#include "scene/ForgeGameScene.h"
#include "scene/ForgeMenuScene.h"
#include "scene/ForgeRecordScene.h"
#include "scene/ForgeSplashScene.h"

// As factories rodam LAZY (no primeiro getScene de cada estado) — capturas
// por VALOR. Estados que recriam a cena a cada visita (game, gameOver)
// ganham partida/nome zerados de graca.
void ForgeSceneFactory::populateForgeScenes(cengine::routing::ISceneRepository& sceneRepository,
                                            const std::shared_ptr<GameRouter>&    gameRouter,
                                            const std::shared_ptr<PlaySession>&   session,
                                            const std::shared_ptr<RecordService>& recordService)
{
    sceneRepository.registerFactory("initial", [gameRouter]() { return std::make_unique<ForgeSplashScene>(gameRouter); });
    sceneRepository.registerFactory("menu", [gameRouter]() { return std::make_unique<ForgeMenuScene>(gameRouter); });
    sceneRepository.registerFactory("game",
                                    [gameRouter, session]() { return std::make_unique<ForgeGameScene>(gameRouter, session); });
    sceneRepository.registerFactory("gameOver", [gameRouter, session, recordService]() {
        return std::make_unique<ForgeGameOverScene>(gameRouter, session, recordService);
    });
    sceneRepository.registerFactory(
        "record", [gameRouter, recordService]() { return std::make_unique<ForgeRecordScene>(gameRouter, recordService); });
    sceneRepository.registerFactory("exit", []() { return std::make_unique<ForgeExitScene>(); });
}
