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
    auto numCardsBefore = m_cards.size();
    // All cards should be in the stack now. Only the cards we add will be fanned out.
    auto count = 0;
    for (auto container = m_itemContainers.rbegin(); container != m_itemContainers.rend() && count < 3; container++, count++)
    {
        container->Root.Offset({ 0, 0, 0 });
    }
    Add(cards);
    // Force the last three cards to be fanned out, regardless of layout
    count = 0;
    for (auto container = m_itemContainers.rbegin(); container != m_itemContainers.rend() && count < 3; container++, count++)
    {
        // Since we're walking the list backwards, we have to flip the count
        // indexRelativeToEnd = (last index) - count
        container->Root.Offset(ComputeOffset(3 - 1 - count, 3));
    }
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
    WINRT_ASSERT(index < totalCards);
    if (index > totalCards - 3)
    {
        return { m_horizontalOffset, 0, 0 };
    }
    return { 0, 0, 0 };
}

winrt::float3 Waste::ComputeBaseSpaceOffset(int index, int totalCards)
{
    WINRT_ASSERT(index < totalCards);
    if (index > totalCards - 3)
    {
        return { (index - (totalCards - 3)) * m_horizontalOffset, 0, 0 };
    }
    return { 0, 0, 0 };
}

void Waste::OnRemovalCompleted(Pile::RemovalOperation operation)
{
    // Force the last three cards to be fanned out, regardless of layout
    auto count = 0;
    for (auto container = m_itemContainers.rbegin(); container != m_itemContainers.rend() && count < 3; container++, count++)
    {
        // Since we're walking the list backwards, we have to flip the count
        // indexRelativeToEnd = (last index) - count
        container->Root.Offset(ComputeOffset(3 - 1 - count, 3));
    }
}
