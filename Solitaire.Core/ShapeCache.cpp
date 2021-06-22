#include "pch.h"
#include "ShapeCache.h"
#include "Card.h"
#include "CompositionCard.h"
#include "SvgShapesBuilder.h"

namespace winrt
{
    using namespace Windows;
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Storage;
    using namespace Windows::Storage::Streams;
    using namespace Windows::UI;
    using namespace Windows::UI::Core;
    using namespace Windows::UI::Composition;
}

namespace util
{
    using namespace robmikh::common::uwp;
}

std::wstring GetSvgFilePath(Card const& card);
winrt::CompositionSpriteShape BuildRoundedRectShape(
    winrt::Compositor const& compositor,
    winrt::float2 const& size,
    winrt::float2 const& cornerRadius,
    float strokeThickness,
    winrt::CompositionBrush const& strokeBrush,
    winrt::CompositionBrush const& fillBrush);

std::future<std::shared_ptr<ShapeCache>> ShapeCache::CreateAsync(
    winrt::Compositor const& compositor)
{
    auto cache = std::make_shared<ShapeCache>();
    co_await cache->FillCacheAsync(compositor);
    co_return cache;
}

SvgCompositionShapes ShapeCache::GetCardFace(
    Card const& key)
{
    return m_geometryCache.at(key);
}

winrt::CompositionShape ShapeCache::GetShape(ShapeType shapeType)
{
    return m_shapeCache.at(shapeType);
}

winrt::IAsyncAction ShapeCache::FillCacheAsync(
    winrt::Compositor const& compositor)
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

    auto d3dDevice = util::CreateD3DDevice();
    auto d2dFactory = util::CreateD2DFactory();
    auto d2dDevice = util::CreateD2DDevice(d2dFactory, d3dDevice);
    winrt::com_ptr<ID2D1DeviceContext> d2dContextBase;
    winrt::check_hresult(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dContextBase.put()));
    auto d2dContext = d2dContextBase.as<ID2D1DeviceContext5>();
    m_geometryCache.clear();
    auto height = 34.0f; // TODO: I guess this should be hardcoded now, get the right number later
    for (auto& card : cards)
    {
        auto filePath = GetSvgFilePath(card);
        auto file = co_await winrt::StorageFile::GetFileFromApplicationUriAsync(winrt::Uri(filePath));
        auto stream = co_await file.OpenReadAsync();
        auto istream = util::CreateStreamFromRandomAccessStream(stream);
        D2D1_SIZE_F viewport = D2D1::SizeF(1, 1);
        winrt::com_ptr<ID2D1SvgDocument> document;
        winrt::check_hresult(d2dContext->CreateSvgDocument(istream.get(), viewport, document.put()));
        auto shapeInfo = SvgShapesBuilder::ConvertSvgDocumentToCompositionShapes(compositor, document);

        m_geometryCache.emplace(card, shapeInfo);
    }

    m_shapeCache.clear();
    {
        auto shapeContainer = compositor.CreateContainerShape();
        auto backgroundBaseColor = winrt::Colors::Blue();

        auto rectShape = BuildRoundedRectShape(
            compositor, 
            CompositionCard::CardSize, 
            CompositionCard::CornerRadius,
            0.5f, 
            compositor.CreateColorBrush(winrt::Colors::Black()), 
            compositor.CreateColorBrush(backgroundBaseColor));
        shapeContainer.Shapes().Append(rectShape);

        winrt::float2 innerOffset{ 12, 12 };
        auto innerRectShape = BuildRoundedRectShape(
            compositor,
            CompositionCard::CardSize - innerOffset,
            CompositionCard::CornerRadius,
            5,
            compositor.CreateColorBrush(winrt::Colors::White()),
            compositor.CreateColorBrush(backgroundBaseColor));
        innerRectShape.Offset(innerRectShape.Offset() + (innerOffset / 2.0f));
        shapeContainer.Shapes().Append(innerRectShape);

        m_shapeCache.emplace(ShapeType::Back, shapeContainer);
    }

    {
        auto shapeContainer = compositor.CreateContainerShape();
        auto backgroundBaseColor = winrt::Colors::Blue();

        auto rectShape = BuildRoundedRectShape(
            compositor,
            CompositionCard::CardSize,
            CompositionCard::CornerRadius,
            5,
            compositor.CreateColorBrush(winrt::Colors::Gray()),
            nullptr);
        shapeContainer.Shapes().Append(rectShape);

        winrt::float2 innerSize{ CompositionCard::CardSize / 2.0f };
        auto innerRoundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
        innerRoundedRectGeometry.CornerRadius(CompositionCard::CornerRadius);
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
        fileName << L"ace";
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
        fileName << L"diamonds";
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

winrt::CompositionSpriteShape BuildRoundedRectShape(
    winrt::Compositor const& compositor,
    winrt::float2 const& size,
    winrt::float2 const& cornerRadius,
    float strokeThickness,
    winrt::CompositionBrush const& strokeBrush,
    winrt::CompositionBrush const& fillBrush)
{
    winrt::float2 strokeAddedSize{ strokeThickness, strokeThickness };
    auto strokeOffset = strokeAddedSize / 2.0f;
    auto roundedRectGeometry = compositor.CreateRoundedRectangleGeometry();
    roundedRectGeometry.CornerRadius(cornerRadius);
    roundedRectGeometry.Size(size - strokeAddedSize);
    auto rectShape = compositor.CreateSpriteShape(roundedRectGeometry);
    rectShape.StrokeBrush(strokeBrush);
    rectShape.FillBrush(fillBrush);
    rectShape.StrokeThickness(strokeThickness);
    rectShape.Offset(strokeOffset);
    return rectShape;
}