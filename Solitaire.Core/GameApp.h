#pragma once
#include "ShapeCache.h"
#include "Game.h"

class GameApp : public ISolitaire
{
public:
    GameApp(
        std::shared_ptr<ShapeCache> shapeCache,
        winrt::Windows::UI::Composition::ContainerVisual const& parentVisual,
        winrt::Windows::Foundation::Numerics::float2 parentSize);
    ~GameApp() {}

    void OnPointerMoved(winrt::Windows::Foundation::Numerics::float2 point) override;
    void OnParentSizeChanged(winrt::Windows::Foundation::Numerics::float2 newSize) override;
    void OnPointerPressed(
        winrt::Windows::Foundation::Numerics::float2 point,
        bool isRightButton,
        bool isEraser) override;
    void OnPointerReleased(
        winrt::Windows::Foundation::Numerics::float2 point,
        bool isRightButton,
        bool isEraser) override;
    void OnKeyUp(
        winrt::Windows::System::VirtualKey key,
        bool isControlDown) override;

private:
    void PrintTree(winrt::Windows::Foundation::Numerics::float2 windowSize);
    winrt::Windows::Foundation::Numerics::float2 GetPointRelativeToContent(
        winrt::Windows::Foundation::Numerics::float2 const windowSize,
        winrt::Windows::Foundation::Numerics::float2 const point);


private:
    winrt::Windows::Foundation::Numerics::float2 m_lastParentSize;
    std::unique_ptr<Game> m_game;
    winrt::Windows::UI::Composition::ContainerVisual m_root{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_background{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_content{ nullptr };
};