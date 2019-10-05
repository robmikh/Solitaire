#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "Pack.h"
#include "ShapeCache.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

Pack::Pack(std::shared_ptr<ShapeCache> const& shapeCache)
{
    m_shapeCache = shapeCache;

    for (auto i = 0; i < (int)Face::King; i++)
    {
        auto face = (Face)(i + 1);
        for (auto j = 0; j < (int)Suit::Club + 1; j++)
        {
            auto suit = (Suit)(j);
            auto card = Card(face, suit);
            m_cards.push_back(std::make_shared<CompositionCard>(card, m_shapeCache));
        }
    }
}

void Pack::Shuffle()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_cards.begin(), m_cards.end(), g);
}