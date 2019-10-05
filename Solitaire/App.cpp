#include "pch.h"
#include "Card.h"
#include "ShapeCache.h"
#include "CompositionCard.h"
#include "Pack.h"
#include "CardStack.h"
#include "Waste.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

enum class HitTestZone
{
    Deck,
    Waste,
    Foundations,
    PlayArea
};

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
    CompositionTarget m_target{ nullptr };
    ContainerVisual m_root{ nullptr };
    ContainerVisual m_boardLayer{ nullptr };
    ContainerVisual m_selectedLayer{ nullptr };
    VisualCollection m_visuals{ nullptr };
    Visual m_selected{ nullptr };
    float2 m_offset{};

    std::shared_ptr<ShapeCache> m_shapeCache;
    std::unique_ptr<Pack> m_pack;
    std::vector<std::shared_ptr<CardStack>> m_stacks;
    std::map<HitTestZone, Rect> m_zoneRects;
    std::vector<std::shared_ptr<CompositionCard>> m_deck;
    std::unique_ptr<Waste> m_waste;

    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const &)
    {
    }

    void Load(hstring const&)
    {
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

        CoreDispatcher dispatcher = window.Dispatcher();
        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
    }

    void SetWindow(CoreWindow const & window)
    {
        Compositor compositor;
        m_shapeCache = std::make_shared<ShapeCache>(compositor);
        m_root = compositor.CreateContainerVisual();
        m_target = compositor.CreateTargetForCurrentView();
        m_target.Root(m_root);

        m_boardLayer = compositor.CreateContainerVisual();
        m_boardLayer.RelativeSizeAdjustment({ 1, 1 });
        m_root.Children().InsertAtTop(m_boardLayer);

        m_selectedLayer = compositor.CreateContainerVisual();
        m_selectedLayer.RelativeSizeAdjustment({ 1, 1 });
        m_root.Children().InsertAtTop(m_selectedLayer);

        m_visuals = m_boardLayer.Children();

        m_pack = std::make_unique<Pack>(m_shapeCache);
        m_pack->Shuffle();
        auto cards = m_pack->Cards();
        auto textHeight = m_shapeCache->TextHeight();

        const auto cardSize = cards.front()->Root().Size();

        auto playAreaOffsetY = cardSize.y + 25.0f;
        auto cardsSoFar = 0;
        auto numberOfStacks = 7;
        for (int i = 0; i < numberOfStacks; i++)
        {
            auto numberOfCards = i + 1;

            auto start = cards.begin() + cardsSoFar;
            std::vector<std::shared_ptr<CompositionCard>> tempStack(start, start + numberOfCards);
            cardsSoFar += numberOfCards;

            auto stack = std::make_shared<CardStack>(tempStack);
            stack->ForceLayout(textHeight);
            auto baseVisual = stack->Base();

            baseVisual.Offset({ (float)i * (cardSize.x + 15.0f), playAreaOffsetY, 0 });
            m_visuals.InsertAtTop(baseVisual);

            m_stacks.push_back(stack);
        }
        for (auto& stack : m_stacks)
        {
            auto cards = stack->Cards();
            for (auto& card : cards)
            {
                card->IsFaceUp(false);
            }
            cards.back()->IsFaceUp(true);
        }
        m_zoneRects.insert({ HitTestZone::PlayArea, { 0, playAreaOffsetY, window.Bounds().Width, window.Bounds().Height - playAreaOffsetY } });

        std::vector<std::shared_ptr<CompositionCard>> deck(cards.begin() + cardsSoFar, cards.end());
        m_deck = deck;
        for (auto& card : deck)
        {
            auto visual = card->Root();
            visual.Offset({ 0, 0, 0 });
            m_visuals.InsertAtTop(visual);
            card->IsFaceUp(false);
        }
        m_zoneRects.insert({ HitTestZone::Deck, { 0, 0, cardSize.x, cardSize.y } });

        m_waste = std::make_unique<Waste>();
        m_zoneRects.insert({ HitTestZone::Waste, { cardSize.x + 25.0f, 0, cardSize.x, cardSize.y } });

        window.PointerPressed({ this, &App::OnPointerPressed });
        window.PointerMoved({ this, &App::OnPointerMoved });

        window.PointerReleased([&](auto && ...)
        {
            m_selected = nullptr;
        });
    }

    void OnPointerPressed(IInspectable const &, PointerEventArgs const & args)
    {
        float2 const point = args.CurrentPoint().Position();

        for (auto& pair : m_zoneRects)
        {
            auto zoneType = pair.first;
            auto rect = pair.second;

            if (point.x >= rect.X &&
                point.x < rect.X + rect.Width &&
                point.y >= rect.Y &&
                point.y < rect.Y + rect.Height)
            {
                switch (zoneType)
                {
                case HitTestZone::Deck:
                {
                    // Take the top 3 cards
                    auto availableCards = 3;
                    if (m_deck.size() < 3)
                    {
                        availableCards = m_deck.size();
                    }

                    if (availableCards > 0)
                    {
                        auto start = m_deck.begin() + (m_deck.size() - availableCards);
                        auto end = m_deck.end();
                        std::vector<std::shared_ptr<CompositionCard>> tempCards(
                            std::make_move_iterator(start),
                            std::make_move_iterator(end));
                        m_deck.erase(start, end);

                        std::vector<std::shared_ptr<CompositionCard>> cards(tempCards.rbegin(), tempCards.rend());
                        m_waste->AddCards(cards);
                        for (auto& card : cards)
                        {
                            auto visual = card->Root();
                            m_visuals.Remove(visual);
                            m_visuals.InsertAtTop(visual);
                            card->IsFaceUp(true);
                        }
                    }
                    else
                    {
                        auto cards = m_waste->Flush();
                        for (auto& card : cards)
                        {
                            auto visual = card->Root();
                            visual.Offset({ 0, 0, 0 });
                            m_visuals.Remove(visual);
                            m_visuals.InsertAtTop(visual);
                            card->IsFaceUp(false);

                            m_deck.push_back(card);
                        }
                    }
                }
                    break;
                case HitTestZone::PlayArea:
                {
                    auto textHeight = m_shapeCache->TextHeight();
                    for (auto& stack : m_stacks)
                    {
                        const auto baseOffset = stack->Base().Offset();
                        auto cards = stack->Cards();

                        for (int i = cards.size() - 1; i > 0; i--)
                        {
                            auto card = cards[i];

                            auto accumulatedVerticalOffset = textHeight * (i - 1);
                            float2 accumulatedOffset = { baseOffset.x, baseOffset.y + accumulatedVerticalOffset };

                            if (card->HitTest(accumulatedOffset, point))
                            {
                                m_selected = stack->Base();
                            }
                        }

                        if (!cards.empty() && cards.front()->HitTest({ 0, 0 }, point))
                        {
                            m_selected = stack->Base();
                        }

                        if (m_selected)
                        {
                            float3 const offset = m_selected.Offset();
                            m_offset.x = offset.x - point.x;
                            m_offset.y = offset.y - point.y;
                        }
                    }
                }
                    break;
                default:
                    continue;
                }

                break;
            }
        }

        if (m_selected)
        {
            m_visuals.Remove(m_selected);
            m_visuals.InsertAtTop(m_selected);
        }
    }

    void OnPointerMoved(IInspectable const &, PointerEventArgs const & args)
    {
        if (m_selected)
        {
            float2 const point = args.CurrentPoint().Position();

            m_selected.Offset(
            {
                point.x + m_offset.x,
                point.y + m_offset.y,
                0.0f
            });
        }
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
