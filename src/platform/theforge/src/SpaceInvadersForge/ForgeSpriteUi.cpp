#include "ForgeSpriteUi.h"

#include "Common_3/Resources/ResourceLoader/Interfaces/IResourceLoader.h"
#include "Common_3/Utilities/Interfaces/ILog.h"

// fsl + SRT do sprite (mesmo header dos .fsl — ver sprite.srt.h)
#include "Common_3/Graphics/FSL/defaults.h"
#include "Shaders/FSL/sprite.srt.h"

namespace {

// Vertice do batcher: posicao ja em NDC, UV no atlas e tint RGBA8 —
// R8G8B8A8_UNORM no vertex layout, o input assembler entrega float4 ao
// shader. 20 bytes/vertice, 6 vertices/sprite (2 triangulos, sem indices).
struct SpriteVertex
{
    float2   position;
    float2   uv;
    uint32_t color; // ABGR na memoria = R,G,B,A em bytes little-endian
};

// Lote generoso para a PoC: a horda inteira usa ~70. Estourou, dropa e loga.
constexpr uint32_t kMaxSprites = 2048;
constexpr uint32_t kVertsPerSprite = 6;
constexpr uint32_t kMaxVerts = kMaxSprites * kVertsPerSprite;

Renderer*      gRenderer = NULL;
Texture*       gAtlasTexture = NULL;
Sampler*       gSampler = NULL;
Shader*        gShader = NULL;
Pipeline*      gPipeline = NULL;
Buffer*        gVertexBuffer = NULL; // kMaxVerts * frameCount, CPU_TO_GPU
DescriptorSet* gDescriptorSet = NULL;

uint32_t gFrameCount = 0;

// estado do quadro corrente
Cmd*     gCmd = NULL;
float    gWidth = 0.0f;
float    gHeight = 0.0f;
uint32_t gFrameIndex = 0;
uint32_t gBaseVertex = 0;  // inicio do trecho deste frame no vertex buffer
uint32_t gFlushedVerts = 0; // ja desenhados neste quadro (cursor)
uint32_t gPendingVerts = 0; // acumulados aguardando flush
bool     gOverflowLogged = false;

forgesprite::Stats gCurrent = {};
forgesprite::Stats gLastFrame = {};

// staging na CPU: drawSprite escreve aqui; flush copia para o trecho do VB
SpriteVertex gStaging[kMaxVerts];

float gAtlasW = 1.0f;
float gAtlasH = 1.0f;

} // namespace

namespace forgesprite {

void init(Renderer* renderer, uint32_t frameCount)
{
    gRenderer = renderer;
    gFrameCount = frameCount;

    TextureLoadDesc atlasDesc = {};
    atlasDesc.pFileName = "atlas.dds";
    atlasDesc.ppTexture = &gAtlasTexture;
    addResource(&atlasDesc, NULL);

    SamplerDesc samplerDesc = { FILTER_NEAREST,
                                FILTER_NEAREST,
                                MIPMAP_MODE_NEAREST,
                                ADDRESS_MODE_CLAMP_TO_EDGE,
                                ADDRESS_MODE_CLAMP_TO_EDGE,
                                ADDRESS_MODE_CLAMP_TO_EDGE };
    addSampler(gRenderer, &samplerDesc, &gSampler);

    // Um unico buffer com um trecho por frame in flight, persistentemente
    // mapeado (receita do UI middleware): escrever no trecho do frame N e
    // seguro porque a fence do cmd ring garante que a GPU ja consumiu o
    // trecho ha frameCount quadros — o bug classico deste degrau e escrever
    // no trecho que a GPU ainda le.
    BufferLoadDesc vbDesc = {};
    vbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    vbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    vbDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    vbDesc.mDesc.mSize = (uint64_t)kMaxVerts * frameCount * sizeof(SpriteVertex);
    vbDesc.mDesc.pName = "Sprite Batcher Vertex Buffer";
    vbDesc.ppBuffer = &gVertexBuffer;
    addResource(&vbDesc, NULL);
}

void exit()
{
    removeResource(gVertexBuffer);
    removeResource(gAtlasTexture);
    removeSampler(gRenderer, gSampler);
    gRenderer = NULL;
}

void load(const ReloadDesc* reloadDesc, TinyImageFormat colorFormat, SampleCount sampleCount, uint32_t sampleQuality)
{
    if (reloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        ShaderLoadDesc shaderDesc = {};
        shaderDesc.mVert.pFileName = "sprite.vert";
        shaderDesc.mFrag.pFileName = "sprite.frag";
        addShader(gRenderer, &shaderDesc, &gShader);

        DescriptorSetDesc setDesc = SRT_SET_DESC(SpriteSrtData, Persistent, 1, 0);
        addDescriptorSet(gRenderer, &setDesc, &gDescriptorSet);
    }

    if (reloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        VertexLayout vertexLayout = {};
        vertexLayout.mBindingCount = 1;
        vertexLayout.mBindings[0].mStride = sizeof(SpriteVertex);
        vertexLayout.mAttribCount = 3;
        vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
        vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32_SFLOAT;
        vertexLayout.mAttribs[0].mBinding = 0;
        vertexLayout.mAttribs[0].mLocation = 0;
        vertexLayout.mAttribs[0].mOffset = 0;
        vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
        vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
        vertexLayout.mAttribs[1].mBinding = 0;
        vertexLayout.mAttribs[1].mLocation = 1;
        vertexLayout.mAttribs[1].mOffset = sizeof(float2);
        vertexLayout.mAttribs[2].mSemantic = SEMANTIC_TEXCOORD1;
        vertexLayout.mAttribs[2].mFormat = TinyImageFormat_R8G8B8A8_UNORM;
        vertexLayout.mAttribs[2].mBinding = 0;
        vertexLayout.mAttribs[2].mLocation = 2;
        vertexLayout.mAttribs[2].mOffset = sizeof(float2) * 2;

        RasterizerStateDesc rasterizerStateDesc = {};
        rasterizerStateDesc.mCullMode = CULL_MODE_NONE;

        // alpha blending straight (receita FontSystem/UI)
        BlendStateDesc blendStateDesc = {};
        blendStateDesc.mSrcFactors[0] = BC_SRC_ALPHA;
        blendStateDesc.mDstFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
        blendStateDesc.mSrcAlphaFactors[0] = BC_SRC_ALPHA;
        blendStateDesc.mDstAlphaFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
        blendStateDesc.mColorWriteMasks[0] = COLOR_MASK_ALL;
        blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_ALL;
        blendStateDesc.mIndependentBlend = false;

        PipelineDesc desc = {};
        desc.mType = PIPELINE_TYPE_GRAPHICS;
        PIPELINE_LAYOUT_DESC(desc, SRT_LAYOUT_DESC(SpriteSrtData, Persistent), NULL, NULL, NULL);
        GraphicsPipelineDesc& pipelineSettings = desc.mGraphicsDesc;
        pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        pipelineSettings.mRenderTargetCount = 1;
        pipelineSettings.pDepthState = NULL; // 2D: camadas = ordem de draw
        pipelineSettings.pBlendState = &blendStateDesc;
        pipelineSettings.pColorFormats = &colorFormat;
        pipelineSettings.mSampleCount = sampleCount;
        pipelineSettings.mSampleQuality = sampleQuality;
        pipelineSettings.pShaderProgram = gShader;
        pipelineSettings.pVertexLayout = &vertexLayout;
        pipelineSettings.pRasterizerState = &rasterizerStateDesc;
        addPipeline(gRenderer, &desc, &gPipeline);

        // dimensoes reais do atlas para converter regioes px -> UV
        gAtlasW = (float)gAtlasTexture->mWidth;
        gAtlasH = (float)gAtlasTexture->mHeight;
    }

    // escreve textura+sampler no set (apos os adds, como no 01_Transformations)
    DescriptorData params[2] = {};
    params[0].mIndex = SRT_RES_IDX(SpriteSrtData, Persistent, gSpriteTexture);
    params[0].ppTextures = &gAtlasTexture;
    params[1].mIndex = SRT_RES_IDX(SpriteSrtData, Persistent, gSpriteSampler);
    params[1].ppSamplers = &gSampler;
    updateDescriptorSet(gRenderer, 0, gDescriptorSet, TF_ARRAY_COUNT(params), params);
}

void unload(const ReloadDesc* reloadDesc)
{
    if (reloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET))
    {
        removePipeline(gRenderer, gPipeline);
    }

    if (reloadDesc->mType & RELOAD_TYPE_SHADER)
    {
        removeShader(gRenderer, gShader);
        removeDescriptorSet(gRenderer, gDescriptorSet);
    }
}

void begin(Cmd* cmd, const float width, const float height, const uint32_t frameIndex)
{
    gCmd = cmd;
    gWidth = width;
    gHeight = height;
    gFrameIndex = frameIndex;
    gBaseVertex = frameIndex * kMaxVerts;
    gFlushedVerts = 0;
    gPendingVerts = 0;
    gOverflowLogged = false;

    gLastFrame = gCurrent;
    gCurrent = {};
}

void drawSprite(const SpriteRegion& region, const float x, const float y, const float scale, const uint32_t colorAbgr)
{
    if (gFlushedVerts + gPendingVerts + kVertsPerSprite > kMaxVerts)
    {
        if (!gOverflowLogged)
        {
            LOGF(eWARNING, "[forgesprite] lote cheio (%u sprites) — sprite dropado", kMaxSprites);
            gOverflowLogged = true;
        }
        return;
    }

    const float x0 = x / gWidth * 2.0f - 1.0f;
    const float x1 = (x + region.w * scale) / gWidth * 2.0f - 1.0f;
    const float y0 = 1.0f - y / gHeight * 2.0f;
    const float y1 = 1.0f - (y + region.h * scale) / gHeight * 2.0f;

    const float u0 = region.x / gAtlasW;
    const float u1 = (region.x + region.w) / gAtlasW;
    const float v0 = region.y / gAtlasH;
    const float v1 = (region.y + region.h) / gAtlasH;

    SpriteVertex* v = &gStaging[gFlushedVerts + gPendingVerts];
    v[0] = { { x0, y0 }, { u0, v0 }, colorAbgr };
    v[1] = { { x1, y0 }, { u1, v0 }, colorAbgr };
    v[2] = { { x1, y1 }, { u1, v1 }, colorAbgr };
    v[3] = { { x0, y0 }, { u0, v0 }, colorAbgr };
    v[4] = { { x1, y1 }, { u1, v1 }, colorAbgr };
    v[5] = { { x0, y1 }, { u0, v1 }, colorAbgr };

    gPendingVerts += kVertsPerSprite;
    ++gCurrent.sprites;
}

void flush()
{
    if (gPendingVerts == 0 || gCmd == NULL)
    {
        return;
    }

    // copia so o lote pendente para o trecho deste frame, a partir do cursor
    BufferUpdateDesc update = { gVertexBuffer, (uint64_t)(gBaseVertex + gFlushedVerts) * sizeof(SpriteVertex),
                                (uint64_t)gPendingVerts * sizeof(SpriteVertex) };
    beginUpdateResource(&update);
    memcpy(update.pMappedData, &gStaging[gFlushedVerts], gPendingVerts * sizeof(SpriteVertex));
    endUpdateResource(&update);

    const uint32_t stride = sizeof(SpriteVertex);
    cmdBindPipeline(gCmd, gPipeline);
    cmdBindDescriptorSet(gCmd, 0, gDescriptorSet);
    cmdBindVertexBuffer(gCmd, 1, &gVertexBuffer, &stride, NULL);
    cmdDraw(gCmd, gPendingVerts, gBaseVertex + gFlushedVerts);

    gFlushedVerts += gPendingVerts;
    gPendingVerts = 0;
    ++gCurrent.drawCalls;
}

Stats lastFrameStats() { return gLastFrame; }

} // namespace forgesprite
