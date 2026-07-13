#pragma once

#include <memory>

namespace cengine::routing {
class ISceneRepository;
}

class GameRouter;
class PlaySession;
class RecordService;

// Registra as cenas The-Forge nos codigos de estado da maquina de fluxo
// (StateGame.h) — mesmo desenho do ForgeSceneFactory do 8puzzle.
class ForgeSceneFactory
{
public:
    static void populateForgeScenes(cengine::routing::ISceneRepository& sceneRepository,
                                    const std::shared_ptr<GameRouter>&    gameRouter,
                                    const std::shared_ptr<PlaySession>&   session,
                                    const std::shared_ptr<RecordService>& recordService);
};
