#pragma once

enum class ShapeType
{
    Back,
    Empty
};

class ShapeCache
{
public:
    ShapeCache(winrt::Windows::UI::Composition::Compositor const& compositor);
    ~ShapeCache() {}

    winrt::Windows::UI::Composition::Compositor Compositor() { return m_compositor; }
    winrt::Windows::UI::Composition::CompositionPathGeometry GetPathGeometry(winrt::hstring const& key);
    winrt::Windows::UI::Composition::CompositionShape GetShape(ShapeType shapeType);
    float TextHeight() { return m_textHeight; }

private:

    void FillCache(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::hstring const& fontFamily,
        float fontSize);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor;
    std::map<winrt::hstring, winrt::Windows::UI::Composition::CompositionPathGeometry> m_geometryCache;
    std::map<ShapeType, winrt::Windows::UI::Composition::CompositionShape> m_shapeCache;
    float m_textHeight;
};