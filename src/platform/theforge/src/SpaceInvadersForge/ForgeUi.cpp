#include "ForgeUi.h"

#include <vector>

#include "ForgeSpriteUi.h"

namespace {

// snapshot do estado segurado, publicado por pushHeldState (WndProc do
// TheForgeWindowManager) a cada quadro
float gMoveAxis = 0.0f;
bool  gFireHeld = false;

// Fila de eventos: o app enfileira no Update; as cenas consomem 1 por input().
constexpr size_t     kQueueMax = 32;
std::vector<KeyEvent> gQueue;

Cmd*     gCmd = NULL;
float    gWidth = 0.0f;
float    gHeight = 0.0f;
uint32_t gFontID = 0;

void push(const Key key, const char character = '\0')
{
    if (gQueue.size() < kQueueMax)
    {
        gQueue.push_back({ key, character });
    }
}

} // namespace

namespace forgeui {

void pushKey(const KeyEvent event) { push(event.key, event.character); }

void pushHeldState(const float moveAxis, const bool fireHeld)
{
    gMoveAxis = moveAxis;
    gFireHeld = fireHeld;
}

void beginDraw(Cmd* cmd, const float width, const float height, const uint32_t fontID)
{
    gCmd = cmd;
    gWidth = width;
    gHeight = height;
    gFontID = fontID;
}

KeyEvent readKey()
{
    if (gQueue.empty())
    {
        return {};
    }
    const KeyEvent event = gQueue.front();
    gQueue.erase(gQueue.begin());
    return event;
}

float moveAxis() { return gMoveAxis; }
bool  fireHeld() { return gFireHeld; }

float screenWidth() { return gWidth; }
float screenHeight() { return gHeight; }

float textWidth(const std::string& text, const float fontSize)
{
    FontDrawDesc desc = {};
    desc.pText = text.c_str();
    desc.mFontID = gFontID;
    desc.mFontSize = fontSize;
    return fntMeasureFontText(desc.pText, &desc).x;
}

void drawText(const std::string& text, const float x, const float y, const float fontSize, const uint32_t colorAbgr)
{
    // Camadas = ordem de chamada atravessando as pontes: sprites pendentes
    // sao desenhados AGORA, para este texto ficar por cima (degrau 3).
    forgesprite::flush();

    FontDrawDesc desc = {};
    desc.pText = text.c_str();
    desc.mFontID = gFontID;
    desc.mFontColor = colorAbgr;
    desc.mFontSize = fontSize;
    cmdDrawTextWithFont(gCmd, float2(x, y), &desc);
}

void drawTextCentered(const std::string& text, const float y, const float fontSize, const uint32_t colorAbgr)
{
    drawText(text, (gWidth - textWidth(text, fontSize)) * 0.5f, y, fontSize, colorAbgr);
}

void drawHints(const std::string& text) { drawTextCentered(text, gHeight - 48.0f, 18.0f, color::kDim); }

} // namespace forgeui
