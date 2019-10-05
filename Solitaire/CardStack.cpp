#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "CardStack.h"

using namespace winrt;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI::Composition;

CardStack::CardStack(std::shared_ptr<ShapeCache> const& shapeCache, std::vector<std::shared_ptr<CompositionCard>> cards)
{
    m_stack = cards;
    
    auto compositor = shapeCache->Compositor();
    m_background = compositor.CreateShapeVisual();
    m_background.Shapes().Append(shapeCache->GetShape(ShapeType::Empty));
    m_background.Size(CompositionCard::CardSize);
}

void CardStack::ForceLayout(float verticalOffset)
{
    m_verticalOffset = verticalOffset;

    if (m_stack.empty())
    {
        return;
    }

    // The first element is the "base" of the stack
    auto parent = m_stack.front();
    m_background.Children().RemoveAll();
    m_background.Children().InsertAtTop(parent->Root());

    for (auto& card : m_stack)
    {
        if (parent == card)
        {
            continue;
        }

        auto cardVisual = card->Root();

        if (cardVisual.Parent())
        {
            cardVisual.Parent().Children().Remove(cardVisual);
        }

        cardVisual.Offset({ 0, m_verticalOffset, 0 });
        parent->ChainCard(*card.get());
        parent = card;
    }
}

int CardStack::HitTest(float2 point)
{
    point.x -= m_background.Offset().x;
    point.y -= m_background.Offset().y;
    for (int i = m_stack.size() - 1; i >= 0; i--)
    {
        auto card = m_stack[i];

        float2 accumulatedOffset = { 0, 0 };
        if (i > 0)
        {
            accumulatedOffset.y = m_verticalOffset * (i - 1);
        }
        
        if (card->HitTest(accumulatedOffset, point))
        {
            return i;
        }
    }

    float2 const size = m_background.Size();
    if (point.x >= 0 &&
        point.x < size.x &&
        point.y >= 0 &&
        point.y < size.y)
    {
        return -2;
    }

    return -1;
}

bool CardStack::CanSplit(int index)
{
    if (m_stack.empty() || index < 0 || index >= m_stack.size())
    {
        return false;
    }

    auto card = m_stack[index];
    if (!card->IsFaceUp())
    {
        return false;
    }

    return true;
}

std::vector<std::shared_ptr<CompositionCard>> CardStack::Split(int index)
{
    WINRT_ASSERT(CanSplit(index));

    auto start = m_stack.begin() + index;
    auto end = m_stack.end();
    std::vector<std::shared_ptr<CompositionCard>> result(
        std::make_move_iterator(start),
        std::make_move_iterator(end));
    m_stack.erase(start, end);

    if (!m_stack.empty())
    {
        m_stack.back()->ClearChain();
    }
    else
    {
        m_background.Children().RemoveAll();
    }

    return result;
}

bool CardStack::CanAdd(std::shared_ptr<CompositionCard> const& card)
{
    auto cardValue = card->Value();
    if (m_stack.empty())
    {
        return cardValue.Face() == Face::King;
    }

    auto lastCard = m_stack.back();
    if (!lastCard->IsFaceUp())
    {
        return false;
    }

    auto lastCardValue = lastCard->Value();
    if (cardValue.IsRed() == lastCardValue.IsRed())
    {
        return false;
    }

    return (int)cardValue.Face() == (int)lastCardValue.Face() - 1;
}

void CardStack::Add(std::vector<std::shared_ptr<CompositionCard>> const& cards)
{
    //WINRT_ASSERT(CanAdd(cards.front()));
    if (cards.empty())
    {
        return;
    }

    auto frontCard = cards.front();
    auto frontCardVisual = frontCard->Root();
    float3 offset = { 0, 0, 0 };
    if (!m_stack.empty())
    {
        auto parent = m_stack.back();
        parent->ChainCard(*frontCard.get());
        offset = { 0, m_verticalOffset, 0 };
    }
    else
    {
        m_background.Children().InsertAtTop(frontCardVisual);
    }
    frontCardVisual.Offset(offset);

    for (auto& card : cards)
    {
        m_stack.push_back(card);
    }
}