#include"pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "Foundation.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI::Composition;
}

bool Foundation::CanSplit(int index)
{
    return !m_cards.empty() && index == m_cards.size() - 1;
}

bool Foundation::CanTake(int index)
{
    return false;
}

bool Foundation::CanAdd(Pile::CardList const& cards)
{
    if (cards.size() != 1)
    {
        return false;
    }

    auto card = cards.front();
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

winrt::float3 Foundation::ComputeOffset(int index)
{
    return { 0, 0, 0 };
}

winrt::float3 Foundation::ComputeBaseSpaceOffset(int index)
{
    return { 0, 0, 0 };
}

void Foundation::CompleteRemoval()
{
}
