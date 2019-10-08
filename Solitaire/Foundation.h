#pragma once
#include "Pile.h"

class ShapeCache;
class CompositionCard;

class Foundation : public Pile
{
public:
    Foundation(std::shared_ptr<ShapeCache> const& shapeCache) : Pile(shapeCache) {}

    virtual bool CanSplit(int index) override;
    virtual bool CanTake(int index) override;
    virtual bool CanAdd(Pile::CardList const& cards) override;

protected:
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeOffset(int index, int totalCards) override;
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeBaseSpaceOffset(int index, int totalCards) override;
    virtual void OnRemovalCompleted(Pile::RemovalOperation operation) override;
};