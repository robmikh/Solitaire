#pragma once

class CompositionCard;

class Waste
{
public:
    Waste() {}
    ~Waste() {}

    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_cards; }
    
    std::shared_ptr<CompositionCard> HitTest(winrt::Windows::Foundation::Numerics::float2 point);
    void AddCards(std::vector<std::shared_ptr<CompositionCard>> const& cards);
    std::vector<std::shared_ptr<CompositionCard>> Flush();

private:
    std::vector<std::shared_ptr<CompositionCard>> m_cards;
};