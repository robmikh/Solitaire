#pragma once

class ShapeCache;

class CompositionCard
{
public:
    static const winrt::Windows::Foundation::Numerics::float2 CardSize;

    CompositionCard(
        Card card,
        std::shared_ptr<ShapeCache> const& shapeCache);
    ~CompositionCard() {}

    Card Value() { return m_card; }
    winrt::Windows::UI::Composition::Visual Root() { return m_root; }
    bool IsFaceUp() { return m_isFaceUp; }

    bool HitTest(
        winrt::Windows::Foundation::Numerics::float2 accumulatedOffset, 
        winrt::Windows::Foundation::Numerics::float2 point);
    void ChainCard(CompositionCard const& card);
    void ClearChain() { m_chainedCards.Children().RemoveAll(); }
    void IsFaceUp(bool isFaceUp);
    void Flip() { IsFaceUp(!m_isFaceUp); }

private:
    winrt::Windows::UI::Composition::ContainerVisual m_root{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_sidesRoot{ nullptr };
    winrt::Windows::UI::Composition::ShapeVisual m_front{ nullptr };
    winrt::Windows::UI::Composition::ShapeVisual m_back{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_chainedCards{ nullptr };
    Card m_card;
    bool m_isFaceUp = true;
};