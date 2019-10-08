#pragma once
#include "Pile.h"

class ShapeCache;
class CompositionCard;

class CardStack : public Pile
{
public:
    CardStack(std::shared_ptr<ShapeCache> const& shapeCache, std::vector<std::shared_ptr<CompositionCard>> cards) : Pile(shapeCache, cards) {}

    void SetLayoutOptions(float verticalOffset);

    virtual bool CanSplit(int index) override;
    virtual bool CanTake(int index) override;
    virtual bool CanAdd(Pile::CardList const& cards) override;

protected:
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeOffset(int index, int totalCards) override;
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeBaseSpaceOffset(int index, int totalCards) override;
    virtual void OnRemovalCompleted(Pile::RemovalOperation operation) override;

private:
    float m_verticalOffset = 0.0f;
};