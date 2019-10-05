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
    winrt::Windows::UI::Composition::ContainerVisual Root() { return m_root; }

private:
    winrt::Windows::UI::Composition::ShapeVisual m_root{ nullptr };
    Card m_card;
};