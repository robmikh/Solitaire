#pragma once

class ShapeCache
{
public:
    ShapeCache(winrt::Windows::UI::Composition::Compositor const& compositor);
    ~ShapeCache() {}

    winrt::Windows::UI::Composition::CompositionPathGeometry GetPathGeometry(winrt::hstring const& key);

private:

    void FillCache(
        winrt::Windows::UI::Composition::Compositor const& compositor,
        winrt::hstring const& fontFamily,
        float fontSize);

private:
    std::map<winrt::hstring, winrt::Windows::UI::Composition::CompositionPathGeometry> m_geometryCache;
};