#pragma once
#include "Card.h"

enum class ShapeType
{
    Back,
    Empty
};

class ShapeCache
{
public:
    static std::future<std::shared_ptr<ShapeCache>> CreateAsync(winrt::Windows::UI::Composition::Compositor const& compositor);
    ~ShapeCache() {}

    winrt::Windows::UI::Composition::Compositor Compositor() { return m_compositor; }
    winrt::Windows::UI::Composition::CompositionPathGeometry GetPathGeometry(Card const& key);
    winrt::Windows::UI::Composition::CompositionShape GetShape(ShapeType shapeType);
    float TextHeight() { return m_textHeight; }

    // Workaround for make_shared
    ShapeCache() {}

private:
    winrt::Windows::Foundation::IAsyncAction FillCacheAsync(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::hstring const& fontFamily,
        float fontSize);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor;
    std::map<Card, winrt::Windows::UI::Composition::CompositionPathGeometry> m_geometryCache;
    std::map<ShapeType, winrt::Windows::UI::Composition::CompositionShape> m_shapeCache;
    float m_textHeight;
};