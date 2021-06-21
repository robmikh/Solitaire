#include "pch.h"
#include "Card.h"
#include "ShapeCache.h"
#include "CompositionCard.h"
#include "Pack.h"
#include "CardStack.h"
#include "Waste.h"
#include "Deck.h"
#include "Foundation.h"
#include "GameApp.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI::Composition;
    using namespace Windows::System;
}

float ComputeScaleFactor(winrt::float2 const windowSize, winrt::float2 const contentSize)
{
    auto windowRatio = windowSize.x / windowSize.y;
    auto contentRatio = contentSize.x / contentSize.y;

    auto scaleFactor = windowSize.x / contentSize.x;
    if (windowRatio > contentRatio)
    {
        scaleFactor = windowSize.y / contentSize.y;
    }

    return scaleFactor;
}

winrt::float4x4 ComputeContentTransform(winrt::float2 const windowSize, winrt::float2 const contentSize)
{
    auto result = winrt::float4x4::identity();
    winrt::float2 const anchorPoint = { 0.5f, 0.5f };
    auto scale = ComputeScaleFactor(windowSize, contentSize);

    result *=
        winrt::make_float4x4_translation({ -1.0f * anchorPoint * contentSize, 0 }) *
        winrt::make_float4x4_scale({ scale, scale, 1.0f }) *
        winrt::make_float4x4_translation({ windowSize / 2.0f, 0 });

    return result;
}

float ComputeRadius(winrt::float2 const windowSize)
{
    return std::sqrt((windowSize.x * windowSize.x) + (windowSize.y * windowSize.y)) / 2.0f;
}

std::future<std::shared_ptr<GameApp>> GameApp::CreateAsync(
    winrt::DispatcherQueue uiThread,
    winrt::ContainerVisual parentVisual, 
    winrt::float2 parentSize)
{
    auto compositor = parentVisual.Compositor();
    co_await winrt::resume_foreground(uiThread);
    auto shapeCache = co_await ShapeCache::CreateAsync(compositor);
    auto app = std::make_shared<GameApp>(shapeCache, parentVisual, parentSize);
    co_return app;
}

GameApp::GameApp(
    std::shared_ptr<ShapeCache> shapeCache, 
    winrt::ContainerVisual const& parentVisual, 
    winrt::float2 parentSize)
{
    auto compositor = parentVisual.Compositor();
    // Base visual tree
    m_root = compositor.CreateSpriteVisual();
    m_root.RelativeSizeAdjustment({ 1, 1 });
    m_root.Comment(L"Application Root");
    parentVisual.Children().InsertAtTop(m_root);

    m_background = compositor.CreateSpriteVisual();
    m_background.AnchorPoint({ 0.5f, 0.5f });
    m_background.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0 });
    auto diameter = ComputeRadius(parentSize) * 2.0f;
    m_background.Size({ diameter, diameter });
    auto backgroundBrush = compositor.CreateRadialGradientBrush();
    backgroundBrush.ColorStops().Append(compositor.CreateColorGradientStop(0.0f, { 255, 14, 144, 58 }));
    backgroundBrush.ColorStops().Append(compositor.CreateColorGradientStop(1.0f, { 255, 7, 69, 32 }));
    m_background.Brush(backgroundBrush);
    m_root.Children().InsertAtBottom(m_background);

    m_content = compositor.CreateContainerVisual();
    m_content.Size({ 1327, 1111 });
    m_content.AnchorPoint({ 0.5f, 0.5f });
    m_content.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
    auto scale = ComputeScaleFactor(parentSize, m_content.Size());
    m_content.Scale({ scale, scale, 1.0f });
    m_root.Children().InsertAtTop(m_content);

    auto size = m_content.Size();
    m_game = std::make_unique<Game>(compositor, size, shapeCache);
    m_content.Children().InsertAtTop(m_game->Root());
}

void GameApp::OnPointerMoved(winrt::float2 point)
{
    auto relativePoint = GetPointRelativeToContent(m_lastParentSize, point);
    m_game->OnPointerMoved(relativePoint);
}

void GameApp::OnParentSizeChanged(winrt::float2 newSize)
{
    m_lastParentSize = newSize;
    auto scale = ComputeScaleFactor(newSize, m_content.Size());
    m_content.Scale({ scale, scale, 1.0f });
    m_game->OnSizeChanged(m_content.Size());
    // Update the background
    auto diameter = ComputeRadius(newSize) * 2.0f;
    m_background.Size({ diameter, diameter });
}

void GameApp::OnPointerPressed(
    winrt::float2 point,
    bool isRightButton,
    bool isEraser)
{
    auto relativePoint = GetPointRelativeToContent(m_lastParentSize, point);
    m_game->OnPointerPressed(relativePoint);
}

void GameApp::OnPointerReleased(
    winrt::float2 point,
    bool isRightButton,
    bool isEraser)
{
    auto relativePoint = GetPointRelativeToContent(m_lastParentSize, point);
    m_game->OnPointerReleased(relativePoint);
}

void GameApp::OnKeyUp(winrt::VirtualKey key, bool isControlDown)
{
    // If an animation is going, ignore the key
    if (m_game->IsAnimating())
    {
        return;
    }

    if (key == winrt::VirtualKey::T && isControlDown)
    {
        PrintTree(m_lastParentSize);
    }
    else if (key == winrt::VirtualKey::Up ||
        key == winrt::VirtualKey::Down ||
        key == winrt::VirtualKey::Left ||
        key == winrt::VirtualKey::Right)
    {
        auto layout = m_game->LayoutInfo();

        if (key == winrt::VirtualKey::Up)
        {
            layout.CardStackVerticalOffset -= 5.0f;
        }
        else if (key == winrt::VirtualKey::Down)
        {
            layout.CardStackVerticalOffset += 5.0f;
        }
        else if (key == winrt::VirtualKey::Left)
        {
            layout.WasteHorizontalOffset -= 5.0f;
        }
        else if (key == winrt::VirtualKey::Right)
        {
            layout.WasteHorizontalOffset += 5.0f;
        }

        m_game->LayoutInfo(layout);
    }
    else if (key == winrt::VirtualKey::N && isControlDown)
    {
        m_game->NewGame();
    }
}

void GameApp::PrintTree(winrt::float2 windowSize)
{
    std::wstringstream stringStream;
    stringStream << L"Window Size: " << windowSize.x << L", " << windowSize.y << std::endl;
    Debug::PrintTree(m_root, stringStream, 0);
    Debug::OutputDebugStringStream(stringStream);
}

winrt::float2 GameApp::GetPointRelativeToContent(winrt::float2 const windowSize, winrt::float2 const point)
{
    auto transform = ComputeContentTransform(windowSize, m_content.Size());
    auto interse = winrt::float4x4::identity();
    if (invert(transform, &interse))
    {
        return winrt::Windows::Foundation::Numerics::transform(point, interse);
    }
    return { -1, -1 };
}