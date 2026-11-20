// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

cbuffer MotionConstants : register(b0) {
    row_major float4x4 ClipToPrevClip;
    int2 Offset;
    uint2 Size;
};

Texture2D<float> DepthTexture : register(t0);
RWTexture2D<float2> MotionTexture : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    float depth = DepthTexture.Load(int3(Offset + int2(id.xy), 0));
    float2 ndc = (id.xy + 0.5) / Size * float2(2.0, -2.0) + float2(-1.0, 1.0);

    float4 prev = mul(float4(ndc, depth, 1.0), ClipToPrevClip);

    // dlss wants pixels pointing from this frame to the last one, y goes down
    MotionTexture[id.xy] = (prev.xy / prev.w - ndc) * Size * float2(0.5, -0.5);
}