#include "ForgeUi.h"

#include <vector>

#include "Common_3/OS/Interfaces/IInput.h"

namespace {

// Bindings proprios (NAO incluir OS/Input/InputCommon.h: ele define os globals
// gInputValues etc. e duplicaria os simbolos do OS.lib â€” aprendizado do
// degrau 2). "released" = valor 1 apenas no quadro em que a tecla e solta.
struct Bindings {
    InputEnum up = {};
    InputEnum down = {};
    InputEnum left = {};
    InputEnum right = {};
    InputEnum enter = {};
    InputEnum escape = {};
    InputEnum backspace = {};
};
Bindings gBindings;

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

void pushIfPressed(const InputEnum binding, const Key key)
{
    if (inputGetValue(0, binding) > 0.0f)
    {
        push(key);
    }
}

} // namespace

namespace forgeui {

void initBindings()
{
    inputAddCustomBindings("si_up; button; K_UPARROW; released\n"
                           "si_down; button; K_DOWNARROW; released\n"
                           "si_left; button; K_LEFTARROW; released\n"
                           "si_right; button; K_RIGHTARROW; released\n"
                           "si_enter; button; K_ENTER; released\n"
                           "si_escape; button; K_ESCAPE; released\n"
                           "si_backspace; button; K_BACKSPACE; released");
    gBindings.up = inputGetCustomBindingEnum("si_up");
    gBindings.down = inputGetCustomBindingEnum("si_down");
    gBindings.left = inputGetCustomBindingEnum("si_left");
    gBindings.right = inputGetCustomBindingEnum("si_right");
    gBindings.enter = inputGetCustomBindingEnum("si_enter");
    gBindings.escape = inputGetCustomBindingEnum("si_escape");
    gBindings.backspace = inputGetCustomBindingEnum("si_backspace");
}

void beginInput(const bool acceptInput)
{
    if (!acceptInput)
    {
        return;
    }

    pushIfPressed(gBindings.up, Key::Up);
    pushIfPressed(gBindings.down, Key::Down);
    pushIfPressed(gBindings.left, Key::Left);
    pushIfPressed(gBindings.right, Key::Right);
    pushIfPressed(gBindings.enter, Key::Enter);
    pushIfPressed(gBindings.escape, Key::Escape);
    pushIfPressed(gBindings.backspace, Key::Backspace);

    // Caracteres digitados (digitos do jogo, T/M dos recordes, nome do
    // recorde). Controles (<32) ja chegam pelos bindings; ASCII apenas â€”
    // acentos (char32 > 126) sao descartados na entrada de nome.
    char32_t* chars = NULL;
    uint32_t  count = 0;
    inputGetCharInput(&chars, &count);
    for (uint32_t i = 0; i < count; ++i)
    {
        if (chars[i] >= 32 && chars[i] < 127)
        {
            push(Key::Char, (char)chars[i]);
        }
    }
}

void pushKey(const KeyEvent event) { push(event.key, event.character); }

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
