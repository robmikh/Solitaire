#pragma once

class ShapeCache;
class CompositionCard;

class Pile
{
public:
    using Card = std::shared_ptr<CompositionCard>;
    using CardList = std::vector<Pile::Card>;

    Pile(std::shared_ptr<ShapeCache> const& shapeCache);
    Pile(std::shared_ptr<ShapeCache> const& shapeCache, Pile::CardList cards);
    ~Pile() {}

    winrt::Windows::UI::Composition::Visual Base() { return m_background; }
    const Pile::CardList& Cards() const { return m_cards; }

    enum class HitTestTarget
    {
        None,
        Base,
        Card
    };

    struct HitTestResult
    {
        Pile::HitTestTarget Target = Pile::HitTestTarget::None;
        int CardIndex = -1;
    };

    Pile::HitTestResult HitTest(winrt::Windows::Foundation::Numerics::float2 point);

    virtual bool CanSplit(int index) = 0;
    Pile::CardList Split(int index);

    virtual bool CanAdd(Pile::CardList const& cards) = 0;
    void Add(Pile::CardList const& cards);
    void Return(Pile::CardList const& cards, int index);

    void ForceLayout();

protected:
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeOffset(int index) = 0;
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeBaseSpaceOffset(int index) = 0;

private:
    std::tuple<Pile::CardList, winrt::Windows::UI::Composition::Visual, winrt::Windows::Foundation::Numerics::float3> SplitInternal(int index);

protected:
    winrt::Windows::UI::Composition::ShapeVisual m_background{ nullptr };
    winrt::Windows::UI::Composition::VisualCollection m_children{ nullptr };
    Pile::CardList m_cards;
};