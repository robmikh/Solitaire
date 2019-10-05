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
    auto parent = m_stack.front();

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

        cardVisual.Offset({ 0, verticalOffset, 0 });
        parent->ChainCard(*card.get());
        parent = card;
    }
}