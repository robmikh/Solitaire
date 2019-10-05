#pragma once

class ShapeCache;
class CompositionCard;

class CardStack
{
public:
    CardStack(std::shared_ptr<ShapeCache> const& shapeCache, std::vector<std::shared_ptr<CompositionCard>> cards);

    winrt::Windows::UI::Composition::Visual Base() { return m_background; }
    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_stack; }
    void ForceLayout(float verticalOffset);

    int HitTest(winrt::Windows::Foundation::Numerics::float2 point);

private:
    std::vector<std::shared_ptr<CompositionCard>> m_stack;
    winrt::Windows::UI::Composition::ShapeVisual m_background{ nullptr };
    float m_verticalOffset = 0.0f;
};