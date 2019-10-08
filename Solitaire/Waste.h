#pragma once
#include "Pile.h"

class ShapeCache;
class CompositionCard;

class Waste : public Pile
{
public:
    Waste(std::shared_ptr<ShapeCache> const& shapeCache) : Pile(shapeCache) { m_background.Comment(L"Waste Root"); }

    void SetLayoutOptions(float horizontalOffset);
    Pile::CardList Flush();
    void Discard(Pile::CardList const& cards);

    virtual bool CanSplit(int index) override { return false; }
    virtual bool CanTake(int index) override;
    virtual bool CanAdd(Pile::CardList const& cards) override;

protected:
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeOffset(int index, int totalCards) override;
    virtual winrt::Windows::Foundation::Numerics::float3 ComputeBaseSpaceOffset(int index, int totalCards) override;
    virtual void OnRemovalCompleted(Pile::RemovalOperation operation) override;

private:
    float m_horizontalOffset = 0.0f;
};