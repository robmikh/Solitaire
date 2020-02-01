#include "pch.h"
#include "ShapeCache.h"
#include "Card.h"
#include "CompositionCard.h"

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Geometry.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Geometry;
using namespace Microsoft::Graphics::Canvas::Text;

CompositionPathGeometry GetPathForCard(CanvasDevice const& device, Compositor const& compositor, hstring const& card);

ShapeCache::ShapeCache(
    ::Compositor const& compositor)
{
    FillCache(compositor);
}

CompositionPathGeometry ShapeCache::GetPathGeometry(
    hstring const& key)
{
    return m_geometryCache.at(key);
}

CompositionShape ShapeCache::GetShape(ShapeType shapeType)
{
    return m_shapeCache.at(shapeType);
}

void ShapeCache::FillCache(::Compositor const& compositor)
{
    hstring faces[] = 
    {
        L"K",
        L"Q",
        L"J",
        L"10",
        L"9",
        L"8",
        L"7",
        L"6",
        L"5",
        L"4",
        L"3",
        L"2",
        L"A"
    };

    hstring suits[] =
    {
        L"♠",
        L"♣",
        L"♥",
        L"♦"
    };

    std::vector<hstring> cards;
    for (auto& face : faces)
    {
        for (auto& suit : suits)
        {
            auto card = face + suit;

            cards.push_back(card);
        }
    }

    auto device = CanvasDevice();
    m_geometryCache.clear();
    auto height = 47.8828125f;
    for (auto& card : cards)
    {
        m_geometryCache.emplace(card, GetPathForCard(device, compositor, card));
    }

    m_shapeCache.clear();
    {
        auto shapeContainer = compositor.CreateContainerShape();
        auto backgroundBaseColor = Colors::Blue();

        auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        roundedRectGeometry.CornerRadius({ 10, 10 });
        roundedRectGeometry.Size(CompositionCard::CardSize);
        auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
        rectShape.StrokeBrush(compositor.CreateColorBrush(Colors::Gray()));
        rectShape.FillBrush(compositor.CreateColorBrush(backgroundBaseColor));
        rectShape.StrokeThickness(2);
        shapeContainer.Shapes().Append(rectShape);

        float2 innerOffset{ 12, 12 };
        auto innerRoundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        innerRoundedRectGeometry.CornerRadius({ 6, 6 });
        innerRoundedRectGeometry.Size(CompositionCard::CardSize - innerOffset);
        auto innerRectShape = compositor.CreateSpriteShape(innerRoundedRectGeometry);
        innerRectShape.StrokeBrush(compositor.CreateColorBrush(Colors::White()));
        innerRectShape.StrokeThickness(5);
        innerRectShape.Offset(innerOffset / 2.0f);
        shapeContainer.Shapes().Append(innerRectShape);

        m_shapeCache.emplace(ShapeType::Back, shapeContainer);
    }

    {
        auto shapeContainer = compositor.CreateContainerShape();
        auto backgroundBaseColor = Colors::Blue();

        auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        roundedRectGeometry.CornerRadius({ 10, 10 });
        roundedRectGeometry.Size(CompositionCard::CardSize);
        auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
        rectShape.StrokeBrush(compositor.CreateColorBrush(Colors::Gray()));
        rectShape.StrokeThickness(5);
        shapeContainer.Shapes().Append(rectShape);

        float2 innerSize{ CompositionCard::CardSize / 2.0f };
        auto innerRoundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        innerRoundedRectGeometry.CornerRadius({ 6, 6 });
        innerRoundedRectGeometry.Size(innerSize);
        auto innerRectShape = compositor.CreateSpriteShape(innerRoundedRectGeometry);
        innerRectShape.FillBrush(compositor.CreateColorBrush(Colors::Gray()));
        innerRectShape.StrokeThickness(5);
        innerRectShape.Offset((CompositionCard::CardSize - innerSize) / 2.0f);
        shapeContainer.Shapes().Append(innerRectShape);

        m_shapeCache.emplace(ShapeType::Empty, shapeContainer);
    }

    m_compositor = compositor;
    m_textHeight = height;
}

// Built from https://github.com/microsoft/cascadia-code
CompositionPathGeometry GetPathForCard(CanvasDevice const& device, Compositor const& compositor, hstring const& card)
{
    auto pathBuilder = CanvasPathBuilder(device);
    if (card.size() == 3)
    {
        // 1
        pathBuilder.BeginFigure({ 0.000f, 24.442f });
        pathBuilder.AddLine({ 0.000f, 21.285f });
        pathBuilder.AddLine({ 6.009f, 21.285f });
        pathBuilder.AddLine({ 6.009f, 3.836f });
        pathBuilder.AddLine({ 0.339f, 5.805f });
        pathBuilder.AddLine({ 0.339f, 2.377f });
        pathBuilder.AddLine({ 6.009f, 0.340f });
        pathBuilder.AddLine({ 9.607f, 0.340f });
        pathBuilder.AddLine({ 9.607f, 21.285f });
        pathBuilder.AddLine({ 14.971f, 21.285f });
        pathBuilder.AddLine({ 14.971f, 24.442f });
        pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        // 0
        pathBuilder.BeginFigure({ 27.667f, 24.782f });
        pathBuilder.AddQuadraticBezier({ 19.452f, 24.782f }, { 19.452f, 12.561f });
        pathBuilder.AddQuadraticBezier({ 19.452f, 0.000f }, { 27.667f, 0.000f });
        pathBuilder.AddQuadraticBezier({ 35.882f, 0.000f }, { 35.882f, 12.561f });
        pathBuilder.AddQuadraticBezier({ 35.882f, 24.782f }, { 27.667f, 24.782f });
        pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        pathBuilder.BeginFigure({ 27.667f, 21.658f });
        pathBuilder.AddQuadraticBezier({ 32.250f, 21.658f }, { 32.250f, 12.561f });
        pathBuilder.AddQuadraticBezier({ 32.250f, 3.123f }, { 27.667f, 3.123f });
        pathBuilder.AddQuadraticBezier({ 23.084f, 3.123f }, { 23.084f, 12.561f });
        pathBuilder.AddQuadraticBezier({ 23.084f, 21.658f }, { 27.667f, 21.658f });
        pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        pathBuilder.BeginFigure({ 27.803f, 15.107f });
        pathBuilder.AddQuadraticBezier({ 26.767f, 15.107f }, { 26.020f, 14.377f });
        pathBuilder.AddQuadraticBezier({ 25.290f, 13.630f }, { 25.290f, 12.561f });
        pathBuilder.AddQuadraticBezier({ 25.290f, 11.508f }, { 26.020f, 10.778f });
        pathBuilder.AddQuadraticBezier({ 26.767f, 10.049f }, { 27.803f, 10.049f });
        pathBuilder.AddQuadraticBezier({ 28.872f, 10.049f }, { 29.602f, 10.778f });
        pathBuilder.AddQuadraticBezier({ 30.349f, 11.508f }, { 30.349f, 12.561f });
        pathBuilder.AddQuadraticBezier({ 30.349f, 13.630f }, { 29.602f, 14.377f });
        pathBuilder.AddQuadraticBezier({ 28.872f, 15.107f }, { 27.803f, 15.107f });
        pathBuilder.EndFigure(CanvasFigureLoop::Closed);

        auto suit = card[2];
        if (suit == L'♠')
        {
            // Spade (Far)
            pathBuilder.BeginFigure({ 42.590f, 21.655f });
            pathBuilder.AddQuadraticBezier({ 40.904f, 21.655f }, { 39.975f, 20.383f });
            pathBuilder.AddQuadraticBezier({ 39.046f, 19.110f }, { 39.046f, 16.805f });
            pathBuilder.AddQuadraticBezier({ 39.046f, 12.263f }, { 41.454f, 8.617f });
            pathBuilder.AddQuadraticBezier({ 43.880f, 4.970f }, { 48.180f, 2.424f });
            pathBuilder.AddQuadraticBezier({ 52.584f, 5.004f }, { 54.940f, 8.651f });
            pathBuilder.AddQuadraticBezier({ 57.314f, 12.298f }, { 57.314f, 16.805f });
            pathBuilder.AddQuadraticBezier({ 57.314f, 19.110f }, { 56.385f, 20.383f });
            pathBuilder.AddQuadraticBezier({ 55.456f, 21.655f }, { 53.771f, 21.655f });
            pathBuilder.AddQuadraticBezier({ 52.154f, 21.655f }, { 51.242f, 20.744f });
            pathBuilder.AddQuadraticBezier({ 50.348f, 19.832f }, { 50.348f, 18.198f });
            pathBuilder.AddLine({ 49.660f, 18.198f });
            pathBuilder.AddLine({ 49.660f, 23.410f });
            pathBuilder.AddLine({ 46.701f, 23.410f });
            pathBuilder.AddLine({ 46.701f, 18.198f });
            pathBuilder.AddLine({ 46.013f, 18.198f });
            pathBuilder.AddQuadraticBezier({ 46.013f, 19.832f }, { 45.084f, 20.744f });
            pathBuilder.AddQuadraticBezier({ 44.172f, 21.655f }, { 42.590f, 21.655f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        } 
        else if (suit == L'♣')
        {
            // Club (Far)
            pathBuilder.BeginFigure({ 42.733f, 21.417f });
            pathBuilder.AddQuadraticBezier({ 41.257f, 21.417f }, { 40.351f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 39.446f, 19.388f }, { 39.446f, 17.711f });
            pathBuilder.AddQuadraticBezier({ 39.446f, 15.950f }, { 40.385f, 14.893f });
            pathBuilder.AddQuadraticBezier({ 41.324f, 13.820f }, { 42.884f, 13.820f });
            pathBuilder.AddQuadraticBezier({ 44.159f, 13.820f }, { 45.031f, 14.591f });
            pathBuilder.AddLine({ 45.702f, 13.685f });
            pathBuilder.AddQuadraticBezier({ 44.008f, 13.115f }, { 43.035f, 11.874f });
            pathBuilder.AddQuadraticBezier({ 42.062f, 10.616f }, { 42.062f, 8.872f });
            pathBuilder.AddQuadraticBezier({ 42.062f, 6.071f }, { 43.790f, 4.377f });
            pathBuilder.AddQuadraticBezier({ 45.517f, 2.667f }, { 48.351f, 2.667f });
            pathBuilder.AddQuadraticBezier({ 51.203f, 2.667f }, { 52.913f, 4.377f });
            pathBuilder.AddQuadraticBezier({ 54.641f, 6.071f }, { 54.641f, 8.872f });
            pathBuilder.AddQuadraticBezier({ 54.641f, 10.583f }, { 53.702f, 11.824f });
            pathBuilder.AddQuadraticBezier({ 52.779f, 13.065f }, { 51.135f, 13.635f });
            pathBuilder.AddLine({ 51.790f, 14.490f });
            pathBuilder.AddQuadraticBezier({ 52.628f, 13.820f }, { 53.819f, 13.820f });
            pathBuilder.AddQuadraticBezier({ 55.379f, 13.820f }, { 56.318f, 14.893f });
            pathBuilder.AddQuadraticBezier({ 57.257f, 15.950f }, { 57.257f, 17.711f });
            pathBuilder.AddQuadraticBezier({ 57.257f, 19.388f }, { 56.351f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 55.463f, 21.417f }, { 53.970f, 21.417f });
            pathBuilder.AddQuadraticBezier({ 52.393f, 21.417f }, { 51.421f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 50.465f, 19.388f }, { 50.465f, 17.711f });
            pathBuilder.AddLine({ 49.794f, 17.711f });
            pathBuilder.AddLine({ 49.794f, 23.128f });
            pathBuilder.AddLine({ 46.909f, 23.128f });
            pathBuilder.AddLine({ 46.909f, 17.711f });
            pathBuilder.AddLine({ 46.238f, 17.711f });
            pathBuilder.AddQuadraticBezier({ 46.238f, 19.388f }, { 45.282f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 44.326f, 21.417f }, { 42.733f, 21.417f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (suit == L'♥')
        {
            // Heart (Far)
            pathBuilder.BeginFigure({ 47.913f, 23.265f });
            pathBuilder.AddQuadraticBezier({ 45.177f, 21.726f }, { 43.125f, 19.469f });
            pathBuilder.AddQuadraticBezier({ 41.090f, 17.195f }, { 39.962f, 14.528f });
            pathBuilder.AddQuadraticBezier({ 38.833f, 11.843f }, { 38.833f, 9.107f });
            pathBuilder.AddQuadraticBezier({ 38.833f, 6.235f }, { 40.081f, 4.491f });
            pathBuilder.AddQuadraticBezier({ 41.330f, 2.747f }, { 43.382f, 2.747f });
            pathBuilder.AddQuadraticBezier({ 45.108f, 2.747f }, { 46.322f, 4.115f });
            pathBuilder.AddQuadraticBezier({ 47.537f, 5.483f }, { 47.810f, 7.722f });
            pathBuilder.AddLine({ 48.015f, 7.722f });
            pathBuilder.AddQuadraticBezier({ 48.289f, 5.483f }, { 49.503f, 4.115f });
            pathBuilder.AddQuadraticBezier({ 50.717f, 2.747f }, { 52.444f, 2.747f });
            pathBuilder.AddQuadraticBezier({ 54.496f, 2.747f }, { 55.744f, 4.491f });
            pathBuilder.AddQuadraticBezier({ 56.992f, 6.235f }, { 56.992f, 9.107f });
            pathBuilder.AddQuadraticBezier({ 56.992f, 11.843f }, { 55.864f, 14.528f });
            pathBuilder.AddQuadraticBezier({ 54.735f, 17.212f }, { 52.683f, 19.486f });
            pathBuilder.AddQuadraticBezier({ 50.648f, 21.743f }, { 47.913f, 23.265f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (suit == L'♦')
        {
            // Diamond (Far)
            pathBuilder.BeginFigure({ 48.103f, 23.763f });
            pathBuilder.AddLine({ 39.701f, 13.070f });
            pathBuilder.AddLine({ 48.103f, 2.377f });
            pathBuilder.AddLine({ 56.369f, 13.070f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
    }
    else
    {
        if (card[0] == L'K')
        {
            // K
            pathBuilder.BeginFigure({ 0.000f, 24.442f });
            pathBuilder.AddLine({ 0.000f, 0.000f });
            pathBuilder.AddLine({ 3.443f, 0.000f });
            pathBuilder.AddLine({ 3.443f, 13.219f });
            pathBuilder.AddQuadraticBezier({ 5.835f, 12.996f }, { 7.574f, 11.825f });
            pathBuilder.AddQuadraticBezier({ 9.329f, 10.655f }, { 10.465f, 8.813f });
            pathBuilder.AddQuadraticBezier({ 11.601f, 6.954f }, { 12.152f, 4.682f });
            pathBuilder.AddQuadraticBezier({ 12.703f, 2.410f }, { 12.703f, 0.000f });
            pathBuilder.AddLine({ 16.249f, 0.000f });
            pathBuilder.AddQuadraticBezier({ 16.249f, 3.770f }, { 14.975f, 7.195f });
            pathBuilder.AddQuadraticBezier({ 13.718f, 10.620f }, { 11.326f, 12.978f });
            pathBuilder.AddLine({ 17.419f, 24.442f });
            pathBuilder.AddLine({ 13.254f, 24.442f });
            pathBuilder.AddLine({ 8.417f, 15.130f });
            pathBuilder.AddQuadraticBezier({ 7.315f, 15.732f }, { 6.059f, 16.111f });
            pathBuilder.AddQuadraticBezier({ 4.820f, 16.490f }, { 3.443f, 16.610f });
            pathBuilder.AddLine({ 3.443f, 24.442f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'Q')
        {
            // Q
            pathBuilder.BeginFigure({ 13.959f, 31.048f });
            pathBuilder.AddQuadraticBezier({ 6.865f, 31.048f }, { 6.865f, 23.856f });
            pathBuilder.AddLine({ 6.865f, 23.725f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 22.630f }, { 0.000f, 12.087f });
            pathBuilder.AddQuadraticBezier({ 0.000f, -0.008f }, { 8.565f, -0.008f });
            pathBuilder.AddQuadraticBezier({ 17.130f, -0.008f }, { 17.130f, 12.087f });
            pathBuilder.AddQuadraticBezier({ 17.130f, 22.745f }, { 10.101f, 23.758f });
            pathBuilder.AddLine({ 10.101f, 23.889f });
            pathBuilder.AddQuadraticBezier({ 10.101f, 28.106f }, { 13.959f, 28.106f });
            pathBuilder.AddLine({ 15.626f, 28.106f });
            pathBuilder.AddLine({ 15.299f, 31.048f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
            pathBuilder.BeginFigure({ 8.565f, 20.849f });
            pathBuilder.AddQuadraticBezier({ 13.697f, 20.849f }, { 13.697f, 12.087f });
            pathBuilder.AddQuadraticBezier({ 13.697f, 3.000f }, { 8.565f, 3.000f });
            pathBuilder.AddQuadraticBezier({ 3.432f, 3.000f }, { 3.432f, 12.087f });
            pathBuilder.AddQuadraticBezier({ 3.432f, 20.849f }, { 8.565f, 20.849f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'J')
        {
            // J
            pathBuilder.BeginFigure({ 8.259f, 24.373f });
            pathBuilder.AddQuadraticBezier({ 4.299f, 24.373f }, { 2.149f, 22.173f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 19.956f }, { 0.000f, 15.979f });
            pathBuilder.AddLine({ 3.385f, 15.979f });
            pathBuilder.AddQuadraticBezier({ 3.385f, 18.517f }, { 4.671f, 19.922f });
            pathBuilder.AddQuadraticBezier({ 5.974f, 21.327f }, { 8.343f, 21.327f });
            pathBuilder.AddQuadraticBezier({ 10.645f, 21.327f }, { 11.880f, 20.108f });
            pathBuilder.AddQuadraticBezier({ 13.133f, 18.890f }, { 13.133f, 16.656f });
            pathBuilder.AddLine({ 13.133f, 3.219f });
            pathBuilder.AddLine({ 6.837f, 3.219f });
            pathBuilder.AddLine({ 6.837f, 0.003f });
            pathBuilder.AddLine({ 16.517f, 0.003f });
            pathBuilder.AddLine({ 16.517f, 16.656f });
            pathBuilder.AddQuadraticBezier({ 16.517f, 20.328f }, { 14.334f, 22.359f });
            pathBuilder.AddQuadraticBezier({ 12.151f, 24.373f }, { 8.259f, 24.373f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'9')
        {
            // 9
            pathBuilder.BeginFigure({ 2.187f, 24.336f });
            pathBuilder.AddLine({ 1.851f, 21.138f });
            pathBuilder.AddQuadraticBezier({ 12.520f, 20.684f }, { 12.520f, 11.580f });
            pathBuilder.AddLine({ 12.217f, 11.580f });
            pathBuilder.AddQuadraticBezier({ 11.712f, 13.448f }, { 10.198f, 14.474f });
            pathBuilder.AddQuadraticBezier({ 8.700f, 15.484f }, { 6.529f, 15.484f });
            pathBuilder.AddQuadraticBezier({ 3.399f, 15.484f }, { 1.699f, 13.549f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 11.614f }, { 0.000f, 8.080f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 4.243f }, { 2.036f, 2.123f });
            pathBuilder.AddQuadraticBezier({ 4.089f, 0.002f }, { 7.842f, 0.002f });
            pathBuilder.AddQuadraticBezier({ 11.880f, 0.002f }, { 14.068f, 2.409f });
            pathBuilder.AddQuadraticBezier({ 16.256f, 4.798f }, { 16.256f, 9.190f });
            pathBuilder.AddLine({ 16.256f, 9.998f });
            pathBuilder.AddQuadraticBezier({ 16.256f, 16.830f }, { 12.772f, 20.415f });
            pathBuilder.AddQuadraticBezier({ 9.306f, 23.982f }, { 2.187f, 24.336f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
            pathBuilder.BeginFigure({ 8.044f, 12.388f });
            pathBuilder.AddQuadraticBezier({ 10.198f, 12.388f }, { 11.359f, 11.176f });
            pathBuilder.AddQuadraticBezier({ 12.520f, 9.965f }, { 12.520f, 7.743f });
            pathBuilder.AddQuadraticBezier({ 12.520f, 3.099f }, { 7.808f, 3.099f });
            pathBuilder.AddQuadraticBezier({ 5.789f, 3.099f }, { 4.695f, 4.411f });
            pathBuilder.AddQuadraticBezier({ 3.601f, 5.707f }, { 3.601f, 8.080f });
            pathBuilder.AddQuadraticBezier({ 3.601f, 10.150f }, { 4.745f, 11.277f });
            pathBuilder.AddQuadraticBezier({ 5.906f, 12.388f }, { 8.044f, 12.388f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'8')
        {
            // 8
            pathBuilder.BeginFigure({ 8.117f, 24.469f });
            pathBuilder.AddQuadraticBezier({ 4.243f, 24.469f }, { 2.113f, 22.675f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 20.864f }, { 0.000f, 17.593f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 15.195f }, { 1.375f, 13.652f });
            pathBuilder.AddQuadraticBezier({ 2.767f, 12.109f }, { 5.333f, 11.589f });
            pathBuilder.AddLine({ 5.333f, 11.338f });
            pathBuilder.AddQuadraticBezier({ 3.086f, 10.834f }, { 1.878f, 9.476f });
            pathBuilder.AddQuadraticBezier({ 0.671f, 8.117f }, { 0.671f, 6.021f });
            pathBuilder.AddQuadraticBezier({ 0.671f, 3.153f }, { 2.616f, 1.577f });
            pathBuilder.AddQuadraticBezier({ 4.579f, -0.017f }, { 8.117f, -0.017f });
            pathBuilder.AddQuadraticBezier({ 11.673f, -0.017f }, { 13.618f, 1.577f });
            pathBuilder.AddQuadraticBezier({ 15.564f, 3.153f }, { 15.564f, 6.021f });
            pathBuilder.AddQuadraticBezier({ 15.564f, 8.101f }, { 14.356f, 9.459f });
            pathBuilder.AddQuadraticBezier({ 13.166f, 10.818f }, { 10.935f, 11.338f });
            pathBuilder.AddLine({ 10.935f, 11.589f });
            pathBuilder.AddQuadraticBezier({ 13.484f, 12.126f }, { 14.859f, 13.669f });
            pathBuilder.AddQuadraticBezier({ 16.235f, 15.195f }, { 16.235f, 17.593f });
            pathBuilder.AddQuadraticBezier({ 16.235f, 20.864f }, { 14.105f, 22.675f });
            pathBuilder.AddQuadraticBezier({ 11.992f, 24.469f }, { 8.117f, 24.469f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
            pathBuilder.BeginFigure({ 8.117f, 9.945f });
            pathBuilder.AddQuadraticBezier({ 9.962f, 9.945f }, { 10.969f, 9.090f });
            pathBuilder.AddQuadraticBezier({ 11.975f, 8.218f }, { 11.975f, 6.692f });
            pathBuilder.AddQuadraticBezier({ 11.975f, 4.948f }, { 10.969f, 4.008f });
            pathBuilder.AddQuadraticBezier({ 9.962f, 3.069f }, { 8.117f, 3.069f });
            pathBuilder.AddQuadraticBezier({ 6.289f, 3.069f }, { 5.266f, 4.008f });
            pathBuilder.AddQuadraticBezier({ 4.260f, 4.948f }, { 4.260f, 6.692f });
            pathBuilder.AddQuadraticBezier({ 4.260f, 8.218f }, { 5.266f, 9.090f });
            pathBuilder.AddQuadraticBezier({ 6.289f, 9.945f }, { 8.117f, 9.945f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
            pathBuilder.BeginFigure({ 8.117f, 21.384f });
            pathBuilder.AddQuadraticBezier({ 10.281f, 21.384f }, { 11.455f, 20.344f });
            pathBuilder.AddQuadraticBezier({ 12.646f, 19.304f }, { 12.646f, 17.425f });
            pathBuilder.AddQuadraticBezier({ 12.646f, 15.413f }, { 11.455f, 14.289f });
            pathBuilder.AddQuadraticBezier({ 10.281f, 13.166f }, { 8.117f, 13.166f });
            pathBuilder.AddQuadraticBezier({ 5.954f, 13.166f }, { 4.763f, 14.289f });
            pathBuilder.AddQuadraticBezier({ 3.589f, 15.413f }, { 3.589f, 17.425f });
            pathBuilder.AddQuadraticBezier({ 3.589f, 19.304f }, { 4.763f, 20.344f });
            pathBuilder.AddQuadraticBezier({ 5.954f, 21.384f }, { 8.117f, 21.384f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'7')
        {
            // 7
            pathBuilder.BeginFigure({ 5.728f, 24.291f });
            pathBuilder.AddLine({ 12.961f, 3.260f });
            pathBuilder.AddLine({ 3.522f, 3.260f });
            pathBuilder.AddLine({ 3.522f, 8.526f });
            pathBuilder.AddLine({ 0.000f, 8.526f });
            pathBuilder.AddLine({ 0.000f, 0.011f });
            pathBuilder.AddLine({ 17.817f, 0.011f });
            pathBuilder.AddLine({ 9.473f, 24.291f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'6')
        {
            // 6
            pathBuilder.BeginFigure({ 8.465f, 24.460f });
            pathBuilder.AddQuadraticBezier({ 4.402f, 24.460f }, { 2.201f, 22.056f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 19.635f }, { 0.000f, 15.217f });
            pathBuilder.AddLine({ 0.000f, 14.404f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 7.514f }, { 3.487f, 3.925f });
            pathBuilder.AddQuadraticBezier({ 6.992f, 0.336f }, { 14.153f, -0.019f });
            pathBuilder.AddLine({ 14.491f, 3.197f });
            pathBuilder.AddQuadraticBezier({ 3.758f, 3.654f }, { 3.758f, 12.813f });
            pathBuilder.AddLine({ 4.063f, 12.813f });
            pathBuilder.AddQuadraticBezier({ 4.571f, 10.934f }, { 6.078f, 9.918f });
            pathBuilder.AddQuadraticBezier({ 7.601f, 8.885f }, { 9.785f, 8.885f });
            pathBuilder.AddQuadraticBezier({ 12.934f, 8.885f }, { 14.644f, 10.832f });
            pathBuilder.AddQuadraticBezier({ 16.354f, 12.779f }, { 16.354f, 16.334f });
            pathBuilder.AddQuadraticBezier({ 16.354f, 20.194f }, { 14.305f, 22.327f });
            pathBuilder.AddQuadraticBezier({ 12.257f, 24.460f }, { 8.465f, 24.460f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
            pathBuilder.BeginFigure({ 8.498f, 21.345f });
            pathBuilder.AddQuadraticBezier({ 10.479f, 21.345f }, { 11.546f, 20.042f });
            pathBuilder.AddQuadraticBezier({ 12.629f, 18.721f }, { 12.629f, 16.334f });
            pathBuilder.AddQuadraticBezier({ 12.629f, 14.252f }, { 11.495f, 13.134f });
            pathBuilder.AddQuadraticBezier({ 10.378f, 12.000f }, { 8.261f, 12.000f });
            pathBuilder.AddQuadraticBezier({ 6.094f, 12.000f }, { 4.909f, 13.219f });
            pathBuilder.AddQuadraticBezier({ 3.724f, 14.438f }, { 3.724f, 16.673f });
            pathBuilder.AddQuadraticBezier({ 3.724f, 21.345f }, { 8.498f, 21.345f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'5')
        {
            // 5
            pathBuilder.BeginFigure({ 7.834f, 24.432f });
            pathBuilder.AddQuadraticBezier({ 4.103f, 24.432f }, { 2.052f, 22.584f });
            pathBuilder.AddQuadraticBezier({ 0.000f, 20.719f }, { 0.000f, 17.345f });
            pathBuilder.AddLine({ 3.629f, 17.345f });
            pathBuilder.AddQuadraticBezier({ 3.629f, 19.227f }, { 4.731f, 20.278f });
            pathBuilder.AddQuadraticBezier({ 5.833f, 21.312f }, { 7.867f, 21.312f });
            pathBuilder.AddQuadraticBezier({ 10.190f, 21.312f }, { 11.428f, 20.007f });
            pathBuilder.AddQuadraticBezier({ 12.683f, 18.684f }, { 12.683f, 16.327f });
            pathBuilder.AddQuadraticBezier({ 12.683f, 11.444f }, { 7.630f, 11.444f });
            pathBuilder.AddQuadraticBezier({ 5.341f, 11.444f }, { 3.357f, 12.563f });
            pathBuilder.AddLine({ 1.458f, 12.563f });
            pathBuilder.AddLine({ 1.797f, 0.016f });
            pathBuilder.AddLine({ 14.785f, 0.016f });
            pathBuilder.AddLine({ 14.785f, 3.170f });
            pathBuilder.AddLine({ 5.188f, 3.170f });
            pathBuilder.AddLine({ 4.849f, 9.087f });
            pathBuilder.AddQuadraticBezier({ 5.850f, 8.680f }, { 6.833f, 8.545f });
            pathBuilder.AddQuadraticBezier({ 7.817f, 8.392f }, { 8.817f, 8.392f });
            pathBuilder.AddQuadraticBezier({ 12.581f, 8.392f }, { 14.446f, 10.410f });
            pathBuilder.AddQuadraticBezier({ 16.311f, 12.428f }, { 16.311f, 16.328f });
            pathBuilder.AddQuadraticBezier({ 16.311f, 20.193f }, { 14.107f, 22.313f });
            pathBuilder.AddQuadraticBezier({ 11.903f, 24.432f }, { 7.834f, 24.432f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'4')
        {
            // 4
            pathBuilder.BeginFigure({ 0.000f, 18.965f });
            pathBuilder.AddLine({ 0.052f, 18.914f });
            pathBuilder.AddLine({ 3.473f, 1.392f });
            pathBuilder.AddLine({ 7.016f, 1.392f });
            pathBuilder.AddLine({ 4.144f, 15.939f });
            pathBuilder.AddLine({ 12.243f, 15.939f });
            pathBuilder.AddLine({ 12.243f, 0.016f });
            pathBuilder.AddLine({ 15.716f, 0.016f });
            pathBuilder.AddLine({ 15.716f, 15.939f });
            pathBuilder.AddLine({ 18.020f, 15.939f });
            pathBuilder.AddLine({ 18.020f, 18.965f });
            pathBuilder.AddLine({ 15.716f, 18.965f });
            pathBuilder.AddLine({ 15.716f, 24.433f });
            pathBuilder.AddLine({ 12.243f, 24.433f });
            pathBuilder.AddLine({ 12.243f, 18.965f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'3')
        {
            // 3
            pathBuilder.BeginFigure({ 6.567f, 24.648f });
            pathBuilder.AddQuadraticBezier({ 4.710f, 24.648f }, { 3.157f, 24.429f });
            pathBuilder.AddQuadraticBezier({ 1.621f, 24.209f }, { 0.422f, 23.838f });
            pathBuilder.AddLine({ 0.844f, 20.613f });
            pathBuilder.AddQuadraticBezier({ 1.874f, 20.901f }, { 3.157f, 21.221f });
            pathBuilder.AddQuadraticBezier({ 4.457f, 21.542f }, { 6.432f, 21.542f });
            pathBuilder.AddQuadraticBezier({ 9.032f, 21.542f }, { 10.467f, 20.495f });
            pathBuilder.AddQuadraticBezier({ 11.902f, 19.449f }, { 11.902f, 17.558f });
            pathBuilder.AddQuadraticBezier({ 11.902f, 15.532f }, { 10.062f, 14.435f });
            pathBuilder.AddQuadraticBezier({ 8.222f, 13.337f }, { 4.845f, 13.337f });
            pathBuilder.AddLine({ 4.305f, 13.337f });
            pathBuilder.AddLine({ 3.900f, 11.143f });
            pathBuilder.AddQuadraticBezier({ 7.850f, 10.602f }, { 9.623f, 9.336f });
            pathBuilder.AddQuadraticBezier({ 11.396f, 8.053f }, { 11.396f, 6.213f });
            pathBuilder.AddQuadraticBezier({ 11.396f, 4.744f }, { 10.450f, 3.934f });
            pathBuilder.AddQuadraticBezier({ 9.522f, 3.107f }, { 7.783f, 3.107f });
            pathBuilder.AddQuadraticBezier({ 6.500f, 3.107f }, { 5.048f, 3.748f });
            pathBuilder.AddQuadraticBezier({ 3.613f, 4.390f }, { 2.330f, 5.673f });
            pathBuilder.AddLine({ 0.000f, 3.360f });
            pathBuilder.AddQuadraticBezier({ 1.553f, 1.604f }, { 3.596f, 0.811f });
            pathBuilder.AddQuadraticBezier({ 5.639f, 0.000f }, { 8.154f, 0.000f });
            pathBuilder.AddQuadraticBezier({ 11.429f, 0.000f }, { 13.219f, 1.401f });
            pathBuilder.AddQuadraticBezier({ 15.008f, 2.786f }, { 15.008f, 5.369f });
            pathBuilder.AddQuadraticBezier({ 15.008f, 7.530f }, { 13.624f, 9.133f });
            pathBuilder.AddQuadraticBezier({ 12.240f, 10.720f }, { 10.011f, 11.311f });
            pathBuilder.AddLine({ 10.011f, 11.666f });
            pathBuilder.AddQuadraticBezier({ 12.493f, 12.105f }, { 13.995f, 13.708f });
            pathBuilder.AddQuadraticBezier({ 15.515f, 15.295f }, { 15.515f, 17.895f });
            pathBuilder.AddQuadraticBezier({ 15.515f, 21.103f }, { 13.168f, 22.876f });
            pathBuilder.AddQuadraticBezier({ 10.838f, 24.648f }, { 6.567f, 24.648f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'2')
        {
            // 2
            pathBuilder.BeginFigure({ 0.782f, 24.500f });
            pathBuilder.AddLine({ 0.782f, 20.690f });
            pathBuilder.AddQuadraticBezier({ 6.378f, 17.595f }, { 9.133f, 14.228f });
            pathBuilder.AddQuadraticBezier({ 11.905f, 10.861f }, { 11.905f, 7.289f });
            pathBuilder.AddQuadraticBezier({ 11.905f, 3.139f }, { 7.789f, 3.139f });
            pathBuilder.AddQuadraticBezier({ 6.412f, 3.139f }, { 4.915f, 3.939f });
            pathBuilder.AddQuadraticBezier({ 3.418f, 4.738f }, { 2.347f, 6.065f });
            pathBuilder.AddLine({ 0.000f, 3.735f });
            pathBuilder.AddQuadraticBezier({ 1.173f, 2.051f }, { 3.333f, 1.031f });
            pathBuilder.AddQuadraticBezier({ 5.493f, 0.010f }, { 7.857f, 0.010f });
            pathBuilder.AddQuadraticBezier({ 15.476f, 0.010f }, { 15.476f, 7.289f });
            pathBuilder.AddQuadraticBezier({ 15.476f, 10.963f }, { 13.146f, 14.347f });
            pathBuilder.AddQuadraticBezier({ 10.833f, 17.714f }, { 6.088f, 20.827f });
            pathBuilder.AddLine({ 6.088f, 21.337f });
            pathBuilder.AddLine({ 16.497f, 21.337f });
            pathBuilder.AddLine({ 16.497f, 24.500f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (card[0] == L'A')
        {
            // A
            pathBuilder.BeginFigure({ 0.000f, 24.442f });
            pathBuilder.AddLine({ 7.053f, 0.016f });
            pathBuilder.AddLine({ 12.041f, 0.016f });
            pathBuilder.AddLine({ 19.094f, 24.442f });
            pathBuilder.AddLine({ 15.481f, 24.442f });
            pathBuilder.AddLine({ 13.882f, 18.490f });
            pathBuilder.AddLine({ 5.212f, 18.490f });
            pathBuilder.AddLine({ 3.612f, 24.442f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
            pathBuilder.BeginFigure({ 9.392f, 2.975f });
            pathBuilder.AddLine({ 6.089f, 15.222f });
            pathBuilder.AddLine({ 13.004f, 15.222f });
            pathBuilder.AddLine({ 9.702f, 2.975f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }

        auto suit = card[1];
        if (suit == L'♠')
        {
            // Spade
            pathBuilder.BeginFigure({ 22.635f, 21.654f });
            pathBuilder.AddQuadraticBezier({ 20.948f, 21.654f }, { 20.018f, 20.380f });
            pathBuilder.AddQuadraticBezier({ 19.089f, 19.106f }, { 19.089f, 16.800f });
            pathBuilder.AddQuadraticBezier({ 19.089f, 12.256f }, { 21.499f, 8.606f });
            pathBuilder.AddQuadraticBezier({ 23.926f, 4.957f }, { 28.229f, 2.410f });
            pathBuilder.AddQuadraticBezier({ 32.635f, 4.992f }, { 34.993f, 8.641f });
            pathBuilder.AddQuadraticBezier({ 37.369f, 12.290f }, { 37.369f, 16.800f });
            pathBuilder.AddQuadraticBezier({ 37.369f, 19.106f }, { 36.439f, 20.380f });
            pathBuilder.AddQuadraticBezier({ 35.510f, 21.654f }, { 33.823f, 21.654f });
            pathBuilder.AddQuadraticBezier({ 32.205f, 21.654f }, { 31.293f, 20.741f });
            pathBuilder.AddQuadraticBezier({ 30.397f, 19.829f }, { 30.397f, 18.194f });
            pathBuilder.AddLine({ 29.709f, 18.194f });
            pathBuilder.AddLine({ 29.709f, 23.409f });
            pathBuilder.AddLine({ 26.748f, 23.409f });
            pathBuilder.AddLine({ 26.748f, 18.194f });
            pathBuilder.AddLine({ 26.060f, 18.194f });
            pathBuilder.AddQuadraticBezier({ 26.060f, 19.829f }, { 25.130f, 20.741f });
            pathBuilder.AddQuadraticBezier({ 24.218f, 21.654f }, { 22.635f, 21.654f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (suit == L'♣')
        {
            // Club
            pathBuilder.BeginFigure({ 22.625f, 21.417f });
            pathBuilder.AddQuadraticBezier({ 21.149f, 21.417f }, { 20.243f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 19.337f, 19.388f }, { 19.337f, 17.711f });
            pathBuilder.AddQuadraticBezier({ 19.337f, 15.950f }, { 20.277f, 14.893f });
            pathBuilder.AddQuadraticBezier({ 21.216f, 13.820f }, { 22.776f, 13.820f });
            pathBuilder.AddQuadraticBezier({ 24.050f, 13.820f }, { 24.922f, 14.591f });
            pathBuilder.AddLine({ 25.593f, 13.685f });
            pathBuilder.AddQuadraticBezier({ 23.899f, 13.115f }, { 22.927f, 11.874f });
            pathBuilder.AddQuadraticBezier({ 21.954f, 10.616f }, { 21.954f, 8.872f });
            pathBuilder.AddQuadraticBezier({ 21.954f, 6.071f }, { 23.681f, 4.377f });
            pathBuilder.AddQuadraticBezier({ 25.409f, 2.667f }, { 28.243f, 2.667f });
            pathBuilder.AddQuadraticBezier({ 31.094f, 2.667f }, { 32.805f, 4.377f });
            pathBuilder.AddQuadraticBezier({ 34.532f, 6.071f }, { 34.532f, 8.872f });
            pathBuilder.AddQuadraticBezier({ 34.532f, 10.583f }, { 33.593f, 11.824f });
            pathBuilder.AddQuadraticBezier({ 32.671f, 13.065f }, { 31.027f, 13.635f });
            pathBuilder.AddLine({ 31.681f, 14.490f });
            pathBuilder.AddQuadraticBezier({ 32.520f, 13.820f }, { 33.711f, 13.820f });
            pathBuilder.AddQuadraticBezier({ 35.270f, 13.820f }, { 36.210f, 14.893f });
            pathBuilder.AddQuadraticBezier({ 37.149f, 15.950f }, { 37.149f, 17.711f });
            pathBuilder.AddQuadraticBezier({ 37.149f, 19.388f }, { 36.243f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 35.354f, 21.417f }, { 33.862f, 21.417f });
            pathBuilder.AddQuadraticBezier({ 32.285f, 21.417f }, { 31.312f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 30.356f, 19.388f }, { 30.356f, 17.711f });
            pathBuilder.AddLine({ 29.685f, 17.711f });
            pathBuilder.AddLine({ 29.685f, 23.128f });
            pathBuilder.AddLine({ 26.801f, 23.128f });
            pathBuilder.AddLine({ 26.801f, 17.711f });
            pathBuilder.AddLine({ 26.130f, 17.711f });
            pathBuilder.AddQuadraticBezier({ 26.130f, 19.388f }, { 25.174f, 20.411f });
            pathBuilder.AddQuadraticBezier({ 24.218f, 21.417f }, { 22.625f, 21.417f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (suit == L'♥')
        {
            // Heart
            pathBuilder.BeginFigure({ 28.837f, 23.042f });
            pathBuilder.AddQuadraticBezier({ 26.130f, 21.519f }, { 24.099f, 19.285f });
            pathBuilder.AddQuadraticBezier({ 22.085f, 17.034f }, { 20.968f, 14.394f });
            pathBuilder.AddQuadraticBezier({ 19.851f, 11.737f }, { 19.851f, 9.029f });
            pathBuilder.AddQuadraticBezier({ 19.851f, 6.186f }, { 21.086f, 4.460f });
            pathBuilder.AddQuadraticBezier({ 22.322f, 2.734f }, { 24.353f, 2.734f });
            pathBuilder.AddQuadraticBezier({ 26.062f, 2.734f }, { 27.263f, 4.088f });
            pathBuilder.AddQuadraticBezier({ 28.465f, 5.441f }, { 28.736f, 7.658f });
            pathBuilder.AddLine({ 28.939f, 7.658f });
            pathBuilder.AddQuadraticBezier({ 29.210f, 5.441f }, { 30.411f, 4.088f });
            pathBuilder.AddQuadraticBezier({ 31.613f, 2.734f }, { 33.322f, 2.734f });
            pathBuilder.AddQuadraticBezier({ 35.353f, 2.734f }, { 36.588f, 4.460f });
            pathBuilder.AddQuadraticBezier({ 37.824f, 6.186f }, { 37.824f, 9.029f });
            pathBuilder.AddQuadraticBezier({ 37.824f, 11.737f }, { 36.707f, 14.394f });
            pathBuilder.AddQuadraticBezier({ 35.590f, 17.051f }, { 33.559f, 19.302f });
            pathBuilder.AddQuadraticBezier({ 31.545f, 21.536f }, { 28.837f, 23.042f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
        else if (suit == L'♦')
        {
            // Diamond
            pathBuilder.BeginFigure({ 28.542f, 23.444f });
            pathBuilder.AddLine({ 20.163f, 12.779f });
            pathBuilder.AddLine({ 28.542f, 2.114f });
            pathBuilder.AddLine({ 36.787f, 12.779f });
            pathBuilder.EndFigure(CanvasFigureLoop::Closed);
        }
    }
    auto canvasGeometry = CanvasGeometry::CreatePath(pathBuilder);
    auto compositionPath = CompositionPath(canvasGeometry);
    auto pathGeometry = compositor.CreatePathGeometry(compositionPath);
    return pathGeometry;
}