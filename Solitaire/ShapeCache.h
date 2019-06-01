#pragma once

class ShapeCache
{
public:
    ShapeCache(winrt::Windows::UI::Composition::Compositor const& compositor);
    ~ShapeCache() {}

    winrt::Windows::UI::Composition::Compositor Compositor() { return m_compositor; }
    winrt::Windows::UI::Composition::CompositionPathGeometry GetPathGeometry(winrt::hstring const& key);
    float TextHeight() { return m_textHeight; }

private:

    void FillCache(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::hstring const& fontFamily,
        float fontSize);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor;
    std::map<winrt::hstring, winrt::Windows::UI::Composition::CompositionPathGeometry> m_geometryCache;
    float m_textHeight;
};