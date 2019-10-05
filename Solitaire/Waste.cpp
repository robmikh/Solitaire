#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "Waste.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

std::shared_ptr<CompositionCard> Waste::HitTest(winrt::Windows::Foundation::Numerics::float2 point)
{
    return nullptr;
}

void Waste::AddCards(std::vector<std::shared_ptr<CompositionCard>> const& cards)
{
    auto count = 0;
    for (auto& card : cards)
    {
        auto visual = card->Root();
        m_cards.push_back(card);
        float2 const size = visual.Size();
        visual.Offset({ size.x + 25.0f + (count * 65.0f), 0, 0 });
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