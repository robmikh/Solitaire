#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "ShapeCache.h"
#include "Waste.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

Waste::Waste(std::shared_ptr<ShapeCache> const& shapeCache)
{
    auto compositor = shapeCache->Compositor();
    m_background = compositor.CreateShapeVisual();
    m_background.Shapes().Append(shapeCache->GetShape(ShapeType::Empty));
    m_background.Size(CompositionCard::CardSize);
}

int Waste::HitTest(winrt::Windows::Foundation::Numerics::float2 point)
{
    float3 const offset = m_background.Offset();
    int index = m_cards.size() - 1;
    for (auto it = m_cards.rbegin(); it != m_cards.rend(); ++it)
    {
        auto card = *it;
        auto temp = point;
        temp.x -= offset.x;
        temp.y -= offset.y;
        if (card->HitTest(temp))
        {
            return index;
        }
        index--;
    }

    return -1;
}

std::shared_ptr<CompositionCard> Waste::Pick(int index)
{
    if (m_cards.empty() || index < 0 || index >= m_cards.size())
    {
        return nullptr;
    }

    auto card = m_cards[index];
    m_cards.erase(m_cards.begin() + index);
    auto visual = card->Root();
    auto parent = visual.Parent();
    auto offset = visual.Offset();
    offset.x += parent.Offset().x;
    visual.Offset(offset);
    parent.Children().Remove(visual);
    return card;
}

void Waste::AddCards(std::vector<std::shared_ptr<CompositionCard>> const& cards)
{
    auto count = 0;
    for (auto& card : cards)
    {
        auto visual = card->Root();
        m_cards.push_back(card);
        float2 const size = visual.Size();
        visual.Offset({ count * m_fanRatio, 0, 0 });
        m_background.Children().InsertAtTop(visual);
        count++;
    }
}

std::vector<std::shared_ptr<CompositionCard>> Waste::Flush()
{
    std::vector<std::shared_ptr<CompositionCard>> result(
        std::make_move_iterator(m_cards.rbegin()),
        std::make_move_iterator(m_cards.rend()));
    m_cards.erase(m_cards.begin(), m_cards.end());
    return result;
}

bool Waste::RemoveCard(std::shared_ptr<CompositionCard> const& remove)
{
    auto find = std::find(m_cards.begin(), m_cards.end(), remove);
    if (find != m_cards.end())
    {
        m_cards.erase(find);
        return true;
    }
    return false;
}

void Waste::ForceLayout(float fanRatio)
{
    m_fanRatio = fanRatio;

    if (m_cards.empty())
    {
        return;
    }

    m_background.Children().RemoveAll();
    auto count = 0;
    auto threshold = m_cards.size() - 3;
    if (m_cards.size() < 3)
    {
        threshold = 0;
    }
    for (auto& card : m_cards)
    {
        auto visual = card->Root();
        
        if (visual.Parent())
        {
            visual.Parent().Children().Remove(visual);
        }

        m_background.Children().InsertAtTop(visual);

        float2 const size = visual.Size();
        visual.Offset({ (count > threshold ? count - threshold : 0) * m_fanRatio, 0, 0 });
        count++;
    }
}

void Waste::InsertCard(std::shared_ptr<CompositionCard> const& card, int index)
{
    m_cards.insert(m_cards.begin() + index, card);
    index--;
    if (index >= 0)
    {
        auto previousVisual = m_cards[index]->Root();
        m_background.Children().InsertAbove(card->Root(), previousVisual);
    }
}