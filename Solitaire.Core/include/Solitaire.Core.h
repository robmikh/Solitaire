#pragma once

class ISolitaire
{
public:
    virtual ~ISolitaire() {}

    virtual void OnPointerMoved(winrt::Windows::Foundation::Numerics::float2 point) = 0;
    virtual void OnParentSizeChanged(winrt::Windows::Foundation::Numerics::float2 newSize) = 0;
    virtual void OnPointerPressed(
        winrt::Windows::Foundation::Numerics::float2 point,
        bool isRightButton,
        bool isEraser) = 0;
    virtual void OnPointerReleased(
        winrt::Windows::Foundation::Numerics::float2 point,
        bool isRightButton,
        bool isEraser) = 0;
    virtual void OnKeyUp(
        winrt::Windows::System::VirtualKey key,
        bool isControlDown) = 0;
};

std::future<std::shared_ptr<ISolitaire>> CreateSolitaireAsync(
    winrt::Windows::UI::Composition::ContainerVisual parentVisual,
    winrt::Windows::Foundation::Numerics::float2 parentSize);