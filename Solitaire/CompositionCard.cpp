#include "pch.h"
#include "Card.h"
#include "ShapeCache.h"
#include "CompositionCard.h"

namespace winrt
{
    using namespace Windows;
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Core;
    using namespace Windows::UI::Composition;
}

const winrt::float2 CompositionCard::CardSize = { 167, 243 };
const winrt::float2 CompositionCard::CornerRadius = { 9.5f, 9.5f };

winrt::ShapeVisual BuildCardFront(
    std::shared_ptr<ShapeCache> const& shapeCache,
    Card const& card,
    winrt::Color const& color)
{
    auto compositor = shapeCache->Compositor();
    auto shapeVisual = compositor.CreateShapeVisual();
    auto shapeContainer = compositor.CreateContainerShape();
    shapeVisual.Shapes().Append(shapeContainer);
    shapeVisual.Size(CompositionCard::CardSize);
    shapeVisual.BackfaceVisibility(winrt::CompositionBackfaceVisibility::Hidden);

    auto shapeInfo = shapeCache->GetCardFace(card);
    shapeContainer.Shapes().Append(shapeInfo.RootShape);
    shapeVisual.ViewBox(shapeInfo.ViewBox);

    shapeVisual.Comment(card.ToString());

    return shapeVisual;
}

winrt::ShapeVisual BuildCardBack(std::shared_ptr<ShapeCache> const& shapeCache)
{
    auto compositor = shapeCache->Compositor();
    auto shapeVisual = compositor.CreateShapeVisual();
    shapeVisual.Shapes().Append(shapeCache->GetShape(ShapeType::Back));
    shapeVisual.Size(CompositionCard::CardSize);
    shapeVisual.BackfaceVisibility(winrt::CompositionBackfaceVisibility::Hidden);
    shapeVisual.RotationAxis({ 0, 1, 0 });
    shapeVisual.RotationAngleInDegrees(180);
    shapeVisual.CenterPoint({ CompositionCard::CardSize.x / 2.0f, CompositionCard::CardSize.y / 2.0f, 0 });

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
    m_sidesRoot.CenterPoint({ CardSize.x / 2.0f, CardSize.y / 2.0f, 0 });
    m_sidesRoot.Comment(L"Card Sides");
    m_root.Children().InsertAtTop(m_sidesRoot);
    m_front = BuildCardFront(
        shapeCache,
        card,
        card.IsRed() ? winrt::Colors::Crimson() : winrt::Colors::Black());
    m_sidesRoot.Children().InsertAtTop(m_front);
    m_back = BuildCardBack(shapeCache);
    m_sidesRoot.Children().InsertAtTop(m_back);
}

bool CompositionCard::HitTest(winrt::float2 point)
{
    winrt::float3 const offset = m_root.Offset();
    winrt::float2 const size = m_root.Size();

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

void CompositionCard::AnimateIsFaceUp(bool isFaceUp, winrt::TimeSpan const& duration, winrt::TimeSpan const& delayTime)
{
    if (m_isFaceUp != isFaceUp)
    {
        m_isFaceUp = isFaceUp;
        auto rotation = m_sidesRoot.RotationAngleInDegrees() + 180;

        auto compositor = m_sidesRoot.Compositor();
        auto animation = compositor.CreateScalarKeyFrameAnimation();
        animation.InsertKeyFrame(1, rotation);
        animation.IterationBehavior(winrt::AnimationIterationBehavior::Count);
        animation.IterationCount(1);
        animation.Duration(duration);
        animation.DelayTime(delayTime);

        m_sidesRoot.StartAnimation(L"RotationAngleInDegrees", animation);
    }
}