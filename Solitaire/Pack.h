#pragma once

class ShapeCache;

class CompositionCard
{
public:
    CompositionCard(
        Card card, 
        std::shared_ptr<ShapeCache> const& shapeCache);
    ~CompositionCard() {}

    Card Value() { return m_card; }
    winrt::Windows::UI::Composition::Visual Root() { return m_root; }

private:
    winrt::Windows::UI::Composition::ShapeVisual m_root{ nullptr };
    Card m_card;
};

class Pack
{
public:
    Pack(std::shared_ptr<ShapeCache> const& shapeCache);
    ~Pack() {}

    const std::vector<CompositionCard>& Cards() const { return m_cards; }

private:
    std::shared_ptr<ShapeCache> m_shapeCache;
    std::vector<CompositionCard> m_cards;
};