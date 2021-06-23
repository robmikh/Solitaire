#pragma once

class ShapeCache;
class CompositionCard;

class Pack
{
public:
    struct ShuffleSeed
    {
        unsigned int Num1 = 0;
        unsigned int Num2 = 0;
        unsigned int Num3 = 0;
        unsigned int Num4 = 0;
    };

    Pack(std::shared_ptr<ShapeCache> const& shapeCache);
    ~Pack() {}

    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_cards; }
    void Shuffle();
    void Shuffle(ShuffleSeed seed);
    
private:
    std::shared_ptr<ShapeCache> m_shapeCache;
    std::vector<std::shared_ptr<CompositionCard>> m_cards;
    ShuffleSeed m_currentSeed = {};
};