#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "CardStack.h"

using namespace winrt;
using namespace Windows::Foundation::Numerics;

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

    return -1;
}