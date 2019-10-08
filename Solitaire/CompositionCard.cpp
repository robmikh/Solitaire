#include "pch.h"
#include "Card.h"
#include "ShapeCache.h"
#include "CompositionCard.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

const float2 CompositionCard::CardSize = { 175, 250 };

ShapeVisual BuildCardFront(
    std::shared_ptr<ShapeCache> const& shapeCache,
    hstring const& card,
    Color const& color)
{
    auto compositor = shapeCache->Compositor();
    auto shapeVisual = compositor.CreateShapeVisual();
    auto shapeContainer = compositor.CreateContainerShape();
    shapeVisual.Shapes().Append(shapeContainer);
    shapeVisual.Size(CompositionCard::CardSize);
    shapeVisual.BackfaceVisibility(CompositionBackfaceVisibility::Hidden);

    auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
    roundedRectGeometry.CornerRadius({ 10, 10 });
    roundedRectGeometry.Size(CompositionCard::CardSize);
    auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
    rectShape.StrokeBrush(compositor.CreateColorBrush(Colors::Gray()));
    rectShape.FillBrush(compositor.CreateColorBrush(Colors::White()));
    rectShape.StrokeThickness(2);
    shapeContainer.Shapes().Append(rectShape);

    auto pathGeometry = shapeCache->GetPathGeometry(card);
    auto pathShape = compositor.CreateSpriteShape(pathGeometry);
    pathShape.Offset({ 5, 0 });
    pathShape.FillBrush(compositor.CreateColorBrush(color));
    shapeContainer.Shapes().Append(pathShape);

    shapeVisual.Comment(card);

    return shapeVisual;
}

ShapeVisual BuildCardBack(std::shared_ptr<ShapeCache> const& shapeCache)
{
    auto compositor = shapeCache->Compositor();
    auto shapeVisual = compositor.CreateShapeVisual();
    shapeVisual.Shapes().Append(shapeCache->GetShape(ShapeType::Back));
    shapeVisual.Size(CompositionCard::CardSize);
    shapeVisual.BackfaceVisibility(CompositionBackfaceVisibility::Hidden);
    shapeVisual.RotationAxis({ 0, 1, 0 });
    shapeVisual.RotationAngleInDegrees(180);

    shapeVisual.Comment(L"Card Back");

    return shapeVisual;
}

CompositionCard::CompositionCard(
    Card card,
    std::shared_ptr<ShapeCache> const& shapeCache)
{
    auto compositor = shapeCache->Compositor();

    m_card = card;
    m_root = compositor.CreateContainerVisual();
    m_root.Size(CardSize);
    m_root.Comment(L"Card Root");

    m_sidesRoot = compositor.CreateContainerVisual();
    m_sidesRoot.RelativeSizeAdjustment({ 1, 1 });
    m_sidesRoot.RotationAxis({ 0, 1, 0 });
    m_sidesRoot.Comment(L"Card Sides");
    m_root.Children().InsertAtTop(m_sidesRoot);
    m_front = BuildCardFront(
        shapeCache,
        card.ToString(),
        card.IsRed() ? Colors::Crimson() : Colors::Black());
    m_sidesRoot.Children().InsertAtTop(m_front);
    m_back = BuildCardBack(shapeCache);
    m_sidesRoot.Children().InsertAtTop(m_back);
}

bool CompositionCard::HitTest(float2 point)
{
    float3 const offset = m_root.Offset();
    float2 const size = m_root.Size();

    if (point.x >= offset.x &&
        point.x < offset.x + size.x &&
        point.y >= offset.y &&
        point.y < offset.y + size.y)
    {
        return true;
    }

    return false;
}

void CompositionCard::IsFaceUp(bool isFaceUp)
{
    if (m_isFaceUp != isFaceUp)
    {
        m_isFaceUp = isFaceUp;
        auto rotation = m_isFaceUp ? 0 : 180;
        m_sidesRoot.RotationAngleInDegrees(rotation);
    }
}