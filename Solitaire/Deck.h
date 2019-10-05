#pragma once

class ShapeCache;
class CompositionCard;

class Deck
{
public:
    Deck(std::shared_ptr<ShapeCache> const& shapeCache, std::vector<std::shared_ptr<CompositionCard>> cards);
    ~Deck() {}

    winrt::Windows::UI::Composition::Visual Base() { return m_background; }
    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_cards; }

    bool HitTest(winrt::Windows::Foundation::Numerics::float2 point);
    std::vector<std::shared_ptr<CompositionCard>> Draw();
    void AddCards(std::vector<std::shared_ptr<CompositionCard>> const& cards);
    void ForceLayout();

private:
    winrt::Windows::UI::Composition::ShapeVisual m_background{ nullptr };
    std::vector<std::shared_ptr<CompositionCard>> m_cards;
    float m_fanRatio = 0;
};