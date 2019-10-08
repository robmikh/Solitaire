#pragma once

class ShapeCache;
class CompositionCard;

class Pile
{
public:
    struct ItemContainer
    {
        winrt::Windows::UI::Composition::ContainerVisual Root{ nullptr };
        winrt::Windows::UI::Composition::ContainerVisual Content{ nullptr };
    };

    struct RemovalOperation
    {
        int Index = -1;
    };

    using Card = std::shared_ptr<CompositionCard>;
    using CardList = std::vector<Pile::Card>;
    using ItemContainerList = std::vector<Pile::ItemContainer>;

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
    std::tuple<Pile::ItemContainerList, Pile::CardList, Pile::RemovalOperation> Split(int index);

    virtual bool CanTake(int index) = 0;
    std::tuple<Pile::ItemContainer, Pile::Card, Pile::RemovalOperation> Take(int index);

    void CompleteRemoval(Pile::RemovalOperation operation);
    void Return(Pile::CardList const& cards, Pile::RemovalOperation operation);

    virtual bool CanAdd(Pile::CardList const& cards) = 0;
    void Add(Pile::CardList const& cards);

    void ForceLayout();

protected:
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeOffset(int index, int totalCards) = 0;
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeBaseSpaceOffset(int index, int totalCards) = 0;
    virtual void OnRemovalCompleted(Pile::RemovalOperation operation) = 0;

protected:
    winrt::Windows::UI::Composition::ShapeVisual m_background{ nullptr };
    winrt::Windows::UI::Composition::VisualCollection m_children{ nullptr };
    Pile::CardList m_cards;
    std::vector<ItemContainer> m_itemContainers;
};