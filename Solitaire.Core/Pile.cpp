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

Pile::ItemContainer CreateItemContainer(winrt::Compositor const& compositor)
{
    auto root = compositor.CreateContainerVisual();
    root.Size(CompositionCard::CardSize);
    root.Comment(L"Item Container Root");
    auto content = compositor.CreateContainerVisual();
    content.RelativeSizeAdjustment({ 1, 1 });
    content.Comment(L"Item Container Content");
    root.Children().InsertAtTop(content);
    return { root, content };
}

std::vector<Pile::ItemContainer> CreateItemContainers(winrt::Compositor const& compositor, uint32_t numberOfItems)
{
    std::vector<Pile::ItemContainer> result(numberOfItems);
    for (int i = 0; i < numberOfItems; i++)
    {
        auto container = CreateItemContainer(compositor);

        auto previousIndex = i - 1;
        if (previousIndex >= 0)
        {
            auto previousContainer = result[previousIndex];
            previousContainer.Root.Children().InsertAbove(container.Root, previousContainer.Content);
        }

        result[i] = container;
    }
    return result;
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
    m_itemContainers = CreateItemContainers(shapeCache->Compositor(), m_cards.size());
}

void Pile::ForceLayout()
{
    if (!m_itemContainers.empty())
    {
        if (!m_itemContainers.front().Root.Parent())
        {
            m_children.InsertAtTop(m_itemContainers.front().Root);
        }
    }
    auto index = 0;
    for (auto& card : m_cards)
    {
        auto visual = card->Root();
        auto offset = ComputeOffset(index, m_cards.size());
        m_itemContainers[index].Root.Offset(offset);
        if (visual.Parent() && visual.Parent() != m_itemContainers[index].Content)
        {
            visual.Parent().Children().Remove(visual);
        }
        else if (!visual.Parent())
        {
            m_itemContainers[index].Content.Children().InsertAtTop(visual);
        }
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

        winrt::float3 accumulatedOffset = ComputeBaseSpaceOffset(i, m_cards.size());

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

std::tuple<Pile::ItemContainerList, Pile::CardList, Pile::RemovalOperation> Pile::Split(int index)
{
    WINRT_ASSERT(CanSplit(index));
    WINRT_ASSERT(m_itemContainers.size() == m_cards.size());

    auto startingSize = m_cards.size();

    auto start = m_cards.begin() + index;
    auto end = m_cards.end();
    Pile::CardList cards(
        std::make_move_iterator(start),
        std::make_move_iterator(end));
    m_cards.erase(start, end);

    auto compositor = m_background.Compositor();
    auto containers = CreateItemContainers(compositor, cards.size());

    auto cardIndex = 0;
    auto mainContainerListIndex = index;
    for (auto& newContainer : containers)
    {
        auto card = cards[cardIndex];
        auto visual = card->Root();
        m_itemContainers[mainContainerListIndex].Content.Children().Remove(visual);

        auto offset = ComputeOffset(mainContainerListIndex, startingSize);
        newContainer.Root.Offset(offset);
        newContainer.Content.Children().InsertAtTop(visual);

        mainContainerListIndex++;
        cardIndex++;
    }
    
    // We don't need to add an offset to the first container since we're 
    // going to use ParentForTransform to put the it in the right place.
    containers.front().Root.Offset({ 0, 0, 0 });
    containers.front().Root.ParentForTransform(m_itemContainers[index].Root);

    return { containers, cards, { index } };
}

std::tuple<Pile::ItemContainer, Pile::Card, Pile::RemovalOperation> Pile::Take(int index)
{
    WINRT_ASSERT(CanTake(index));
    WINRT_ASSERT(m_itemContainers.size() == m_cards.size());

    auto startingSize = m_cards.size();

    auto card = m_cards[index];
    auto visualToRemove = card->Root();
    m_cards.erase(m_cards.begin() + index);

    auto oldContainer = m_itemContainers[index];
    oldContainer.Content.Children().Remove(visualToRemove);

    auto compositor = m_background.Compositor();
    auto newContainer = CreateItemContainer(compositor);
    newContainer.Content.Children().InsertAtTop(visualToRemove);

    newContainer.Root.ParentForTransform(m_itemContainers[index].Root);

    return { newContainer, card, { index } };
}

void Pile::Add(Pile::CardList const& cards)
{
    WINRT_ASSERT(CanAdd(cards));
    AddInternal(cards);
}

void Pile::AddInternal(Pile::CardList const& cards)
{
    WINRT_ASSERT(m_itemContainers.size() == m_cards.size());
    if (cards.empty())
    {
        return;
    }

    auto compositor = m_background.Compositor();
    auto newContainers = CreateItemContainers(compositor, cards.size());

    if (!m_itemContainers.empty())
    {
        m_itemContainers.back().Root.Children().InsertAtTop(newContainers.front().Root);
    }
    else
    {
        m_children.InsertAtTop(newContainers.front().Root);
    }

    auto newContainerIndex = 0;
    auto totalSize = m_cards.size() + cards.size();
    for (auto& card : cards)
    {
        auto mainListIndex = m_cards.size();
        auto visual = card->Root();
        visual.Offset({ 0, 0, 0 });

        auto newContainer = newContainers[newContainerIndex];
        newContainer.Root.Offset(ComputeOffset(mainListIndex, totalSize));
        newContainer.Content.Children().InsertAtTop(visual);
        m_itemContainers.push_back(newContainer);
        m_cards.push_back(card);

        newContainerIndex++;
    }
}

void Pile::Return(Pile::CardList const& cards, Pile::RemovalOperation operation)
{
    if (cards.empty())
    {
        return;
    }

    auto index = operation.Index;
    auto returnedCardIndex = 0;
    for (
        auto container = m_itemContainers.begin() + operation.Index; 
        container != m_itemContainers.begin() + operation.Index + cards.size(); 
        container++)
    {
        auto cardVisual = cards[returnedCardIndex]->Root();
        cardVisual.Offset({ 0, 0, 0 });
        container->Content.Children().InsertAtTop(cardVisual);
        m_cards.insert(m_cards.begin() + index, cards[returnedCardIndex]);
        returnedCardIndex++;
        index++;
    }

    WINRT_ASSERT(m_itemContainers.size() == m_cards.size());
}

void Pile::CompleteRemoval(Pile::RemovalOperation operation)
{
    WINRT_ASSERT(m_itemContainers.size() > m_cards.size());
    auto endIndex = operation.Index;
    for (auto container = m_itemContainers.begin() + operation.Index; container != m_itemContainers.end(); container++)
    {
        if (container->Content.Children().Count() > 0)
        {
            break;
        }
        endIndex++;
    }

    auto start = m_itemContainers.begin() + operation.Index;
    auto end = m_itemContainers.begin() + endIndex;
    Pile::ItemContainerList containers(
        std::make_move_iterator(start),
        std::make_move_iterator(end));
    m_itemContainers.erase(start, end);

    auto previousIndex = operation.Index - 1;
    auto parentChildren = m_children;
    if (previousIndex >= 0)
    {
        parentChildren = m_itemContainers[previousIndex].Root.Children();
    }
    parentChildren.Remove(containers.front().Root);

    if (operation.Index < m_itemContainers.size())
    {
        auto shiftedVisual = m_itemContainers[operation.Index].Root;
        if (shiftedVisual.Parent())
        {
            shiftedVisual.Parent().Children().Remove(shiftedVisual);
        }
        parentChildren.InsertAtTop(shiftedVisual);

        auto currentIndex = operation.Index;
        for (auto container = m_itemContainers.begin() + operation.Index; container != m_itemContainers.end(); container++)
        {
            container->Root.Offset(ComputeOffset(currentIndex, m_itemContainers.size()));
            currentIndex++;
        }
    }

    WINRT_ASSERT(m_itemContainers.size() == m_cards.size());
    OnRemovalCompleted(operation);
}