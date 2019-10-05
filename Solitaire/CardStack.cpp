#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "CardStack.h"

CardStack::CardStack(std::vector<std::shared_ptr<CompositionCard>> cards)
{
    m_stack = cards;
}

void CardStack::ForceLayout(float verticalOffset)
{
    if (m_stack.empty())
    {
        return;
    }

    // The first element is the "base" of the stack
    auto parentVisual = m_stack.front()->Root();

    for (auto& card : m_stack)
    {
        auto cardVisual = card->Root();

        if (parentVisual == cardVisual)
        {
            continue;
        }

        if (cardVisual.Parent() && cardVisual.Parent() != parentVisual)
        {
            cardVisual.Parent().Children().Remove(cardVisual);
        }

        cardVisual.Offset({ 0, verticalOffset, 0 });
        parentVisual.Children().InsertAtTop(cardVisual);
        parentVisual = cardVisual;
    }
}