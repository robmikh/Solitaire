#include "pch.h"
#include "SvgShapesBuilder.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
}

namespace util
{
    using namespace robmikh::common::uwp;
}

winrt::Color D2DColorToWinRTColor(D2D1_COLOR_F const& color)
{
    return winrt::Color
    {
        static_cast<uint8_t>(color.a * 255.0f),
        static_cast<uint8_t>(color.r * 255.0f),
        static_cast<uint8_t>(color.g * 255.0f),
        static_cast<uint8_t>(color.b * 255.0f),
    };
}

std::wstring GetTag(winrt::com_ptr<ID2D1SvgElement> const& element)
{
    auto length = element->GetTagNameLength();
    std::wstring name(length + 1, 0);
    element->GetTagName(name.data(), name.length());
    name.resize(length);
    return name;
}

D2D1_SVG_VIEWBOX GetRectangleAttribute(winrt::com_ptr<ID2D1SvgElement> const& element, std::wstring const& attributeName)
{
    D2D1_SVG_VIEWBOX rect = {};
    winrt::check_hresult(element->GetAttributeValue(
        attributeName.c_str(),
        D2D1_SVG_ATTRIBUTE_POD_TYPE_VIEWBOX,
        static_cast<void*>(&rect),
        sizeof(rect)));
    return rect;
}

std::wstring GetIdAttribute(winrt::com_ptr<ID2D1SvgElement> const& element, std::wstring const& attributeName)
{
    uint32_t attributeLength = 0;
    winrt::check_hresult(element->GetAttributeValueLength(
        attributeName.c_str(),
        D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
        &attributeLength));
    std::wstring result(attributeLength + 1, 0);
    winrt::check_hresult(element->GetAttributeValue(
        attributeName.c_str(),
        D2D1_SVG_ATTRIBUTE_STRING_TYPE_ID,
        result.data(),
        result.length()));
    result.resize(attributeLength);
    return result;
}

float GetFloatAttribute(winrt::com_ptr<ID2D1SvgElement> const& element, std::wstring const& attributeName)
{
    float value = 0;
    winrt::check_hresult(element->GetAttributeValue(
        attributeName.c_str(),
        &value));
    return value;
}

winrt::float3x2 GetTransformAttribute(winrt::com_ptr<ID2D1SvgElement> const& element, std::wstring const& attributeName)
{
    winrt::float3x2 value = {};
    winrt::check_hresult(element->GetAttributeValue(
        attributeName.c_str(),
        reinterpret_cast<D2D1_MATRIX_3X2_F*>(&value)));
    return value;
}

winrt::Color GetColorAttribute(winrt::com_ptr<ID2D1SvgElement> const& element, std::wstring const& attributeName)
{
    D2D1_COLOR_F color = {};
    winrt::check_hresult(element->GetAttributeValue(
        attributeName.c_str(),
        &color));
    return D2DColorToWinRTColor(color);
}

std::vector<std::wstring> GetSpecifiedAttributes(winrt::com_ptr<ID2D1SvgElement> const& element)
{
    std::vector<std::wstring> names;
    uint32_t count = element->GetSpecifiedAttributeCount();
    for (auto i = 0; i < count; i++)
    {
        uint32_t length = 0;
        winrt::check_hresult(element->GetSpecifiedAttributeNameLength(i, &length, FALSE));
        std::wstring name(length + 1, 0);
        winrt::check_hresult(element->GetSpecifiedAttributeName(i, name.data(), name.length(), FALSE));
        name.resize(length);
        names.push_back(name);
    }
    return names;
}

SvgCompositionShapes SvgShapesBuilder::ConvertSvgDocumentToCompositionShapes(
    winrt::Compositor const& compositor, 
    winrt::com_ptr<ID2D1SvgDocument> const& document)
{
    auto rootVisual = compositor.CreateShapeVisual();

    // Assumption: There is only one "svg" element and it is at the root.
    // Techincally this is incorrect, as the spec specifically states that
    // having multiple "svg" elements embeded in each other is possible.
    // https://www.w3.org/TR/SVG11/struct.html
    winrt::com_ptr<ID2D1SvgElement> rootElement;
    document->GetRoot(rootElement.put());
    auto tag = GetTag(rootElement);
    WINRT_VERIFY(tag == L"svg");

    winrt::CompositionViewBox viewBox{ nullptr };

    if (rootElement->IsAttributeSpecified(L"viewBox"))
    {
        auto rect = GetRectangleAttribute(rootElement, L"viewBox");
        viewBox = compositor.CreateViewBox();
        viewBox.Size({ rect.width, rect.height });
        viewBox.Offset({ rect.x, rect.y });
    }

    auto container = compositor.CreateContainerShape();
    std::stack<Presentation> presentationStack;
    presentationStack.push(Presentation
        {
            std::make_shared<ColorBrushInfo>(winrt::Colors::Black()),
            std::make_shared<ColorBrushInfo>(winrt::Colors::Transparent()),
            1.0f
        });

    winrt::com_ptr<ID2D1SvgElement> child;
    rootElement->GetFirstChild(child.put());
    while (child != nullptr)
    {
        ProcessSvgElement(presentationStack, container, child);
        winrt::com_ptr<ID2D1SvgElement> newChild;
        winrt::check_hresult(rootElement->GetNextChild(child.get(), newChild.put()));
        child = newChild;
    }

    return { viewBox, container };
}

void SvgShapesBuilder::ProcessSvgElement(
    std::stack<Presentation>& presentationStack, 
    winrt::CompositionContainerShape const& parentShape,
    winrt::com_ptr<ID2D1SvgElement> const& element)
{
    auto current = element;
    if (!current->IsTextContent())
    {
        auto compositor = parentShape.Compositor();
        auto currentShape = compositor.CreateContainerShape();
        parentShape.Shapes().Append(currentShape);

        auto& currentPresentation = presentationStack.top();
        Presentation presentation = currentPresentation;

        // Record the id for debugging
        if (current->IsAttributeSpecified(L"id"))
        {
            auto id = GetIdAttribute(current, L"id");
            currentShape.Comment(id);
        }

        // General attributes
        auto attributeNames = GetSpecifiedAttributes(current);
        for (auto&& attributeName : attributeNames) 
        {
            if (attributeName == L"x")
            {
                auto offset = currentShape.Offset();
                auto value = GetFloatAttribute(current, attributeName);
                offset.x = value;
            }
            else if (attributeName == L"y")
            {
                auto offset = currentShape.Offset();
                auto value = GetFloatAttribute(current, attributeName);
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
                auto strokeWidth = GetFloatAttribute(current, attributeName);
                presentation.StrokeWidth = strokeWidth;
            }
            else if (attributeName == L"transform")
            {
                auto transform = GetTransformAttribute(current, attributeName);
                currentShape.TransformMatrix(transform);
            }
        }

        // Special cases
        auto tag = GetTag(current);
        if (tag == L"defs")
        {
            // TODO: Keep track of children, prevent them from rendering
            // For now, let's just disconnect ourselves from the tree. A bit nasty.
            parentShape.Shapes().RemoveAtEnd();
        }
        else if (tag == L"use")
        {
            auto href = GetIdAttribute(current, L"xlink:href");
            winrt::com_ptr<ID2D1SvgDocument> document;
            current->GetDocument(document.put());
            winrt::com_ptr<ID2D1SvgElement> reference;
            winrt::check_hresult(document->FindElementById(href.c_str(), reference.put()));
            ProcessSvgElement(presentationStack, currentShape, reference);
        }
        else if (tag == L"circle")
        {
            auto centerX = GetFloatAttribute(current, L"cx");
            auto centerY = GetFloatAttribute(current, L"cy");
            auto radius =  GetFloatAttribute(current, L"r");

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
            auto x = GetFloatAttribute(current, L"x");
            auto y = GetFloatAttribute(current, L"y");
            auto width = GetFloatAttribute(current, L"width");
            auto height = GetFloatAttribute(current, L"height");

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
            winrt::com_ptr<ID2D1SvgAttribute> attribute;
            winrt::check_hresult(current->GetAttributeValue(L"d", attribute.put()));
            auto pathAttribute = attribute.as<ID2D1SvgPathData>();
            winrt::com_ptr<ID2D1PathGeometry1> d2dGeometry;
            winrt::check_hresult(pathAttribute->CreatePathGeometry(D2D1_FILL_MODE_ALTERNATE, d2dGeometry.put()));
            auto geometrySource = winrt::make<util::GeometrySource>(d2dGeometry);
            auto compositionPath = winrt::CompositionPath(geometrySource);
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
        if (current->HasChildren())
        {
            winrt::com_ptr<ID2D1SvgElement> child;
            current->GetFirstChild(child.put());
            while (child != nullptr)
            {
                ProcessSvgElement(presentationStack, currentShape, child);
                winrt::com_ptr<ID2D1SvgElement> newChild;
                winrt::check_hresult(current->GetNextChild(child.get(), newChild.put()));
                child = newChild;
            }
        }

        presentationStack.pop();
    }
}

std::shared_ptr<SvgShapesBuilder::IBrushInfo> SvgShapesBuilder::GetBrushInfo(
    winrt::com_ptr<ID2D1SvgElement> const& element, 
    std::wstring const& attributeName)
{
    winrt::com_ptr<ID2D1SvgAttribute> attribute;
    winrt::check_hresult(element->GetAttributeValue(attributeName.c_str(), attribute.put()));
    if (auto paintAttribute = attribute.try_as<ID2D1SvgPaint>())
    {
        switch (paintAttribute->GetPaintType())
        {
        case D2D1_SVG_PAINT_TYPE_COLOR:
        {
            D2D1_COLOR_F color = {};
            paintAttribute->GetColor(&color);
            return std::make_shared<ColorBrushInfo>(D2DColorToWinRTColor(color));
        }
        case D2D1_SVG_PAINT_TYPE_URI:
        {
            uint32_t length = 0;
            winrt::check_hresult(paintAttribute->GetIdLength());
            std::wstring id(length + 1, 0);
            winrt::check_hresult(paintAttribute->GetId(id.data(), length));
            id.resize(length);
            winrt::com_ptr<ID2D1SvgDocument> document;
            element->GetDocument(document.put());
            return CreateBrushInfoFromId(document, id);
        }
        default:
            break;
        }
    }
    return nullptr;
}

std::shared_ptr<SvgShapesBuilder::IBrushInfo> SvgShapesBuilder::CreateBrushInfoFromId(
    winrt::com_ptr<ID2D1SvgDocument> const& document,
    std::wstring const& id)
{
    winrt::com_ptr<ID2D1SvgElement> reference;
    winrt::check_hresult(document->FindElementById(id.c_str(), reference.put()));
    auto tag = GetTag(reference);
    if (tag == L"linearGradient")
    {
        return CreateLinearGradientBrushInfo(reference);
    }
    return nullptr;
}

std::shared_ptr<SvgShapesBuilder::IBrushInfo> SvgShapesBuilder::CreateLinearGradientBrushInfo(
    winrt::com_ptr<ID2D1SvgElement> const& element)
{
    std::vector<GradientStopInfo> stops;
    if (element->HasChildren())
    {
        winrt::com_ptr<ID2D1SvgElement> child;
        element->GetFirstChild(child.put());
        while (child != nullptr)
        {
            auto current = child;
            if (!current->IsTextContent())
            {
                auto offset = GetFloatAttribute(current, L"offset");
                auto color = GetColorAttribute(current, L"stop-color");
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
