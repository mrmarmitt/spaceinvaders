// SpaceInvadersForge — degrau 0 da PoC sprites (.ai/task/01-poc-sprites.md):
// o casco da plataforma The-Forge reinstanciado fora do repo do 8puzzle.
//
// Mesma receita do 8PuzzleForge.cpp: o IApp hospeda o EngineManager da
// cengine em MODO HOSPEDADO (cengine 0.4.0, task 15) — sem window manager
// (a janela e do host) e sem start(); o host dirige o quadro:
//
//   Update(dt) -> forgeui::beginInput + guarda o dt
//   Draw()     -> forgeui::beginDraw + engine.frame(dt); false -> requestShutdown()
//
// Cenas de teste A <-> B (ENTER navega, provando onEnter/onExit e a
// reativacao A->B->A com recriacao de cena) e exit (ESC). As cenas falam com
// a plataforma so atraves do forgeui (fila de teclado + texto imediato) —
// nos degraus seguintes o forgeui ganha o drawSprite() do batcher.

// cengine — C++ puro, ANTES dos headers do The-Forge (IMemory.h por ultimo).
#include <cengine/core/EngineManager.hpp>
#include <cengine/core/IScene.hpp>
#include <cengine/core/Time.hpp>
#include <cengine/routing/GameManager.hpp>
#include <cengine/routing/IState.hpp>
#include <cengine/routing/RouterInMemory.hpp>
#include <cengine/routing/SceneRepository.hpp>
#include <cengine/routing/StateCodes.hpp>

#include "ForgeUi.h"

#include <memory>
#include <string>

// Interfaces
#include "Common_3/Application/Interfaces/IApp.h"
#include "Common_3/Application/Interfaces/IFont.h"
#include "Common_3/Application/Interfaces/IUI.h"
#include "Common_3/Utilities/Interfaces/IFileSystem.h"
#include "Common_3/Utilities/Interfaces/ILog.h"

#include "Common_3/Utilities/RingBuffer.h"

// Renderer
#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "Common_3/Resources/ResourceLoader/Interfaces/IResourceLoader.h"

// fsl (INIT_RS_DESC)
#include "Common_3/Graphics/FSL/defaults.h"

#include "Common_3/Utilities/Interfaces/IMemory.h" // deve ser o ultimo include

// Duas cenas de recursos: uma em voo na GPU, uma sendo gravada na CPU.
const uint32_t gDataBufferCount = 2;

Renderer*  pRenderer = NULL;
Queue*     pGraphicsQueue = NULL;
GpuCmdRing gGraphicsCmdRing = {};

SwapChain* pSwapChain = NULL;
Semaphore* pImageAcquiredSemaphore = NULL;

uint32_t gFrameIndex = 0;
uint32_t gFontID = 0;

std::shared_ptr<cengine::routing::RouterInMemory> gRouter;
std::unique_ptr<cengine::core::EngineManager>     gEngine;

// dt do Update() do host, consumido pelo frame() no Draw() (receita do modo
// hospedado — ver task 15 da cengine).
float gDt = 0.0f;

/// Estado generico do degrau 0: codigo + nome legivel.
class AppState final: public cengine::routing::IState
{
    std::string m_code;
    std::string m_name;

public:
    AppState(std::string code, std::string name): m_code(std::move(code)), m_name(std::move(name)) {}

    [[nodiscard]] std::string getCode() const override { return m_code; }
    [[nodiscard]] std::string getName() const override { return m_name; }
};

/// Cena de teste: mostra o proprio nome, o tempo/updates acumulados (prova o
/// update(dt) com passo fixo e que a recriacao no reload zera o estado) e
/// navega com ENTER (proxima) / ESC (sair) via fila do forgeui.
class DemoScene final: public cengine::core::IScene
{
    const char* m_label;
    const char* m_nextCode;
    const char* m_nextLabel;
    uint32_t    m_accent; // ABGR

    double   m_elapsed = 0.0;
    uint64_t m_updates = 0;

public:
    DemoScene(const char* label, const char* nextCode, const char* nextLabel, uint32_t accent):
        m_label(label), m_nextCode(nextCode), m_nextLabel(nextLabel), m_accent(accent)
    {
    }

    void onEnter() override { LOGF(eINFO, "[cengine] onEnter: %s", m_label); }

    void update(cengine::core::Seconds dt) override
    {
        m_elapsed += dt.count();
        ++m_updates;
    }

    void input() override
    {
        const KeyEvent event = forgeui::readKey();
        if (event.key == Key::Enter)
        {
            gRouter->requestState(std::make_unique<AppState>(m_nextCode, m_nextLabel));
        }
        else if (event.key == Key::Escape)
        {
            gRouter->requestState(std::make_unique<AppState>(std::string(cengine::routing::kExitStateCode), "Exit"));
        }
    }

    void draw() override
    {
        const float height = forgeui::screenHeight();

        forgeui::drawTextCentered(m_label, height * 0.26f, 64.0f, m_accent);

        char status[128];
        snprintf(status, sizeof(status), "tempo na cena: %.1fs  |  updates: %llu", m_elapsed, (unsigned long long)m_updates);
        forgeui::drawTextCentered(status, height * 0.26f + 96.0f, 22.0f, forgeui::color::kText);

        char nav[128];
        snprintf(nav, sizeof(nav), "ENTER -> %s   |   ESC -> sair", m_nextLabel);
        forgeui::drawTextCentered(nav, height * 0.26f + 136.0f, 22.0f, forgeui::color::kDim);

        forgeui::drawHints("casco The-Forge + cengine hospedada — degrau 0 da PoC sprites");
    }

    void onExit() override { LOGF(eINFO, "[cengine] onExit: %s (%.1fs, %llu updates)", m_label, m_elapsed, (unsigned long long)m_updates); }
};

/// Cena do estado de saida: nunca chega a desenhar — o frame() devolve false
/// logo apos o commit da navegacao e o host pede o shutdown do framework.
class ExitScene final: public cengine::core::IScene
{
public:
    void onEnter() override {}
    void update(cengine::core::Seconds) override {}
    void draw() override {}
    void input() override {}
    void onExit() override {}
};

class SpaceInvadersForge: public IApp
{
public:
    bool Init()
    {
        // renderer
        RendererDesc settings;
        memset(&settings, 0, sizeof(settings));
        initGPUConfiguration(settings.pExtendedSettings);
        initRenderer(GetName(), &settings, &pRenderer);
        if (!pRenderer)
        {
            ShowUnsupportedMessage(getUnsupportedGPUMsg());
            return false;
        }
        setupGPUConfigurationPlatformParameters(pRenderer, settings.pExtendedSettings);

        QueueDesc queueDesc = {};
        queueDesc.mType = QUEUE_TYPE_GRAPHICS;
        queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
        initQueue(pRenderer, &queueDesc, &pGraphicsQueue);

        GpuCmdRingDesc cmdRingDesc = {};
        cmdRingDesc.pQueue = pGraphicsQueue;
        cmdRingDesc.mPoolCount = gDataBufferCount;
        cmdRingDesc.mCmdPerPoolCount = 1;
        cmdRingDesc.mAddSyncPrimitives = true;
        initGpuCmdRing(pRenderer, &cmdRingDesc, &gGraphicsCmdRing);

        initSemaphore(pRenderer, &pImageAcquiredSemaphore);

        initResourceLoaderInterface(pRenderer);

        RootSignatureDesc rootDesc = {};
        INIT_RS_DESC(rootDesc, "default.rootsig", "compute.rootsig");
        initRootSignature(pRenderer, &rootDesc);

        // fontes (mesma fonte dos unit tests; resolvida via PathStatement.txt)
        FontDesc font = {};
        font.pFontPath = "TitilliumText/TitilliumText-Bold.otf";
        fntDefineFonts(&font, 1, &gFontID);

        FontSystemDesc fontRenderDesc = {};
        fontRenderDesc.pRenderer = pRenderer;
        if (!initFontSystem(&fontRenderDesc))
            return false;

        UserInterfaceDesc uiRenderDesc = {};
        uiRenderDesc.pRenderer = pRenderer;
        initUserInterface(&uiRenderDesc);

        forgeui::initBindings();

        // montagem cengine: repositorio (factories) -> router -> engine.
        // As factories rodam so no primeiro getScene de cada estado — gRouter
        // ja esta atribuido nesse ponto.
        auto sceneRepository = std::make_unique<cengine::routing::SceneRepository>();
        sceneRepository->registerFactory("scene_a",
                                         [] { return std::make_unique<DemoScene>("CENA A", "scene_b", "Cena B", forgeui::color::kTitle); });
        sceneRepository->registerFactory("scene_b",
                                         [] { return std::make_unique<DemoScene>("CENA B", "scene_a", "Cena A", forgeui::color::kAccent); });
        sceneRepository->registerFactory(std::string(cengine::routing::kExitStateCode), [] { return std::make_unique<ExitScene>(); });

        gRouter = std::make_shared<cengine::routing::RouterInMemory>(
            std::move(sceneRepository), std::make_unique<AppState>("scene_a", "Cena A"));

        // Modo hospedado: sem window manager (janela e do The-Forge) e sem
        // start() — o Draw() dirige via frame(dt).
        gEngine = std::make_unique<cengine::core::EngineManager>(
            nullptr, std::make_unique<cengine::routing::GameManager>(gRouter));

        waitForAllResourceLoads();

        return true;
    }

    void Exit()
    {
        gEngine->cleanup(); // no modo hospedado o cleanup e do host
        gEngine.reset();
        gRouter.reset();

        exitUserInterface();

        exitFontSystem();

        exitGpuCmdRing(pRenderer, &gGraphicsCmdRing);
        exitSemaphore(pRenderer, pImageAcquiredSemaphore);

        exitRootSignature(pRenderer);
        exitResourceLoaderInterface(pRenderer);

        exitQueue(pRenderer, pGraphicsQueue);

        exitRenderer(pRenderer);
        exitGPUConfiguration();
        pRenderer = NULL;
    }

    bool Load(ReloadDesc* pReloadDesc)
    {
        if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
        {
            if (!addSwapChain())
                return false;
        }

        UserInterfaceLoadDesc uiLoad = {};
        uiLoad.mColorFormat = pSwapChain->ppRenderTargets[0]->mFormat;
        uiLoad.mHeight = mSettings.mHeight;
        uiLoad.mWidth = mSettings.mWidth;
        uiLoad.mLoadType = pReloadDesc->mType;
        loadUserInterface(&uiLoad);

        FontSystemLoadDesc fontLoad = {};
        fontLoad.mColorFormat = pSwapChain->ppRenderTargets[0]->mFormat;
        fontLoad.mHeight = mSettings.mHeight;
        fontLoad.mWidth = mSettings.mWidth;
        fontLoad.mLoadType = pReloadDesc->mType;
        loadFontSystem(&fontLoad);

        return true;
    }

    void Unload(ReloadDesc* pReloadDesc)
    {
        waitQueueIdle(pGraphicsQueue);

        unloadFontSystem(pReloadDesc->mType);
        unloadUserInterface(pReloadDesc->mType);

        if (pReloadDesc->mType & (RELOAD_TYPE_RESIZE | RELOAD_TYPE_RENDERTARGET))
        {
            removeSwapChain(pRenderer, pSwapChain);
        }
    }

    void Update(float deltaTime)
    {
        // O quadro inteiro roda no frame() do Draw(); aqui so capturamos o
        // input do host e guardamos o dt que ele mediu.
        forgeui::beginInput(!uiIsFocused());
        gDt = deltaTime;
    }

    void Draw()
    {
        if ((bool)pSwapChain->mEnableVsync != mSettings.mVSyncEnabled)
        {
            waitQueueIdle(pGraphicsQueue);
            ::toggleVSync(pRenderer, &pSwapChain);
        }

        uint32_t swapchainImageIndex;
        acquireNextImage(pRenderer, pSwapChain, pImageAcquiredSemaphore, NULL, &swapchainImageIndex);

        RenderTarget*     pRenderTarget = pSwapChain->ppRenderTargets[swapchainImageIndex];
        GpuCmdRingElement elem = getNextGpuCmdRingElement(&gGraphicsCmdRing, true, 1);

        // Espera se a CPU estiver gDataBufferCount quadros a frente da GPU.
        FenceStatus fenceStatus;
        getFenceStatus(pRenderer, elem.pFence, &fenceStatus);
        if (fenceStatus == FENCE_STATUS_INCOMPLETE)
            waitForFences(pRenderer, 1, &elem.pFence);

        resetCmdPool(pRenderer, elem.pCmdPool);

        Cmd* cmd = elem.pCmds[0];
        beginCmd(cmd);

        RenderTargetBarrier barriers[] = {
            { pRenderTarget, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET },
        };
        cmdResourceBarrier(cmd, 0, NULL, 0, NULL, 1, barriers);

        BindRenderTargetsDesc bindRenderTargets = {};
        bindRenderTargets.mRenderTargetCount = 1;
        bindRenderTargets.mRenderTargets[0] = { pRenderTarget, LOAD_ACTION_CLEAR };
        bindRenderTargets.mDepthStencil = { NULL, LOAD_ACTION_DONTCARE };
        cmdBindRenderTargets(cmd, &bindRenderTargets);
        cmdSetViewport(cmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
        cmdSetScissor(cmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

        // a cena atual desenha atraves do alvo publicado no forgeui; o
        // frame() executa o quadro completo da cengine (fases + fixed
        // timestep) e devolve false quando o jogo pediu saida.
        forgeui::beginDraw(cmd, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, gFontID);
        const bool keepRunning = gEngine->frame(cengine::core::Seconds{ (double)gDt });

        cmdDrawUserInterface(cmd);

        cmdBindRenderTargets(cmd, NULL);

        barriers[0] = { pRenderTarget, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT };
        cmdResourceBarrier(cmd, 0, NULL, 0, NULL, 1, barriers);

        endCmd(cmd);

        FlushResourceUpdateDesc flushUpdateDesc = {};
        flushUpdateDesc.mNodeIndex = 0;
        flushResourceUpdates(&flushUpdateDesc);
        Semaphore* waitSemaphores[2] = { flushUpdateDesc.pOutSubmittedSemaphore, pImageAcquiredSemaphore };

        QueueSubmitDesc submitDesc = {};
        submitDesc.mCmdCount = 1;
        submitDesc.mSignalSemaphoreCount = 1;
        submitDesc.mWaitSemaphoreCount = TF_ARRAY_COUNT(waitSemaphores);
        submitDesc.ppCmds = &cmd;
        submitDesc.ppSignalSemaphores = &elem.pSemaphore;
        submitDesc.ppWaitSemaphores = waitSemaphores;
        submitDesc.pSignalFence = elem.pFence;
        queueSubmit(pGraphicsQueue, &submitDesc);

        QueuePresentDesc presentDesc = {};
        presentDesc.mIndex = (uint8_t)swapchainImageIndex;
        presentDesc.mWaitSemaphoreCount = 1;
        presentDesc.pSwapChain = pSwapChain;
        presentDesc.ppWaitSemaphores = &elem.pSemaphore;
        presentDesc.mSubmitDone = true;

        queuePresent(pGraphicsQueue, &presentDesc);

        gFrameIndex = (gFrameIndex + 1) % gDataBufferCount;

        if (!keepRunning)
        {
            requestShutdown();
        }
    }

    const char* GetName() { return "SpaceInvadersForge"; }

    bool addSwapChain()
    {
        SwapChainDesc swapChainDesc = {};
        swapChainDesc.mWindowHandle = pWindow->handle;
        swapChainDesc.mPresentQueueCount = 1;
        swapChainDesc.ppPresentQueues = &pGraphicsQueue;
        swapChainDesc.mWidth = mSettings.mWidth;
        swapChainDesc.mHeight = mSettings.mHeight;
        swapChainDesc.mImageCount = getRecommendedSwapchainImageCount(pRenderer, &pWindow->handle);
        swapChainDesc.mColorFormat = getSupportedSwapchainFormat(pRenderer, &swapChainDesc, COLOR_SPACE_SDR_SRGB);
        swapChainDesc.mColorSpace = COLOR_SPACE_SDR_SRGB;
        swapChainDesc.mColorClearValue = { { 0.02f, 0.02f, 0.05f, 1.0f } }; // preto-espaco
        swapChainDesc.mEnableVsync = mSettings.mVSyncEnabled;
        ::addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);

        return pSwapChain != NULL;
    }
};

DEFINE_APPLICATION_MAIN(SpaceInvadersForge)
