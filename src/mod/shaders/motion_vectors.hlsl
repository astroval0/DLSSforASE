// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

cbuffer MotionConstants : register(b0) {
    row_major float4x4 ClipToPrevClip;
    int2 Offset;
    uint2 Size;
};

Texture2D<float> DepthTexture : register(t0);
Texture2D<float> VelocityTexture : register(t1);
RWTexture2D<float2> MotionTexture : register(u0);

[numthreads(8, 8, 1)]
void main(uint3 id : SV_DispatchThreadID) {
    int2 pixel = Offset + int2(id.xy);
    float depth = DepthTexture.Load(int3(pixel, 0));
    float2 ndc = (id.xy + 0.5) / Size * float2(2.0, -2.0) + float2(-1.0, 1.0);

    float4 prev = mul(float4(ndc, depth, 1.0), ClipToPrevClip);
    float2 motion = prev.xy / prev.w - ndc;

    // 0 means nothing moving got drawn here, also what a null srv reads when the engine skipped velocity lol
    float2 velocity = VelocityTexture.Load(int3(pixel, 0));

    if (velocity.x > 0.0) {
        // engine packs this frame minus the last frame in ndc (EncodeVelocityToTexture)
        motion = -(velocity - 32767.0 / 65535.0) / (0.499 * 0.5);
    }

    // dlss wants pixels pointing from this frame to the last one, y goes down
    MotionTexture[id.xy] = motion * Size * float2(0.5, -0.5);
}