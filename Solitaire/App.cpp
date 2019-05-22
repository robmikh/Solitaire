#include "pch.h"
#include "ShapeCache.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    CompositionTarget m_target{ nullptr };
    VisualCollection m_visuals{ nullptr };
    Visual m_selected{ nullptr };
    float2 m_offset{};

    std::unique_ptr<ShapeCache> m_shapeCache;

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
        Compositor compositor;
        m_shapeCache = std::make_unique<ShapeCache>(compositor);
        ContainerVisual root = compositor.CreateContainerVisual();
        m_target = compositor.CreateTargetForCurrentView();
        m_target.Root(root);
        m_visuals = root.Children();

        auto cardVisual = BuildCard(compositor, L"K♥", Colors::Crimson());
        m_visuals.InsertAtTop(cardVisual);
        cardVisual = BuildCard(compositor, L"Q♠", Colors::Black());
        m_visuals.InsertAtTop(cardVisual);

        window.PointerPressed({ this, &App::OnPointerPressed });
        window.PointerMoved({ this, &App::OnPointerMoved });

        window.PointerReleased([&](auto && ...)
        {
            m_selected = nullptr;
        });
    }

    Visual BuildCard(
        Compositor const& compositor, 
        hstring const& card,
        Color const& color)
    {
        auto shapeVisual = compositor.CreateShapeVisual();
        auto shapeContainer = compositor.CreateContainerShape();
        shapeVisual.Shapes().Append(shapeContainer);
        shapeVisual.Size({ 175, 250 });

        auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        roundedRectGeometry.CornerRadius({ 10, 10 });
        roundedRectGeometry.Size({ 175, 250 });
        auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
        rectShape.StrokeBrush(compositor.CreateColorBrush(Colors::Gray()));
        rectShape.FillBrush(compositor.CreateColorBrush(Colors::White()));
        rectShape.StrokeThickness(2);
        shapeContainer.Shapes().Append(rectShape);

        auto pathGeometry = m_shapeCache->GetPathGeometry(card);
        auto pathShape = compositor.CreateSpriteShape(pathGeometry);
        pathShape.FillBrush(compositor.CreateColorBrush(color));
        shapeContainer.Shapes().Append(pathShape);

        return shapeVisual;
    }

    void OnPointerPressed(IInspectable const &, PointerEventArgs const & args)
    {
        float2 const point = args.CurrentPoint().Position();

        for (Visual visual : m_visuals)
        {
            float3 const offset = visual.Offset();
            float2 const size = visual.Size();

            if (point.x >= offset.x &&
                point.x < offset.x + size.x &&
                point.y >= offset.y &&
                point.y < offset.y + size.y)
            {
                m_selected = visual;
                m_offset.x = offset.x - point.x;
                m_offset.y = offset.y - point.y;
            }
        }

        if (m_selected)
        {
            m_visuals.Remove(m_selected);
            m_visuals.InsertAtTop(m_selected);
        }
    }

    void OnPointerMoved(IInspectable const &, PointerEventArgs const & args)
    {
        if (m_selected)
        {
            float2 const point = args.CurrentPoint().Position();

            m_selected.Offset(
            {
                point.x + m_offset.x,
                point.y + m_offset.y,
                0.0f
            });
        }
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
