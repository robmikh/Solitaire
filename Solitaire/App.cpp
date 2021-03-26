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
        window.Activate();

        winrt::CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(winrt::CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(winrt::CoreWindow const & window)
    {
        winrt::float2 const windowSize = { window.Bounds().Width, window.Bounds().Height };

        m_compositor = winrt::Compositor();

        // Base visual tree
        m_root = m_compositor.CreateSpriteVisual();
        m_root.RelativeSizeAdjustment({ 1, 1 });
        m_root.Brush(m_compositor.CreateColorBrush({ 255, 70, 70, 70 })); // ARGB
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

        // DEBUG: Svg card rendering
        auto dispatcher = window.Dispatcher();
        dispatcher.RunAsync(winrt::CoreDispatcherPriority::Normal, [=]() -> winrt::fire_and_forget
            {
                auto root = m_root;
                auto compositor = m_compositor;
                auto device = winrt::Microsoft::Graphics::Canvas::CanvasDevice();

                auto file = co_await winrt::Windows::Storage::StorageFile::GetFileFromApplicationUriAsync(winrt::Uri(L"ms-appx:///Assets/CardFaces/2_of_spades.svg"));
                {
                    auto stream = co_await file.OpenReadAsync();
                    auto document = co_await winrt::Microsoft::Graphics::Canvas::Svg::CanvasSvgDocument::LoadAsync(device, stream);

                    auto visual = SvgShapesBuilder::ConvertSvgDocumentToCompositionShapes(compositor, document);
                    root.Children().InsertAtTop(visual);
                }
            });
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

    void App::OnSizeChanged(winrt::CoreWindow const& window, winrt::WindowSizeChangedEventArgs const& args)
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
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    winrt::CoreApplication::Run(winrt::make<App>());
}
