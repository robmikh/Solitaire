#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "Deck.h"

using namespace winrt;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI::Composition;

Deck::Deck(std::shared_ptr<ShapeCache> const& shapeCache, std::vector<std::shared_ptr<CompositionCard>> cards)
{
    m_cards = cards;

    auto compositor = shapeCache->Compositor();
    m_background = compositor.CreateShapeVisual();
    m_background.Shapes().Append(shapeCache->GetShape(ShapeType::Empty));
    m_background.Size(CompositionCard::CardSize);
    m_background.Comment(L"Deck Root");
}

bool Deck::HitTest(float2 point)
{
    float3 const offset = m_background.Offset();
    float2 const size = m_background.Size();
    if (point.x >= offset.x &&
        point.x < offset.x + size.x &&
        point.y >= offset.y &&
        point.y < offset.y + size.y)
    {
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<CompositionCard>> Deck::Draw()
{
    // Take the top 3 cards
    auto availableCards = 3;
    if (m_cards.size() < 3)
    {
        availableCards = m_cards.size();
    }

    if (availableCards > 0)
    {
        auto start = m_cards.begin() + (m_cards.size() - availableCards);
        auto end = m_cards.end();
        std::vector<std::shared_ptr<CompositionCard>> tempCards(
            std::make_move_iterator(start),
            std::make_move_iterator(end));
        m_cards.erase(start, end);

        std::vector<std::shared_ptr<CompositionCard>> cards(tempCards.rbegin(), tempCards.rend());
        for (auto& card : cards)
        {
            m_background.Children().Remove(card->Root());
            //card->IsFaceUp(true);
        }

        return cards;
    }

    return {};
}

void Deck::AddCards(std::vector<std::shared_ptr<CompositionCard>> const& cards)
{
    for (auto& card : cards)
    {
        auto visual = card->Root();
        visual.Offset({ 0, 0, 0 });
        //visual.Parent().Children().Remove(visual);
        m_background.Children().InsertAtTop(visual);
        card->IsFaceUp(false);

        m_cards.push_back(card);
    }
}

void Deck::ForceLayout()
{
    m_background.Children().RemoveAll();
    for (auto& card : m_cards)
    {
        auto visual = card->Root();
        visual.Offset({ 0, 0, 0 });

        if (visual.Parent())
        {
            visual.Parent().Children().Remove(visual);
        }

        card->IsFaceUp(false);
        m_background.Children().InsertAtTop(visual);
    }
}
