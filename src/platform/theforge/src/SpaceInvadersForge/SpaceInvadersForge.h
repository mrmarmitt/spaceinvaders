#pragma once

// O casco da plataforma The-Forge (modo hospedado — fase 1): implementa os
// callbacks do IApp que o framework invoca. Quem chama cada metodo e o
// WindowsBase.cpp do The-Forge, dono do main/loop real — ver main.cpp para
// a historia completa do boot.
//
// O estado do casco (renderer, swapchain, engine da cengine) vive como
// globals no SpaceInvadersForge.cpp, no idioma dos samples do The-Forge.

#include "Common_3/Application/Interfaces/IApp.h"

class SpaceInvadersForge: public IApp
{
public:
    // ciclo de vida (uma vez por processo)
    bool Init() override;
    void Exit() override;

    // ciclo de recursos (inicializacao + resize/fullscreen/reload de shader)
    bool Load(ReloadDesc* pReloadDesc) override;
    void Unload(ReloadDesc* pReloadDesc) override;

    // o quadro (uma vez por frame, nesta ordem)
    void Update(float deltaTime) override;
    void Draw() override;

    const char* GetName() override;

private:
    bool addSwapChain();
};
