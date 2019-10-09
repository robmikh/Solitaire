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

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::System;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Popups;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    Compositor m_compositor{ nullptr };
    CompositionTarget m_target{ nullptr };
    ContainerVisual m_root{ nullptr };

    std::unique_ptr<Game> m_game;
    ContainerVisual m_content{ nullptr };

    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const &)
    {
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

        CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(CoreWindow const & window)
    {
        float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };

        m_compositor = Compositor();

        // Base visual tree
        m_root = m_compositor.CreateContainerVisual();
        m_root.RelativeSizeAdjustment({ 1, 1 });
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
        
        m_game = std::make_unique<Game>(m_compositor, m_content.Size());
        m_content.Children().InsertAtTop(m_game->Root());

        window.PointerPressed({ this, &App::OnPointerPressed });
        window.PointerMoved({ this, &App::OnPointerMoved });
        window.PointerReleased({ this, &App::OnPointerReleased });
        window.SizeChanged({ this, &App::OnSizeChanged });
        window.KeyUp({ this, &App::OnKeyUp });
    }

    float ComputeScaleFactor(float2 const windowSize, float2 const contentSize)
    {
        auto diffX = contentSize.x - windowSize.x;
        auto diffY = contentSize.y - windowSize.y;

        auto scaleX = windowSize.x / contentSize.x;
        auto scaleY = windowSize.y / contentSize.y;

        float scaleFactor = scaleY;

        if (diffX > diffY)
        {
            scaleFactor = scaleX;
        }

        return scaleFactor;
    }

    float4x4 ComputeContentTransform(float2 const windowSize, float2 const contentSize)
    {
        auto result = float4x4::identity();
        float2 const anchorPoint = { 0.5f, 0.5f };
        auto scale = ComputeScaleFactor(windowSize, contentSize);

        result *=
            make_float4x4_translation({ -1.0f * anchorPoint *contentSize, 0 }) *
            make_float4x4_scale({ scale, scale, 1.0f }) *
            make_float4x4_translation({ windowSize / 2.0f, 0 });

        return result;
    }

    float2 GetPointRelativeToContent(float2 const windowSize, float2 const point)
    {
        auto transform = ComputeContentTransform(windowSize, m_content.Size());
        auto temp = winrt::Windows::Foundation::Numerics::transform(point, transform);
        auto temp2 = float4x4::identity();
        invert(transform, &temp2);
        auto temp3 = winrt::Windows::Foundation::Numerics::transform(point, temp2);
        return temp3;
    }

    void OnPointerPressed(CoreWindow const& window, PointerEventArgs const & args)
    {
        float2 const point = args.CurrentPoint().Position();
        float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto relativePoint = GetPointRelativeToContent(windowSize, point);
        m_game->OnPointerPressed(relativePoint);
    }

    void OnPointerMoved(CoreWindow const& window, PointerEventArgs const & args)
    {
        float2 const point = args.CurrentPoint().Position();
        float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto relativePoint = GetPointRelativeToContent(windowSize, point);
        m_game->OnPointerMoved(relativePoint);
    }

    void OnPointerReleased(CoreWindow const& window, PointerEventArgs const& args)
    {
        float2 const point = args.CurrentPoint().Position();
        float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto relativePoint = GetPointRelativeToContent(windowSize, point);
        m_game->OnPointerReleased(relativePoint);
    }

    void App::OnSizeChanged(CoreWindow const& window, WindowSizeChangedEventArgs const& args)
    {
        float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };
        auto scale = ComputeScaleFactor(windowSize, m_content.Size());
        m_content.Scale({ scale, scale, 1.0f });
        m_game->OnSizeChanged(m_content.Size());
    }

    void App::OnKeyUp(CoreWindow const& window, KeyEventArgs const& args)
    {
        // If an animation is going, ignore the key
        if (m_game->IsAnimating())
        {
            return;
        }
        const auto isControlDown = (window.GetKeyState(VirtualKey::Control) & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down;

        auto key = args.VirtualKey();
        if (key == VirtualKey::T && isControlDown)
        {
            PrintTree(window);
        }
        else if (key == VirtualKey::Up ||
            key == VirtualKey::Down ||
            key == VirtualKey::Left ||
            key == VirtualKey::Right)
        {
            auto layout = m_game->LayoutInfo();

            if (key == VirtualKey::Up)
            {
                layout.CardStackVerticalOffset -= 5.0f;
            }
            else if (key == VirtualKey::Down)
            {
                layout.CardStackVerticalOffset += 5.0f;
            }
            else if (key == VirtualKey::Left)
            {
                layout.WasteHorizontalOffset -= 5.0f;
            }
            else if (key == VirtualKey::Right)
            {
                layout.WasteHorizontalOffset += 5.0f;
            }

            m_game->LayoutInfo(layout);
        }
        else if (key == VirtualKey::N && isControlDown)
        {
            m_game->NewGame();
        }
    }

    void PrintTree(CoreWindow const& window)
    {
        std::wstringstream stringStream;
        stringStream << L"Window Size: " << window.Bounds().Width << L", " << window.Bounds().Height << std::endl;
        Debug::PrintTree(m_root, stringStream, 0);
        Debug::OutputDebugStringStream(stringStream);
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
