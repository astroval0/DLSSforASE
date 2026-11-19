// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

Texture2D<float4> InputTexture : register(t0);

float4 main(float4 position : SV_Position, float2 uv : TEXCOORD0) : SV_Target {
    uint width, height;
    InputTexture.GetDimensions(width, height);

    return InputTexture.Load(int3(uv * float2(width, height), 0));
}