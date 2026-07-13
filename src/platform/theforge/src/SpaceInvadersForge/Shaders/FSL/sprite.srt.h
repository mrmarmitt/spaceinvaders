// Degrau 2 da PoC sprites: o primeiro SRT (Shader Resource Table) do projeto.
//
// Este header e compartilhado entre o FSL (via #include nos .fsl) e o C++
// (via #include no SpaceInvadersForge.cpp, depois do defaults.h) - cada lado
// expande as macros do seu jeito, e e isso que mantem shader e descriptor
// set em acordo (SRT_SET_DESC/SRT_LAYOUT_DESC/SRT_RES_IDX derivam daqui).
//
// BEGIN_SRT_NO_AB como no 01_Transformations (sem argument buffers de iOS).

#pragma once

BEGIN_SRT_NO_AB(SpriteSrtData)
    BEGIN_SRT_SET(Persistent)
        DECL_TEXTURE(Persistent, Tex2D(float4), gSpriteTexture)
        DECL_SAMPLER(Persistent, SamplerState, gSpriteSampler)
    END_SRT_SET(Persistent)
END_SRT(SpriteSrtData)
