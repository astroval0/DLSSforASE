// Astro's DLSSforASE - https://github.com/astroval0/DLSSforASE
// Copyright (c) 2026 AstroVal0. Licensed under Apache-2.0, see LICENSE and NOTICE.

#pragma once

#include <d3d11.h>
#include <string>

bool InitDlss(ID3D11Device* device, const std::wstring& dllDirectory);
