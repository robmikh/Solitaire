#include "pch.h"
#include "ShapeCache.h"

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.Geometry.h>
#include <winrt/Microsoft.Graphics.Canvas.Text.h>

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

using namespace Microsoft::Graphics::Canvas;
using namespace Microsoft::Graphics::Canvas::Geometry;
using namespace Microsoft::Graphics::Canvas::Text;


ShapeCache::ShapeCache(
    Compositor const& compositor)
{
    FillCache(
        compositor,
        L"Segoe UI",
        36);
}

CompositionPathGeometry ShapeCache::GetPathGeometry(
    hstring const& key)
{
    return m_geometryCache.at(key);
}

void ShapeCache::FillCache(
    Compositor const& compositor,
    hstring const& fontFamily,
    float fontSize)
{
    hstring faces[] = 
    {
        L"K",
        L"Q",
        L"J",
        L"10",
        L"9",
        L"8",
        L"7",
        L"6",
        L"5",
        L"4",
        L"3",
        L"2",
        L"A"
    };

    hstring suits[] =
    {
        L"♠",
        L"♣",
        L"♥",
        L"♦"
    };

    std::vector<hstring> cards;
    for (auto& face : faces)
    {
        for (auto& suit : suits)
        {
            auto card = face + suit;

            cards.push_back(card);
        }
    }

    auto device = CanvasDevice();
    m_geometryCache.clear();
    for (auto& card : cards)
    {
        auto textFormat = CanvasTextFormat();
        textFormat.FontFamily(fontFamily);
        textFormat.FontSize(fontSize);
        auto textLayout = CanvasTextLayout(device, card, textFormat, 500, 0);
        auto geometry = CanvasGeometry::CreateText(textLayout);

        auto compositionPath = CompositionPath(geometry);
        auto pathGeoemtry = compositor.CreatePathGeometry(compositionPath);

        m_geometryCache.emplace(card, pathGeoemtry);
    }
}