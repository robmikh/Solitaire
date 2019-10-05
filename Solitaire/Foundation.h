#pragma once

class ShapeCache;
class CompositionCard;

class Foundation
{
public:
    Foundation(std::shared_ptr<ShapeCache> const& shapeCache);

    winrt::Windows::UI::Composition::Visual Base() { return m_background; }
    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_cards; }
    void ForceLayout();

    bool HitTest(winrt::Windows::Foundation::Numerics::float2 point);
    bool CanTake();
    std::shared_ptr<CompositionCard> Take();
    bool CanAdd(std::shared_ptr<CompositionCard> const& card);
    void Add(std::shared_ptr<CompositionCard> const& card);

private:
    std::vector<std::shared_ptr<CompositionCard>> m_cards;
    winrt::Windows::UI::Composition::ShapeVisual m_background{ nullptr };
};