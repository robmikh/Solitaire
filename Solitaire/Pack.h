#pragma once

class ShapeCache;
class CompositionCard;

class Pack
{
public:
    Pack(std::shared_ptr<ShapeCache> const& shapeCache);
    ~Pack() {}

    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_cards; }
    void Shuffle();

private:
    std::shared_ptr<ShapeCache> m_shapeCache;
    std::vector<std::shared_ptr<CompositionCard>> m_cards;
};