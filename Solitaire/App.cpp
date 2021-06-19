#include "pch.h"
#include "Card.h"
#include "ShapeCache.h"
#include "CompositionCard.h"
#include "Pack.h"
#include "CardStack.h"
#include "Waste.h"
#include "Deck.h"
#include "Foundation.h"
#include "Game.h"

// DEBUG
#include "SvgShapesBuilder.h"

namespace winrt
{
    using namespace Windows;
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::System;
    using namespace Windows::UI;
    using namespace Windows::UI::Core;
    using namespace Windows::UI::Composition;
    using namespace Windows::UI::Popups;
}

struct App : winrt::implements<App, winrt::IFrameworkViewSource, winrt::IFrameworkView>
{
    winrt::Compositor m_compositor{ nullptr };
    winrt::CompositionTarget m_target{ nullptr };
    winrt::SpriteVisual m_root{ nullptr };

    std::unique_ptr<Game> m_game;
    winrt::ContainerVisual m_content{ nullptr };

    winrt::IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(winrt::CoreApplicationView const &)
    {
    }

    void Load(winrt::hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        winrt::CoreWindow window = winrt::CoreWindow::GetForCurrentThread();
        //window.Activate();

        winrt::CoreDispatcher dispatcher = window.Dispatcher();

        // Load assets
        dispatcher.RunAsync(winrt::CoreDispatcherPriority::Normal, [=]() -> winrt::fire_and_forget
            {
                co_await InitializeAsync(window);
            });
        dispatcher.ProcessEvents(winrt::CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(winrt::CoreWindow const & window)
    {
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };

        m_compositor = winrt::Compositor();

        // Base visual tree
        m_root = m_compositor.CreateSpriteVisual();
        m_root.RelativeSizeAdjustment({ 1, 1 });
        auto backgroundBrush = m_compositor.CreateRadialGradientBrush();
        backgroundBrush.ColorStops().Append(m_compositor.CreateColorGradientStop(0.0f, { 255, 14, 144, 58 })); // ARGB
        backgroundBrush.ColorStops().Append(m_compositor.CreateColorGradientStop(1.0f, { 255, 7, 69, 32 }));
        m_root.Brush(backgroundBrush);
        m_root.Comment(L"Application Root");
        m_target = m_compositor.CreateTargetForCurrentView();
        m_target.Root(m_root);

        m_content = m_compositor.CreateContainerVisual();
        m_content.Size({ 1327, 1111 });
        m_content.AnchorPoint({ 0.5f, 0.5f });
        m_content.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
        auto scale = ComputeScaleFactor(windowSize, m_content.Size());
        m_content.Scale({ scale, scale, 1.0f });
        m_root.Children().InsertAtTop(m_content);
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
            winrt::make_float4x4_translation({ -1.0f * anchorPoint *contentSize, 0 }) *
            winrt::make_float4x4_scale({ scale, scale, 1.0f }) *
            winrt::make_float4x4_translation({ windowSize / 2.0f, 0 });

        return result;
    }

    winrt::float2 GetPointRelativeToContent(winrt::float2 const windowSize, winrt::float2 const point)
    {
        auto transform = ComputeContentTransform(windowSize, m_content.Size());
        auto interse = winrt::float4x4::identity();
        if (invert(transform, &interse))
        {
            return winrt::Windows::Foundation::Numerics::transform(point, interse);
        }
        return { -1, -1 };
    }

    void OnPointerPressed(winrt::CoreWindow const& window, winrt::PointerEventArgs const & args)
    {
        winrt::float2 const point = args.CurrentPoint().Position();
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto relativePoint = GetPointRelativeToContent(windowSize, point);
        m_game->OnPointerPressed(relativePoint);
    }

    void OnPointerMoved(winrt::CoreWindow const& window, winrt::PointerEventArgs const & args)
    {
        winrt::float2 const point = args.CurrentPoint().Position();
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto relativePoint = GetPointRelativeToContent(windowSize, point);
        m_game->OnPointerMoved(relativePoint);
    }

    void OnPointerReleased(winrt::CoreWindow const& window, winrt::PointerEventArgs const& args)
    {
        winrt::float2 const point = args.CurrentPoint().Position();
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto relativePoint = GetPointRelativeToContent(windowSize, point);
        m_game->OnPointerReleased(relativePoint);
    }

    void App::OnSizeChanged(winrt::CoreWindow const& window, winrt::WindowSizeChangedEventArgs const&)
    {
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto scale = ComputeScaleFactor(windowSize, m_content.Size());
        m_content.Scale({ scale, scale, 1.0f });
        m_game->OnSizeChanged(m_content.Size());
    }

    void App::OnKeyUp(winrt::CoreWindow const& window, winrt::KeyEventArgs const& args)
    {
        // If an animation is going, ignore the key
        if (m_game->IsAnimating())
        {
            return;
        }
        const auto isControlDown = (window.GetKeyState(winrt::VirtualKey::Control) & winrt::CoreVirtualKeyStates::Down) == winrt::CoreVirtualKeyStates::Down;

        auto key = args.VirtualKey();
        if (key == winrt::VirtualKey::T && isControlDown)
        {
            PrintTree(window);
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

    void PrintTree(winrt::CoreWindow const& window)
    {
        std::wstringstream stringStream;
        stringStream << L"Window Size: " << window.Bounds().Width << L", " << window.Bounds().Height << std::endl;
        Debug::PrintTree(m_root, stringStream, 0);
        Debug::OutputDebugStringStream(stringStream);
    }

    winrt::IAsyncAction InitializeAsync(winrt::CoreWindow const& window)
    {
        auto tempWindow = window;
        auto temp = this;
        auto dispatcher = window.Dispatcher();
        auto shapeCache = co_await ShapeCache::CreateAsync(m_compositor);
        co_await dispatcher;
        auto size = m_content.Size();
        m_game = std::make_unique<Game>(m_compositor, size, shapeCache);
        m_content.Children().InsertAtTop(m_game->Root());

        tempWindow.PointerPressed({ temp, &App::OnPointerPressed });
        tempWindow.PointerMoved({ temp, &App::OnPointerMoved });
        tempWindow.PointerReleased({ temp, &App::OnPointerReleased });
        tempWindow.SizeChanged({ temp, &App::OnSizeChanged });
        tempWindow.KeyUp({ temp, &App::OnKeyUp });

        tempWindow.Activate();
        co_return;
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    winrt::CoreApplication::Run(winrt::make<App>());
}
