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
#include "GameApp.h"

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
    std::shared_ptr<GameApp> m_game;

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
        m_compositor = winrt::Compositor();

        // Base visual tree
        m_root = m_compositor.CreateSpriteVisual();
        m_root.RelativeSizeAdjustment({ 1, 1 });
        m_root.Brush(m_compositor.CreateColorBrush({ 255, 70, 70, 70 })); // ARGB
        m_target = m_compositor.CreateTargetForCurrentView();
        m_target.Root(m_root);
    }

    void OnPointerPressed(winrt::CoreWindow const& window, winrt::PointerEventArgs const & args)
    {
        winrt::float2 const point = args.CurrentPoint().Position();
        m_game->OnPointerPressed(
            point,
            args.CurrentPoint().Properties().IsRightButtonPressed(),
            args.CurrentPoint().Properties().IsEraser());
    }

    void OnPointerMoved(winrt::CoreWindow const& window, winrt::PointerEventArgs const & args)
    {
        winrt::float2 const point = args.CurrentPoint().Position();
        m_game->OnPointerMoved(point);
        
    }

    void OnPointerReleased(winrt::CoreWindow const& window, winrt::PointerEventArgs const& args)
    {
        winrt::float2 const point = args.CurrentPoint().Position();
        m_game->OnPointerReleased(
            point,
            args.CurrentPoint().Properties().IsRightButtonPressed(),
            args.CurrentPoint().Properties().IsEraser());
    }

    void App::OnSizeChanged(winrt::CoreWindow const& window, winrt::WindowSizeChangedEventArgs const&)
    {
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        m_game->OnParentSizeChanged(windowSize);
    }

    void App::OnKeyUp(winrt::CoreWindow const& window, winrt::KeyEventArgs const& args)
    {
        auto key = args.VirtualKey();
        const auto isControlDown = (window.GetKeyState(winrt::VirtualKey::Control) & winrt::CoreVirtualKeyStates::Down) == winrt::CoreVirtualKeyStates::Down;
        m_game->OnKeyUp(key, isControlDown);
    }

    winrt::IAsyncAction InitializeAsync(winrt::CoreWindow const& window)
    {
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto tempWindow = window;
        auto temp = this;
        auto dispatcher = window.Dispatcher();
        m_game = co_await GameApp::CreateAsync(winrt::DispatcherQueue::GetForCurrentThread(), m_root, windowSize);
        co_await dispatcher;

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
