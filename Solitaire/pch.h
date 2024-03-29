﻿#pragma once

// Windows
#include <windows.h>
#include <Unknwn.h>
#include <inspectable.h>

#include <wil/cppwinrt.h>

// WinRT
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Numerics.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.Popups.h>

// WIL
#include <wil/resource.h>

// D3D
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d2d1_3.h>
#include <wincodec.h>

// STL
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <algorithm>
#include <random>
#include <queue>
#include <stack>
#include <type_traits>
#include <sstream>
#include <future>

// Common
#include "robmikh.common/composition.interop.h"
#include "robmikh.common/d3dHelpers.h"
#include "robmikh.common/direct3d11.interop.h"
#include "robmikh.common/graphics.interop.h"
#include "robmikh.common/stream.interop.h"
