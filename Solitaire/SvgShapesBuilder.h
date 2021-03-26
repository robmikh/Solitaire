#pragma once

class SvgShapesBuilder 
{
public:
    static winrt::Windows::UI::Composition::ShapeVisual ConvertSvgDocumentToCompositionShapes(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::Microsoft::Graphics::Canvas::Svg::CanvasSvgDocument const& document);

private:
    SvgShapesBuilder() {}

    struct IBrushInfo
    {
        virtual winrt::Windows::UI::Composition::CompositionBrush CreateBrush(winrt::Windows::UI::Composition::Compositor const& compositor) = 0;
    };

    struct ColorBrushInfo : public IBrushInfo
    {
        winrt::Windows::UI::Color Color;

        ColorBrushInfo(winrt::Windows::UI::Color color) { Color = color; }
        winrt::Windows::UI::Composition::CompositionBrush CreateBrush(winrt::Windows::UI::Composition::Compositor const& compositor) override;
    };

    struct GradientStopInfo
    {
        float Offset;
        winrt::Windows::UI::Color Color;
    };

    struct LinearGradientBrushInfo : public IBrushInfo
    {
        std::vector<GradientStopInfo> Stops;

        LinearGradientBrushInfo(std::vector<GradientStopInfo> stops) { Stops = stops; }
        winrt::Windows::UI::Composition::CompositionBrush CreateBrush(winrt::Windows::UI::Composition::Compositor const& compositor) override;
    };

    struct Presentation
    {
        std::shared_ptr<IBrushInfo> Fill;
        std::shared_ptr<IBrushInfo> Stroke;
        float StrokeWidth;
    };

    static void ProcessSvgElement(
        std::stack<Presentation>& presentationStack,
        winrt::Windows::UI::Composition::CompositionContainerShape const& parentShape,
        winrt::Microsoft::Graphics::Canvas::Svg::ICanvasSvgElement const& element);

    static std::shared_ptr<IBrushInfo> GetBrushInfo(
        winrt::Microsoft::Graphics::Canvas::Svg::CanvasSvgNamedElement const& element,
        winrt::hstring const& attributeName);

    static std::shared_ptr<IBrushInfo> CreateBrushInfoFromId(
        winrt::Microsoft::Graphics::Canvas::Svg::CanvasSvgDocument const& document,
        winrt::hstring const& id);

    static std::shared_ptr<IBrushInfo> CreateLinearGradientBrushInfo(
        winrt::Microsoft::Graphics::Canvas::Svg::CanvasSvgNamedElement const& element);

    static winrt::Windows::UI::Composition::CompositionBrush CreateBrushFromBrushInfo(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        std::shared_ptr<IBrushInfo> const& brushInfo);
};