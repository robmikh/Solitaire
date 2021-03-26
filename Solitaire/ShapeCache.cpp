#include "pch.h"
#include "ShapeCache.h"
#include "Card.h"
#include "CompositionCard.h"

namespace winrt
{
    using namespace Windows;
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Core;
    using namespace Windows::UI::Composition;

    using namespace Microsoft::Graphics::Canvas;
    using namespace Microsoft::Graphics::Canvas::Geometry;
    using namespace Microsoft::Graphics::Canvas::Text;
}

std::wstring GetSvgFilePath(Card const& card);

std::future<std::shared_ptr<ShapeCache>> ShapeCache::CreateAsync(
    winrt::Compositor const& compositor)
{
    auto cache = std::make_shared<ShapeCache>();
    co_await cache->FillCacheAsync(compositor, L"Segoe UI", 36);
    co_return cache;
}

winrt::CompositionPathGeometry ShapeCache::GetPathGeometry(
    Card const& key)
{
    return m_geometryCache.at(key);
}

winrt::CompositionShape ShapeCache::GetShape(ShapeType shapeType)
{
    return m_shapeCache.at(shapeType);
}

winrt::IAsyncAction ShapeCache::FillCacheAsync(
    winrt::Compositor const& compositor,
    winrt::hstring const& fontFamily,
    float fontSize)
{
    std::vector<Card> cards;
    for (auto i = 0; i < (int)Face::King; i++)
    {
        auto face = (Face)(i + 1);
        for (auto j = 0; j < (int)Suit::Club + 1; j++)
        {
            auto suit = (Suit)(j);
            auto card = Card(face, suit);
            cards.push_back(card);
        }
    }

    auto device = winrt::CanvasDevice();
    m_geometryCache.clear();
    auto height = -1.0f;
    for (auto& card : cards)
    {
        auto filePath = GetSvgFilePath(card);



        auto textFormat = winrt::CanvasTextFormat();
        textFormat.FontFamily(fontFamily);
        textFormat.FontSize(fontSize);
        auto textLayout = winrt::CanvasTextLayout(device, card.ToString(), textFormat, 500, 0);
        auto geometry = winrt::CanvasGeometry::CreateText(textLayout);

        auto compositionPath = winrt::CompositionPath(geometry);
        auto pathGeoemtry = compositor.CreatePathGeometry(compositionPath);

        m_geometryCache.emplace(card, pathGeoemtry);

        if (height < 0)
        {
            height = textLayout.LayoutBounds().Height;
        }
        WINRT_ASSERT(height == textLayout.LayoutBounds().Height);
    }

    m_shapeCache.clear();
    {
        auto shapeContainer = compositor.CreateContainerShape();
        auto backgroundBaseColor = winrt::Colors::Blue();

        auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        roundedRectGeometry.CornerRadius({ 10, 10 });
        roundedRectGeometry.Size(CompositionCard::CardSize);
        auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
        rectShape.StrokeBrush(compositor.CreateColorBrush(winrt::Colors::Gray()));
        rectShape.FillBrush(compositor.CreateColorBrush(backgroundBaseColor));
        rectShape.StrokeThickness(2);
        shapeContainer.Shapes().Append(rectShape);

        winrt::float2 innerOffset{ 12, 12 };
        auto innerRoundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        innerRoundedRectGeometry.CornerRadius({ 6, 6 });
        innerRoundedRectGeometry.Size(CompositionCard::CardSize - innerOffset);
        auto innerRectShape = compositor.CreateSpriteShape(innerRoundedRectGeometry);
        innerRectShape.StrokeBrush(compositor.CreateColorBrush(winrt::Colors::White()));
        innerRectShape.StrokeThickness(5);
        innerRectShape.Offset(innerOffset / 2.0f);
        shapeContainer.Shapes().Append(innerRectShape);

        m_shapeCache.emplace(ShapeType::Back, shapeContainer);
    }

    {
        auto shapeContainer = compositor.CreateContainerShape();
        auto backgroundBaseColor = winrt::Colors::Blue();

        auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        roundedRectGeometry.CornerRadius({ 10, 10 });
        roundedRectGeometry.Size(CompositionCard::CardSize);
        auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
        rectShape.StrokeBrush(compositor.CreateColorBrush(winrt::Colors::Gray()));
        rectShape.StrokeThickness(5);
        shapeContainer.Shapes().Append(rectShape);

        winrt::float2 innerSize{ CompositionCard::CardSize / 2.0f };
        auto innerRoundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        innerRoundedRectGeometry.CornerRadius({ 6, 6 });
        innerRoundedRectGeometry.Size(innerSize);
        auto innerRectShape = compositor.CreateSpriteShape(innerRoundedRectGeometry);
        innerRectShape.FillBrush(compositor.CreateColorBrush(winrt::Colors::Gray()));
        innerRectShape.StrokeThickness(5);
        innerRectShape.Offset((CompositionCard::CardSize - innerSize) / 2.0f);
        shapeContainer.Shapes().Append(innerRectShape);

        m_shapeCache.emplace(ShapeType::Empty, shapeContainer);
    }

    m_compositor = compositor;
    m_textHeight = height;
    co_return;
}

std::wstring GetSvgFilePath(Card const& card)
{
    std::wstringstream fileName;
    fileName << L"ms-appx:///Assets/CardFaces/";
    switch (card.Face())
    {
    case Face::Ace:
        fileName << L"A";
        break;
    case Face::Two:
        fileName << L"2";
        break;
    case Face::Three:
        fileName << L"3";
        break;
    case Face::Four:
        fileName << L"4";
        break;
    case Face::Five:
        fileName << L"5";
        break;
    case Face::Six:
        fileName << L"6";
        break;
    case Face::Seven:
        fileName << L"7";
        break;
    case Face::Eight:
        fileName << L"8";
        break;
    case Face::Nine:
        fileName << L"9";
        break;
    case Face::Ten:
        fileName << L"10";
        break;
    case Face::Jack:
        fileName << L"jack";
        break;
    case Face::Queen:
        fileName << L"queen";
        break;
    case Face::King:
        fileName << L"king";
        break;
    }
    fileName << L"_of_";
    switch (card.Suit())
    {
    case Suit::Diamond:
        fileName << L"diamond";
        break;
    case Suit::Spade:
        fileName << L"spades";
        break;
    case Suit::Heart:
        fileName << L"hearts";
        break;
    case Suit::Club:
        fileName << L"clubs";
        break;
    }
    fileName << L".svg";
    return fileName.str();
}