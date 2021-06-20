#pragma once

struct SvgCompositionShapes
{
    winrt::Windows::UI::Composition::CompositionViewBox ViewBox;
    winrt::Windows::UI::Composition::CompositionContainerShape RootShape;
};

class SvgShapesBuilder 
{
public:
    static SvgCompositionShapes ConvertSvgDocumentToCompositionShapes(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::com_ptr<ID2D1SvgDocument> const& document);

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
        winrt::com_ptr<ID2D1SvgElement> const& element);

    static std::shared_ptr<IBrushInfo> GetBrushInfo(
        winrt::com_ptr<ID2D1SvgElement> const& element,
        std::wstring const& attributeName);

    static std::shared_ptr<IBrushInfo> CreateBrushInfoFromId(
        winrt::com_ptr<ID2D1SvgDocument> const& document,
        std::wstring const& id);

    static std::shared_ptr<IBrushInfo> CreateLinearGradientBrushInfo(
        winrt::com_ptr<ID2D1SvgElement> const& element);

    static winrt::Windows::UI::Composition::CompositionBrush CreateBrushFromBrushInfo(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        std::shared_ptr<IBrushInfo> const& brushInfo);
};