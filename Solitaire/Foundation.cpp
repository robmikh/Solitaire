#include"pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "Foundation.h"

using namespace winrt;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI::Composition;

Foundation::Foundation(std::shared_ptr<ShapeCache> const& shapeCache)
{
    auto compositor = shapeCache->Compositor();
    m_background = compositor.CreateShapeVisual();
    m_background.Shapes().Append(shapeCache->GetShape(ShapeType::Empty));
    m_background.Size(CompositionCard::CardSize);
}

void Foundation::ForceLayout()
{
    if (m_cards.empty())
    {
        return;
    }

    m_background.Children().RemoveAll();
    for (auto& card : m_cards)
    {
        auto visual = card->Root();
        visual.Offset({ 0, 0, 0 });

        if (visual.Parent())
        {
            visual.Parent().Children().Remove(visual);
        }

        card->IsFaceUp(true);
        m_background.Children().InsertAtTop(visual);
    }
}

bool Foundation::HitTest(float2 point)
{
    point.x -= m_background.Offset().x;
    point.y -= m_background.Offset().y;
    for (int i = m_cards.size() - 1; i >= 0; i--)
    {
        auto card = m_cards[i];
        if (card->HitTest({ 0, 0 }, point))
        {
            return true;
        }
    }

    float2 const size = m_background.Size();
    if (point.x >= 0 &&
        point.x < size.x &&
        point.y >= 0 &&
        point.y < size.y)
    {
        return true;
    }

    return false;
}

bool Foundation::CanTake()
{
    return !m_cards.empty();
}

std::shared_ptr<CompositionCard> Foundation::Take()
{
    if (!CanTake())
    {
        return nullptr;
    }

    auto it = m_cards.begin() + (m_cards.size() - 1);
    auto card = *it;
    m_cards.erase(it);
    auto visual = card->Root();
    m_background.Children().Remove(visual);
    visual.Offset(m_background.Offset());
    return card;
}

bool Foundation::CanAdd(std::shared_ptr<CompositionCard> const& card)
{
    auto cardValue = card->Value();
    if (m_cards.empty())
    {
        return cardValue.Face() == Face::Ace;
    }

    auto lastCard = m_cards.back();
    auto lastCardValue = lastCard->Value();
    if (cardValue.Suit() != lastCardValue.Suit())
    {
        return false;
    }

    return (int)cardValue.Face() == (int)lastCardValue.Face() + 1;
}

void Foundation::Add(std::shared_ptr<CompositionCard> const& card)
{
    WINRT_ASSERT(CanAdd(card));

    auto cardVisual = card->Root();
    float3 offset = { 0, 0, 0 };
    m_background.Children().InsertAtTop(cardVisual);
    cardVisual.Offset(offset);
    m_cards.push_back(card);
}