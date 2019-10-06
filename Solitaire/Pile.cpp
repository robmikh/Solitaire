#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "Pile.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI::Composition;
}

winrt::ShapeVisual CreateBaseVisual(std::shared_ptr<ShapeCache> const& shapeCache)
{
    auto compositor = shapeCache->Compositor();
    auto visual = compositor.CreateShapeVisual();
    visual.Shapes().Append(shapeCache->GetShape(ShapeType::Empty));
    visual.Size(CompositionCard::CardSize);
    return visual;
}

Pile::Pile(std::shared_ptr<ShapeCache> const& shapeCache)
{
    m_background = CreateBaseVisual(shapeCache);
    m_children = m_background.Children();
}

Pile::Pile(std::shared_ptr<ShapeCache> const& shapeCache, Pile::CardList cards)
{
    m_background = CreateBaseVisual(shapeCache);
    m_children = m_background.Children();
    m_cards = cards;

    
}

void Pile::ForceLayout()
{
    auto parentChildren = m_children;
    auto index = 0;
    for (auto& card : m_cards)
    {
        auto visual = card->Root();
        auto offset = ComputeOffset(index);
        visual.Offset(offset);
        parentChildren.InsertAtTop(visual);
        parentChildren = card->Children();
        index++;
    }
}

// The coordinates provides are assumed to be in "base space" (the local space for the base visual)
Pile::HitTestResult Pile::HitTest(winrt::float2 point)
{
    Pile::HitTestResult result;

    for (int i = m_cards.size() - 1; i >= 0; i--)
    {
        auto card = m_cards[i];

        winrt::float3 accumulatedOffset = { 0, 0, 0 };
        if (i > 0)
        {
            accumulatedOffset = ComputeBaseSpaceOffset(i - 1);
        }

        auto tempPoint = point;
        tempPoint.x -= accumulatedOffset.x;
        tempPoint.y -= accumulatedOffset.y;
        if (card->HitTest(tempPoint))
        {
            result.Target = Pile::HitTestTarget::Card;
            result.CardIndex = i;
            return result;
        }
    }

    winrt::float2 const size = m_background.Size();
    if (point.x >= 0 &&
        point.x < size.x &&
        point.y >= 0 &&
        point.y < size.y)
    {
        result.Target = Pile::HitTestTarget::Base;
        return result;
    }

    WINRT_ASSERT(result.Target == Pile::HitTestTarget::None);
    WINRT_ASSERT(result.CardIndex < 0);
    return result;
}

Pile::CardList Pile::Split(int index)
{
    WINRT_ASSERT(CanSplit(index));

    auto start = m_cards.begin() + index;
    auto end = m_cards.end();
    Pile::CardList result(
        std::make_move_iterator(start),
        std::make_move_iterator(end));
    m_cards.erase(start, end);

    auto firstCardSplit = result.front();
    auto visualToRemove = firstCardSplit->Root();
    auto offset = visualToRemove.Offset();

    auto indexOfLastCard = (int)m_cards.size() - 1;
    if (indexOfLastCard >= 0)
    {
        offset += ComputeBaseSpaceOffset(indexOfLastCard);
        m_cards[indexOfLastCard]->Children().Remove(visualToRemove);
    }
    else
    {
        m_children.Remove(visualToRemove);
    }
    offset += m_background.Offset();
    visualToRemove.Offset({ offset });

    return result;
}

void Pile::Add(Pile::CardList const& cards)
{
    //WINRT_ASSERT(CanAdd(cards.front()));
    if (cards.empty())
    {
        return;
    }

    auto parentChildren = m_cards.empty() ? m_children : m_cards.back()->Children();
    for (auto& card : cards)
    {
        auto index = m_cards.size();
        auto visual = card->Root();
        visual.Offset(ComputeOffset(index));
        if (!visual.Parent())
        {
            parentChildren.InsertAtTop(visual);
        }
        // TODO: Assert the parent is correct
        parentChildren = card->Children();
        m_cards.push_back(card);
    }
}