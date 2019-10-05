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

const float2 CardSize = { 175, 250 };

ShapeVisual BuildCardFront(
    std::shared_ptr<ShapeCache> const& shapeCache,
    hstring const& card,
    Color const& color)
{
    auto compositor = shapeCache->Compositor();
    auto shapeVisual = compositor.CreateShapeVisual();
    auto shapeContainer = compositor.CreateContainerShape();
    shapeVisual.Shapes().Append(shapeContainer);
    shapeVisual.Size(CardSize);
    shapeVisual.BackfaceVisibility(CompositionBackfaceVisibility::Hidden);

    auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
    roundedRectGeometry.CornerRadius({ 10, 10 });
    roundedRectGeometry.Size(CardSize);
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

    return shapeVisual;
}

ShapeVisual BuildCardBack(Compositor const& compositor, Color const& color)
{
    auto shapeVisual = compositor.CreateShapeVisual();
    auto shapeContainer = compositor.CreateContainerShape();
    shapeVisual.Shapes().Append(shapeContainer);
    shapeVisual.Size(CardSize);
    shapeVisual.BackfaceVisibility(CompositionBackfaceVisibility::Hidden);
    shapeVisual.RotationAxis({ 0, 1, 0 });
    shapeVisual.RotationAngleInDegrees(180);

    auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
    roundedRectGeometry.CornerRadius({ 10, 10 });
    roundedRectGeometry.Size(CardSize);
    auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
    rectShape.StrokeBrush(compositor.CreateColorBrush(Colors::Gray()));
    rectShape.FillBrush(compositor.CreateColorBrush(color));
    rectShape.StrokeThickness(2);
    shapeContainer.Shapes().Append(rectShape);

    float2 innerOffset{ 12, 12 };
    auto innerRoundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
    innerRoundedRectGeometry.CornerRadius({ 6, 6 });
    innerRoundedRectGeometry.Size({ CardSize.x - innerOffset.x, CardSize.y - innerOffset.y });
    auto innerRectShape = compositor.CreateSpriteShape(innerRoundedRectGeometry);
    innerRectShape.StrokeBrush(compositor.CreateColorBrush(Colors::White()));
    innerRectShape.FillBrush(compositor.CreateColorBrush(color));
    innerRectShape.StrokeThickness(5);
    innerRectShape.Offset(innerOffset / 2.0f);
    shapeContainer.Shapes().Append(innerRectShape);

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

    m_sidesRoot = compositor.CreateContainerVisual();
    m_sidesRoot.RelativeSizeAdjustment({ 1, 1 });
    m_sidesRoot.RotationAxis({ 0, 1, 0 });
    m_root.Children().InsertAtTop(m_sidesRoot);
    m_front = BuildCardFront(
        shapeCache,
        card.ToString(),
        card.IsRed() ? Colors::Crimson() : Colors::Black());
    m_sidesRoot.Children().InsertAtTop(m_front);
    m_back = BuildCardBack(compositor, Colors::Blue());
    m_sidesRoot.Children().InsertAtTop(m_back);

    m_chainedCards = compositor.CreateContainerVisual();
    m_chainedCards.RelativeSizeAdjustment({ 1, 1 });
    m_root.Children().InsertAtTop(m_chainedCards);
}

bool CompositionCard::HitTest(float2 accumulatedOffset, float2 point)
{
    auto offset = m_root.Offset();
    offset.x += accumulatedOffset.x;
    offset.y += accumulatedOffset.y;
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

void CompositionCard::ChainCard(CompositionCard const& card)
{
    m_chainedCards.Children().InsertAtTop(card.m_root);
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