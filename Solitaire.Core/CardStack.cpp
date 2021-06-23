#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "CardStack.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI::Composition;
}

void CardStack::SetLayoutOptions(float verticalOffset)
{
    m_verticalOffset = verticalOffset;
}

bool CardStack::CanSplit(int index)
{
    if (m_cards.empty() || index < 0 || index >= m_cards.size())
    {
        return false;
    }

    auto card = m_cards[index];
    if (!card->IsFaceUp())
    {
        return false;
    }

    return true;
}

bool CardStack::CanTake(int index)
{
    return false;
}

bool CardStack::CanAdd(Pile::CardList const& cards)
{
    if (cards.empty())
    {
        return true;
    }

    if (m_cards.size() != m_itemContainers.size())
    {
        return false;
    }

    auto card = cards.front();
    auto cardValue = card->Value();
    if (m_cards.empty())
    {
        return cardValue.Face() == Face::King;
    }

    auto lastCard = m_cards.back();
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

winrt::float3 CardStack::ComputeOffset(int index, int totalCards)
{
    return { 0, index == 0 ? 0 : m_verticalOffset, 0 };
}

winrt::float3 CardStack::ComputeBaseSpaceOffset(int index, int totalCards)
{
    return { 0, index * m_verticalOffset, 0 };
}

void CardStack::OnRemovalCompleted(Pile::RemovalOperation operation)
{
    if (!m_cards.empty())
    {
        auto card = m_cards.back();
        card->AnimateIsFaceUp(true, std::chrono::milliseconds(250), std::chrono::milliseconds(0));
    }
}