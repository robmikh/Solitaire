#include "pch.h"
#include "Card.h"
#include "ShapeCache.h"
#include "CompositionCard.h"
#include "Pack.h"
#include "CardStack.h"
#include "Waste.h"
#include "Deck.h"
#include "Foundation.h"

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
    ContainerVisual m_foundationVisual{ nullptr };
    ContainerVisual m_deckVisual{ nullptr };
    ContainerVisual m_wasteVisual{ nullptr };
    ContainerVisual m_playAreaVisual{ nullptr };
    ContainerVisual m_selectedLayer{ nullptr };
    VisualCollection m_visuals{ nullptr };
    Visual m_selectedVisual{ nullptr };
    std::vector<std::shared_ptr<CompositionCard>> m_selectedCards;
    std::shared_ptr<CardStack> m_lastStack;
    bool m_isSelectedWasteCard = false;
    int m_lastWasteIndex = -1;
    float2 m_offset{};
    std::shared_ptr<::Foundation> m_lastFoundation;

    std::shared_ptr<ShapeCache> m_shapeCache;
    std::unique_ptr<Pack> m_pack;
    std::vector<std::shared_ptr<CardStack>> m_stacks;
    std::map<HitTestZone, Rect> m_zoneRects;
    std::unique_ptr<Deck> m_deck;
    std::unique_ptr<Waste> m_waste;
    std::vector<std::shared_ptr<::Foundation>> m_foundations;

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
        // Base visual tree
        m_shapeCache = std::make_shared<ShapeCache>(compositor);
        m_root = compositor.CreateContainerVisual();
        m_root.RelativeSizeAdjustment({ 1, 1 });
        m_target = compositor.CreateTargetForCurrentView();
        m_target.Root(m_root);

        m_boardLayer = compositor.CreateContainerVisual();
        m_boardLayer.RelativeSizeAdjustment({ 1, 1 });
        m_root.Children().InsertAtTop(m_boardLayer);

        m_selectedLayer = compositor.CreateContainerVisual();
        m_selectedLayer.RelativeSizeAdjustment({ 1, 1 });
        m_root.Children().InsertAtTop(m_selectedLayer);

        m_visuals = m_boardLayer.Children();

        // Init cards
        m_pack = std::make_unique<Pack>(m_shapeCache);
        m_pack->Shuffle();
        auto cards = m_pack->Cards();
        auto textHeight = m_shapeCache->TextHeight();

        const auto cardSize = CompositionCard::CardSize;

        // Play Area
        auto playAreaOffsetY = cardSize.y + 25.0f;
        m_playAreaVisual = compositor.CreateContainerVisual();
        m_playAreaVisual.Offset({ 0, playAreaOffsetY, 0 });
        m_playAreaVisual.Size({ 0, -playAreaOffsetY });
        m_playAreaVisual.RelativeSizeAdjustment({ 1, 1 });
        m_visuals.InsertAtTop(m_playAreaVisual);
        auto playAreaVisuals = m_playAreaVisual.Children();
        auto cardsSoFar = 0;
        auto numberOfStacks = 7;
        for (int i = 0; i < numberOfStacks; i++)
        {
            auto numberOfCards = i + 1;

            auto start = cards.begin() + cardsSoFar;
            std::vector<std::shared_ptr<CompositionCard>> tempStack(start, start + numberOfCards);
            cardsSoFar += numberOfCards;

            auto stack = std::make_shared<CardStack>(m_shapeCache, tempStack);
            stack->SetLayoutOptions(textHeight);
            stack->ForceLayout();
            auto baseVisual = stack->Base();

            baseVisual.Offset({ (float)i * (cardSize.x + 15.0f), 0, 0 });
            playAreaVisuals.InsertAtTop(baseVisual);

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

        // Deck
        std::vector<std::shared_ptr<CompositionCard>> deck(cards.begin() + cardsSoFar, cards.end());
        m_deck = std::make_unique<Deck>(m_shapeCache, deck);
        m_deck->ForceLayout();
        m_visuals.InsertAtTop(m_deck->Base());
        m_zoneRects.insert({ HitTestZone::Deck, { 0, 0, cardSize.x, cardSize.y } });

        // Waste
        m_waste = std::make_unique<Waste>(m_shapeCache);
        m_waste->ForceLayout(65.0f);
        auto wasteVisual = m_waste->Base();
        wasteVisual.Offset({ cardSize.x + 25.0f, 0, 0 });
        m_visuals.InsertAtTop(wasteVisual);
        m_zoneRects.insert({ HitTestZone::Waste, { cardSize.x + 25.0f, 0, (2.0f * 65.0f) + cardSize.x, cardSize.y } });

        // Foundation
        m_foundationVisual = compositor.CreateContainerVisual();
        m_foundationVisual.Size({ 4.0f * cardSize.x + 3.0f * 15.0f, cardSize.y });
        m_foundationVisual.AnchorPoint({ 1, 0 });
        m_foundationVisual.RelativeOffsetAdjustment({ 1, 0, 0 });
        m_visuals.InsertAtTop(m_foundationVisual);
        for (int i = 0; i < 4; i++)
        {
            auto foundation = std::make_shared<::Foundation>(m_shapeCache);
            auto visual = foundation->Base();
            visual.Offset({i * (cardSize.x + 15.0f), 0, 0 });
            m_foundationVisual.Children().InsertAtTop(visual);
            m_foundations.push_back(foundation);
        }
        m_zoneRects.insert({ HitTestZone::Foundations, { window.Bounds().Width - m_foundationVisual.Size().x, 0, m_foundationVisual.Size().x, m_foundationVisual.Size().y } });

        window.PointerPressed({ this, &App::OnPointerPressed });
        window.PointerMoved({ this, &App::OnPointerMoved });
        window.PointerReleased({ this, &App::OnPointerReleased });
        window.SizeChanged({ this, &App::OnSizeChanged });
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
                    if (m_deck->HitTest(point))
                    {
                        auto cards = m_deck->Draw();

                        if (!cards.empty())
                        {
                            m_waste->AddCards(cards);
                            m_waste->ForceLayout(65.0f);
                        }
                        else
                        {
                            auto wasteCards = m_waste->Flush();
                            m_deck->AddCards(wasteCards);
                        }
                    }
                }
                    break;
                case HitTestZone::PlayArea:
                {
                    // The point is in window space
                    // Convert from window space to play area space
                    auto playAreaPoint = point;
                    auto playAreaHitTestRect = m_zoneRects[HitTestZone::PlayArea];
                    playAreaPoint.x -= playAreaHitTestRect.X;
                    playAreaPoint.y -= playAreaHitTestRect.Y;
                    for (auto& stack : m_stacks)
                    {
                        // The point is in play area space
                        // Convert from play area space to local space for the pile
                        auto localPoint = playAreaPoint;
                        localPoint.x -= stack->Base().Offset().x;
                        localPoint.y -= stack->Base().Offset().y;
                        auto result = stack->HitTest(localPoint);
                        if (result.Target == Pile::HitTestTarget::Card &&
                            stack->CanSplit(result.CardIndex))
                        {
                            m_lastStack = stack;
                            m_selectedCards = stack->Split(result.CardIndex);
                            // The first visual in the list will have its offset updated by 
                            // the pile. However, the pile only knows about its own transforms,
                            // not the play area's. Update the offset so the visual shows up
                            // properly when added to the selection layer.
                            m_selectedVisual = m_selectedCards.front()->Root();
                            auto offset = m_selectedVisual.Offset();
                            offset.x += playAreaHitTestRect.X;
                            offset.y += playAreaHitTestRect.Y;
                            m_selectedVisual.Offset(offset);
                        }
                        auto cards = stack->Cards();

                        if (m_selectedVisual)
                        {
                            float3 const offset = m_selectedVisual.Offset();
                            m_offset.x = offset.x - point.x;
                            m_offset.y = offset.y - point.y;
                            break;
                        }
                    }
                }
                    break;
                case HitTestZone::Waste:
                {
                    auto index = m_waste->HitTest(point);
                    if (index >= 0)
                    {
                        auto card = m_waste->Pick(index);
                        m_selectedVisual = card->Root();
                        m_selectedCards = { card };
                        m_lastStack = nullptr;
                        m_isSelectedWasteCard = true;
                        m_lastWasteIndex = index;
                    }

                    if (m_selectedVisual)
                    {
                        float3 const offset = m_selectedVisual.Offset();
                        m_offset.x = offset.x - point.x;
                        m_offset.y = offset.y - point.y;
                        break;
                    }
                }
                    break;
                case HitTestZone::Foundations:
                {
                    auto foundationPoint = point;
                    auto foundationHitTestRect = m_zoneRects[HitTestZone::Foundations];
                    foundationPoint.x -= foundationHitTestRect.X;
                    foundationPoint.y -= foundationHitTestRect.Y;
                    for (auto& foundation : m_foundations)
                    {
                        auto temp = foundationPoint;
                        temp.x -= foundation->Base().Offset().x;
                        temp.y -= foundation->Base().Offset().y;
                        auto result = foundation->HitTest(temp);
                        if (result.Target == Pile::HitTestTarget::Card)
                        {
                            if (foundation->CanSplit(result.CardIndex))
                            {
                                auto cards = foundation->Split(result.CardIndex);
                                WINRT_ASSERT(cards.size() == 1);
                                auto card = cards.front();
                                m_selectedVisual = card->Root();
                                auto offset = m_selectedVisual.Offset();
                                offset.x += foundationHitTestRect.X;
                                offset.y += foundationHitTestRect.Y;
                                m_selectedVisual.Offset(offset);
                                m_selectedCards = { card };
                                m_lastStack = nullptr;
                                m_isSelectedWasteCard = false;
                                m_lastFoundation = foundation;
                            }

                            break;
                        }
                    }

                    if (m_selectedVisual)
                    {
                        float3 const offset = m_selectedVisual.Offset();
                        m_offset.x = offset.x - point.x;
                        m_offset.y = offset.y - point.y;
                        break;
                    }
                }
                    break;
                default:
                    continue;
                }

                break;
            }
        }

        if (m_selectedVisual)
        {
            m_selectedLayer.Children().InsertAtTop(m_selectedVisual);
        }
    }

    void OnPointerMoved(IInspectable const &, PointerEventArgs const & args)
    {
        if (m_selectedVisual)
        {
            float2 const point = args.CurrentPoint().Position();

            m_selectedVisual.Offset(
            {
                point.x + m_offset.x,
                point.y + m_offset.y,
                0.0f
            });
        }
    }

    void OnPointerReleased(IInspectable const&, PointerEventArgs const& args)
    {
        if (m_selectedVisual)
        {
            float2 const point = args.CurrentPoint().Position();
            m_selectedLayer.Children().RemoveAll();

            std::shared_ptr<CardStack> foundStack;
            auto playAreaPoint = point;
            auto playAreaHitTestRect = m_zoneRects[HitTestZone::PlayArea];
            playAreaPoint.x -= playAreaHitTestRect.X;
            playAreaPoint.y -= playAreaHitTestRect.Y;
            for (auto& stack : m_stacks)
            {
                auto temp = playAreaPoint;
                temp.x -= stack->Base().Offset().x;
                temp.y -= stack->Base().Offset().y;
                auto result = stack->HitTest(temp);
                if (result.Target == Pile::HitTestTarget::Card || 
                    result.Target == Pile::HitTestTarget::Base)
                {
                    foundStack = stack;
                    break;
                }
            }

            std::shared_ptr<::Foundation> foundFoundation;
            auto foundationPoint = point;
            auto foundationHitTestRect = m_zoneRects[HitTestZone::Foundations];
            foundationPoint.x -= foundationHitTestRect.X;
            foundationPoint.y -= foundationHitTestRect.Y;
            for (auto& foundation : m_foundations)
            {
                auto temp = foundationPoint;
                temp.x -= foundation->Base().Offset().x;
                temp.y -= foundation->Base().Offset().y;
                auto result = foundation->HitTest(temp);
                if (result.Target == Pile::HitTestTarget::Card ||
                    result.Target == Pile::HitTestTarget::Base)
                {
                    foundFoundation = foundation;
                    break;
                }
            }

            auto shouldBeInStack = foundStack && foundStack->CanAdd(m_selectedCards);
            auto shouldBeInFoundation = foundFoundation && foundFoundation->CanAdd(m_selectedCards);

            if (shouldBeInStack || shouldBeInFoundation)
            {
                if (shouldBeInStack)
                {
                    foundStack->Add(m_selectedCards);
                }
                else
                {
                    foundFoundation->Add(m_selectedCards);
                }

                // Flip the last card in the old stack
                if (m_lastStack)
                {
                    auto cards = m_lastStack->Cards();
                    if (!cards.empty())
                    {
                        auto card = cards.back();
                        card->IsFaceUp(true);
                    }
                }

                // Remove the card from the waste pile
                if (m_isSelectedWasteCard)
                {
                    m_waste->RemoveCard(m_selectedCards.front());
                    m_waste->ForceLayout(65.0f);
                }
            }
            else if (m_lastStack)
            {
                m_lastStack->Add(m_selectedCards);
            }
            else if (m_isSelectedWasteCard)
            {
                m_waste->InsertCard(m_selectedCards.front(), m_lastWasteIndex);
                m_waste->ForceLayout(65.0f);
            }
            else if (m_lastFoundation)
            {
                m_lastFoundation->Add(m_selectedCards);
                m_lastFoundation->ForceLayout();
            }
            else
            {
                WINRT_ASSERT(false);
            }
        }
        m_selectedVisual = nullptr;
        m_selectedCards.clear();
        m_lastStack = nullptr;
        m_isSelectedWasteCard = false;
        m_lastWasteIndex = -1;
    }

    void App::OnSizeChanged(CoreWindow const& window, WindowSizeChangedEventArgs const& args)
    {
        m_zoneRects[HitTestZone::Foundations] = { window.Bounds().Width - m_foundationVisual.Size().x, 0, m_foundationVisual.Size().x, m_foundationVisual.Size().y };
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(make<App>());
}
