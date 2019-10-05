#pragma once

class ShapeCache;
class CompositionCard;

class Waste
{
public:
    Waste(std::shared_ptr<ShapeCache> const& shapeCache);
    ~Waste() {}

    winrt::Windows::UI::Composition::Visual Base() { return m_background; }
    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_cards; }
    
    int HitTest(winrt::Windows::Foundation::Numerics::float2 point);
    std::shared_ptr<CompositionCard> Pick(int index);
    void AddCards(std::vector<std::shared_ptr<CompositionCard>> const& cards);
    std::vector<std::shared_ptr<CompositionCard>> Flush();
    bool RemoveCard(std::shared_ptr<CompositionCard> const& remove);
    void ForceLayout(float fanRatio);
    void InsertCard(std::shared_ptr<CompositionCard> const& card, int index);

private:
    winrt::Windows::UI::Composition::ShapeVisual m_background{ nullptr };
    std::vector<std::shared_ptr<CompositionCard>> m_cards;
    float m_fanRatio = 0;
};