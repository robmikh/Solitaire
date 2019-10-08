#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "Waste.h"

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

void Waste::SetLayoutOptions(float horizontalOffset)
{
    m_horizontalOffset = horizontalOffset;
}

Pile::CardList Waste::Flush()
{
    for (auto& container : m_itemContainers)
    {
        container.Content.Children().RemoveAll();
    }
    m_itemContainers.clear();
    m_children.RemoveAll();

    std::vector<std::shared_ptr<CompositionCard>> result(
        std::make_move_iterator(m_cards.rbegin()),
        std::make_move_iterator(m_cards.rend()));
    m_cards.erase(m_cards.begin(), m_cards.end());

    return result;
}

void Waste::Discard(Pile::CardList const& cards)
{
    Add(cards);
    ForceLayout();
}

bool Waste::CanTake(int index)
{
    if (m_cards.empty() || index < 0 || index >= m_cards.size())
    {
        return false;
    }

    return true;
}

bool Waste::CanAdd(Pile::CardList const& cards)
{
    return false;
}

winrt::float3 Waste::ComputeOffset(int index, int totalCards)
{
    if (index > totalCards - 3 && index < totalCards)
    {
        return { m_horizontalOffset, 0, 0 };
    }
    return { 0, 0, 0 };
}

winrt::float3 Waste::ComputeBaseSpaceOffset(int index, int totalCards)
{
    if (index > totalCards - 3 && index < totalCards)
    {
        return { (index - (totalCards - 3)) * m_horizontalOffset, 0, 0 };
    }
    return { 0, 0, 0 };
}

void Waste::OnRemovalCompleted(Pile::RemovalOperation operation)
{
}
