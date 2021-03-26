#include "pch.h"
#include "SvgShapesBuilder.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Microsoft::Graphics::Canvas::Svg;
}

SvgCompositionShapes SvgShapesBuilder::ConvertSvgDocumentToCompositionShapes(
    winrt::Compositor const& compositor, 
    winrt::CanvasSvgDocument const& document)
{
    auto rootVisual = compositor.CreateShapeVisual();

    // Assumption: There is only one "svg" element and it is at the root.
    // Techincally this is incorrect, as the spec specifically states that
    // having multiple "svg" elements embeded in each other is possible.
    // https://www.w3.org/TR/SVG11/struct.html
    auto rootElement = document.Root();
    WINRT_VERIFY(rootElement.Tag() == L"svg");

    winrt::CompositionViewBox viewBox{ nullptr };

    if (rootElement.IsAttributeSpecified(L"viewBox"))
    {
        auto rect = rootElement.GetRectangleAttribute(L"viewBox");
        viewBox = compositor.CreateViewBox();
        viewBox.Size({ rect.Width, rect.Height });
        viewBox.Offset({ rect.X, rect.Y });
    }

    auto container = compositor.CreateContainerShape();
    std::stack<Presentation> presentationStack;
    presentationStack.push(Presentation
        {
            std::make_shared<ColorBrushInfo>(winrt::Colors::Black()),
            std::make_shared<ColorBrushInfo>(winrt::Colors::Transparent()),
            1.0f
        });

    auto child = rootElement.FirstChild();
    while (child != nullptr)
    {
        ProcessSvgElement(presentationStack, container, child);
        child = rootElement.GetNextSibling(child);
    }

    return { viewBox, container };
}

void SvgShapesBuilder::ProcessSvgElement(
    std::stack<Presentation>& presentationStack, 
    winrt::CompositionContainerShape const& parentShape,
    winrt::ICanvasSvgElement const& element)
{
    if (auto current = element.try_as<winrt::CanvasSvgNamedElement>())
    {
        auto compositor = parentShape.Compositor();
        auto currentShape = compositor.CreateContainerShape();
        parentShape.Shapes().Append(currentShape);

        auto& currentPresentation = presentationStack.top();
        Presentation presentation = currentPresentation;

        // Record the id for debugging
        if (current.IsAttributeSpecified(L"id"))
        {
            auto id = current.GetIdAttribute(L"id");
            currentShape.Comment(id);
        }

        // General attributes
        auto attributeNames = current.SpecifiedAttributes();
        for (auto&& attributeName : attributeNames) 
        {
            if (attributeName == L"x")
            {
                auto offset = currentShape.Offset();
                auto value = current.GetFloatAttribute(attributeName);
                offset.x = value;
            }
            else if (attributeName == L"y")
            {
                auto offset = currentShape.Offset();
                auto value = current.GetFloatAttribute(attributeName);
                offset.y = value;
            }
            else if (attributeName == L"fill")
            {
                presentation.Fill = GetBrushInfo(current, attributeName);
            }
            else if (attributeName == L"stroke")
            {
                presentation.Stroke = GetBrushInfo(current, attributeName);
            }
            else if (attributeName == L"stroke-width")
            {
                auto strokeWidth = current.GetFloatAttribute(attributeName);
                presentation.StrokeWidth = strokeWidth;
            }
            else if (attributeName == L"transform")
            {
                auto transform = current.GetTransformAttribute(attributeName);
                currentShape.TransformMatrix(transform);
            }
        }

        // Special cases
        auto tag = current.Tag();
        if (tag == L"defs")
        {
            // TODO: Keep track of children, prevent them from rendering
            // For now, let's just disconnect ourselves from the tree. A bit nasty.
            parentShape.Shapes().RemoveAtEnd();
        }
        else if (tag == L"use")
        {
            auto href = current.GetIdAttribute(L"xlink:href");
            auto document = current.ContainingDocument();
            auto reference = document.FindElementById(href);
            ProcessSvgElement(presentationStack, currentShape, reference);
        }
        else if (tag == L"circle")
        {
            auto centerX = current.GetFloatAttribute(L"cx");
            auto centerY = current.GetFloatAttribute(L"cy");
            auto radius = current.GetFloatAttribute(L"r");

            auto geometry = compositor.CreateEllipseGeometry();
            geometry.Center({ centerX, centerY });
            geometry.Radius({ radius, radius });
            auto spriteShape = compositor.CreateSpriteShape();

            spriteShape.FillBrush(CreateBrushFromBrushInfo(compositor, presentation.Fill));
            spriteShape.StrokeBrush(CreateBrushFromBrushInfo(compositor, presentation.Stroke));
            spriteShape.StrokeThickness(presentation.StrokeWidth);

            currentShape.Shapes().Append(spriteShape);
        }
        else if (tag == L"rect")
        {
            auto x = current.GetFloatAttribute(L"x");
            auto y = current.GetFloatAttribute(L"y");
            auto width = current.GetFloatAttribute(L"width");
            auto height = current.GetFloatAttribute(L"height");

            auto geometry = compositor.CreateRectangleGeometry();
            geometry.Offset({ x, y });
            geometry.Size({ width, height });
            auto spriteShape = compositor.CreateSpriteShape();

            spriteShape.FillBrush(CreateBrushFromBrushInfo(compositor, presentation.Fill));
            spriteShape.StrokeBrush(CreateBrushFromBrushInfo(compositor, presentation.Stroke));
            spriteShape.StrokeThickness(presentation.StrokeWidth);

            currentShape.Shapes().Append(spriteShape);
        }
        else if (tag == L"g") { }
        else if (tag == L"path")
        {
            auto attribute = current.GetAttribute(L"d").as<winrt::CanvasSvgPathAttribute>();
            auto canvasGeometry = attribute.CreatePathGeometry();
            auto compositionPath = winrt::CompositionPath(canvasGeometry);
            auto pathGeometry = compositor.CreatePathGeometry(compositionPath);
            auto spriteShape = compositor.CreateSpriteShape(pathGeometry);

            spriteShape.FillBrush(CreateBrushFromBrushInfo(compositor, presentation.Fill));
            spriteShape.StrokeBrush(CreateBrushFromBrushInfo(compositor, presentation.Stroke));
            spriteShape.StrokeThickness(presentation.StrokeWidth);

            currentShape.Shapes().Append(spriteShape);
        }
        // Currently these are handled elsewhere.
        else if (tag == L"stop") {}
        else if (tag == L"linearGradient") {}
        else if (tag == L"radialGradient") {}
        else
        {
            // Record unknown tags for debugging purposes
            std::wstringstream message;
            message << L"Unknown tag: " << tag.c_str() << std::endl;
            OutputDebugStringW(message.str().c_str());
        }

        presentationStack.push(presentation);

        // Process children
        if (current.HasChildren())
        {
            auto child = current.FirstChild();
            while (child != nullptr)
            {
                ProcessSvgElement(presentationStack, currentShape, child);
                child = current.GetNextSibling(child);
            }
        }

        presentationStack.pop();
    }
}

std::shared_ptr<SvgShapesBuilder::IBrushInfo> SvgShapesBuilder::GetBrushInfo(
    winrt::CanvasSvgNamedElement const& element, 
    winrt::hstring const& attributeName)
{
    auto temp = element.GetAttribute(attributeName);
    if (auto attribute = temp.try_as<winrt::CanvasSvgPaintAttribute>())
    {
        switch (attribute.PaintType())
        {
        case winrt::CanvasSvgPaintType::Color:
            return std::make_shared<ColorBrushInfo>(attribute.Color());
        case winrt::CanvasSvgPaintType::Uri:
            return CreateBrushInfoFromId(element.ContainingDocument(), attribute.Id());
        default:
            break;
        }
    }
    return nullptr;
}

std::shared_ptr<SvgShapesBuilder::IBrushInfo> SvgShapesBuilder::CreateBrushInfoFromId(
    winrt::CanvasSvgDocument const& document, 
    winrt::hstring const& id)
{
    auto reference = document.FindElementById(id);
    auto tag = reference.Tag();
    if (tag == L"linearGradient")
    {
        return CreateLinearGradientBrushInfo(reference);
    }
    return nullptr;
}

std::shared_ptr<SvgShapesBuilder::IBrushInfo> SvgShapesBuilder::CreateLinearGradientBrushInfo(
    winrt::CanvasSvgNamedElement const& element)
{
    std::vector<GradientStopInfo> stops;
    if (element.HasChildren())
    {
        auto child = element.FirstChild();
        while (child != nullptr)
        {
            if (auto current = child.try_as<winrt::CanvasSvgNamedElement>())
            {
                auto offset = current.GetFloatAttribute(L"offset");
                auto color = current.GetColorAttribute(L"stop-color");
                auto stop = GradientStopInfo{ offset, color };
                stops.push_back(stop);
            }
        }
    }
    return std::make_shared<LinearGradientBrushInfo>(stops);
}

winrt::CompositionBrush SvgShapesBuilder::CreateBrushFromBrushInfo(
    winrt::Compositor const& compositor, 
    std::shared_ptr<IBrushInfo> const& brushInfo)
{
    return brushInfo == nullptr ? nullptr : brushInfo->CreateBrush(compositor);
}

winrt::CompositionBrush SvgShapesBuilder::ColorBrushInfo::CreateBrush(winrt::Compositor const& compositor)
{
    return compositor.CreateColorBrush(Color);
}

winrt::CompositionBrush SvgShapesBuilder::LinearGradientBrushInfo::CreateBrush(winrt::Compositor const& compositor)
{
    auto brush = compositor.CreateLinearGradientBrush();
    auto colorStops = brush.ColorStops();
    for (auto&& stop : Stops)
    {
        auto compositionStop = compositor.CreateColorGradientStop(stop.Offset, stop.Color);
        colorStops.Append(compositionStop);
    }
    return brush;
}
