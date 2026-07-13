#pragma once

// TheForgeWindowManager — o The-Forge como BIBLIOTECA atras do port
// IWindowManager da cengine (mesma receita do 8puzzle fase 2, degrau 2 da
// task 02 daquele repo). A cengine e dona do loop (EngineManager::start());
// este window manager encapsula TUDO de janela/GPU no par update()/present()
// da 0.5.0:
//
//   init()     janela Win32 propria + cola do fontstash + renderer + fontes
//              + batcher de sprites (forgesprite) + swapchain
//   update()   pump de mensagens (WndProc alimenta a fila de teclado E o
//              estado segurado do forgeui), resize pendente (recria
//              swapchain), acquire da imagem, beginCmd + barriers +
//              forgesprite::begin + forgeui::beginDraw
//   present()  forgesprite::flush (cauda do lote), fecha o command buffer,
//              submit e queuePresent
//   cleanup()  teardown na ordem inversa
//
// As cenas nao conhecem esta classe: desenham/leem teclado via forgeui e
// forgesprite, o mesmo contrato do modo hospedado — nenhuma cena mudou na
// migracao IApp -> IWindowManager.

#include <cstdint>

#include <cengine/core/IWindowManager.hpp>

class TheForgeWindowManager final: public cengine::core::IWindowManager
{
public:
    /// @param appName nome do app (initRenderer, titulo da janela).
    /// @param width/height tamanho INICIAL do client area; a janela e
    ///        redimensionavel (WM_SIZE recria o swapchain no update()).
    TheForgeWindowManager(const char* appName, int32_t width, int32_t height);

    void init() override;
    void update() override;
    void present() override;
    void cleanup() override;

private:
    bool initGraphics();
    void applyPendingResize();

    const char* m_appName;
    int32_t     m_width;
    int32_t     m_height;
};
