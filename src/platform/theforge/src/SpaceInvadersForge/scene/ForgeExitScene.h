#pragma once

#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>

// Cena do estado "exit" (kExitStateCode): nunca chega a desenhar — o
// frame() devolve false logo apos o commit da navegacao e o host encerra.
class ForgeExitScene final: public cengine::core::IScene
{
public:
    void onEnter() override {}
    void update(cengine::core::Seconds) override {}
    void draw() override {}
    void input() override {}
    void onExit() override {}
};
