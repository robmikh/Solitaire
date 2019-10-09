#include "pch.h"
#include "Card.h"
#include "CompositionCard.h"
#include "CardStack.h"
#include "Waste.h"
#include "Foundation.h"
#include "Deck.h"
#include "Pack.h"
#include "ShapeCache.h"
#include "Game.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::UI::Popups;
}

template <typename PileType>
std::pair<Pile::HitTestResult, std::shared_ptr<PileType>> HitTestPiles(
    winrt::float2 const point,
    std::map<HitTestZone, winrt::Rect>& zoneRects,
    std::vector<std::shared_ptr<PileType>> const& piles,
    HitTestZone zoneType,
    std::initializer_list<Pile::HitTestTarget> const& desiredTargets)
{
    // The point is in window space
    // Convert from window space to container space (Play Area, Foundation, etc)
    auto containerPoint = point;
    auto containerHitTestRect = zoneRects[zoneType];
    containerPoint.x -= containerHitTestRect.X;
    containerPoint.y -= containerHitTestRect.Y;
    for (auto& pile : piles)
    {
        // The point is in container space
        // Convert from container space to local space for the pile
        auto localPoint = containerPoint;
        localPoint.x -= pile->Base().Offset().x;
        localPoint.y -= pile->Base().Offset().y;
        auto result = pile->HitTest(localPoint);
        for (auto& target : desiredTargets)
        {
            if (result.Target == target)
            {
                return { result, pile };
            }
        }
    }

    return { Pile::HitTestResult(), nullptr };
}

Game::Game(winrt::Compositor const& compositor, winrt::float2 const hostSize)
{
    m_compositor = compositor;
    // Base visual tree
    m_shapeCache = std::make_shared<ShapeCache>(m_compositor);
    m_root = m_compositor.CreateContainerVisual();
    m_root.RelativeSizeAdjustment({ 1, 1 });
    m_root.Comment(L"Game Root");

    m_boardLayer = m_compositor.CreateContainerVisual();
    m_boardLayer.RelativeSizeAdjustment({ 1, 1 });
    m_boardLayer.Comment(L"Board Layer");
    m_root.Children().InsertAtTop(m_boardLayer);

    m_selectedLayer = m_compositor.CreateContainerVisual();
    m_selectedLayer.RelativeSizeAdjustment({ 1, 1 });
    m_selectedLayer.Comment(L"Selection Layer");
    m_root.Children().InsertAtTop(m_selectedLayer);

    m_visuals = m_boardLayer.Children();

    // Get layout info
    auto textHeight = m_shapeCache->TextHeight();
    m_layoutInfo.CardStackVerticalOffset = textHeight;
    const auto cardSize = CompositionCard::CardSize;

    // Play Area
    auto playAreaOffsetY = cardSize.y + 25.0f;
    m_playAreaVisual = m_compositor.CreateContainerVisual();
    m_playAreaVisual.Offset({ 0, playAreaOffsetY, 0 });
    m_playAreaVisual.Size({ 0, -playAreaOffsetY });
    m_playAreaVisual.RelativeSizeAdjustment({ 1, 1 });
    m_playAreaVisual.Comment(L"Play Area Root");
    m_visuals.InsertAtTop(m_playAreaVisual);
    m_zoneRects.insert({ HitTestZone::PlayArea, { 0, playAreaOffsetY, hostSize.x, hostSize.y - playAreaOffsetY } });

    // Deck
    m_deckVisual = m_compositor.CreateContainerVisual();
    m_deckVisual.Size(CompositionCard::CardSize);
    m_deckVisual.Comment(L"Deck Area Root");
    m_visuals.InsertAtTop(m_deckVisual);
    m_zoneRects.insert({ HitTestZone::Deck, { 0, 0, cardSize.x, cardSize.y } });

    // Waste
    m_wasteVisual = m_compositor.CreateContainerVisual();
    m_wasteVisual.Size({ (2.0f * m_layoutInfo.WasteHorizontalOffset) + cardSize.x, cardSize.y });
    m_wasteVisual.Offset({ cardSize.x + 25.0f, 0, 0 });
    m_wasteVisual.Comment(L"Waste Area Root");
    m_visuals.InsertAtTop(m_wasteVisual);
    m_zoneRects.insert({ HitTestZone::Waste, { cardSize.x + 25.0f, 0, (2.0f * m_layoutInfo.WasteHorizontalOffset) + cardSize.x, cardSize.y } });

    // Foundation
    m_foundationVisual = m_compositor.CreateContainerVisual();
    m_foundationVisual.Size({ 4.0f * cardSize.x + 3.0f * 15.0f, cardSize.y });
    m_foundationVisual.AnchorPoint({ 1, 0 });
    m_foundationVisual.RelativeOffsetAdjustment({ 1, 0, 0 });
    m_foundationVisual.Comment(L"Foundations Root");
    m_visuals.InsertAtTop(m_foundationVisual);
    m_zoneRects.insert({ HitTestZone::Foundations, { hostSize.x - m_foundationVisual.Size().x, 0, m_foundationVisual.Size().x, m_foundationVisual.Size().y } });

    NewGame();
}

void Game::NewGame()
{
    m_pack = std::make_unique<Pack>(m_shapeCache);
#ifdef _DEBUG
    m_pack->Shuffle({ 1318857190, 1541316502, 3202618166, 965450609 });
    //m_pack->Shuffle();
#else
    m_pack->Shuffle();
#endif
    auto cards = m_pack->Cards();

    auto [stacks, numCardsUsed] = ConstructStacks(cards);
    m_stacks = stacks;
    m_deck = ConstructDeck(cards, numCardsUsed);
    m_waste = ConstructWaste();
    m_foundations = ConstructFoundations();

    m_selectedLayer.Children().RemoveAll();
    m_selectedVisual = nullptr;
    m_selectedCards.clear();
    m_selectedItemContainers.clear();
    m_lastOperation = Pile::RemovalOperation();
    m_lastPile = nullptr;
    m_lastHitTest = Pile::HitTestResult();
}

void Game::OnPointerPressed(winrt::float2 const point)
{
    if (IsAnimating())
    {
        return;
    }

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
                        // Compute difference between the two zones
                        auto deckZoneRect = m_zoneRects[HitTestZone::Deck];
                        auto wasteZoneRect = m_zoneRects[HitTestZone::Waste];
                        auto dX = wasteZoneRect.X - deckZoneRect.X;
                        auto dy = wasteZoneRect.Y - deckZoneRect.Y;

                        auto batch = m_compositor.CreateScopedBatch(winrt::CompositionBatchTypes::Animation);

                        auto count = 0;
                        for (auto& card : cards)
                        {
                            auto visual = card->Root();
                            m_visuals.InsertAtTop(visual);

                            auto duration = std::chrono::milliseconds(250);
                            auto delayTime = std::chrono::milliseconds(50 * count);

                            // TODO: Sync this up with the deck visual's actual position (transform parent?)
                            auto xAnimation = m_compositor.CreateScalarKeyFrameAnimation();
                            xAnimation.InsertKeyFrame(0, 0);
                            xAnimation.InsertKeyFrame(1, CompositionCard::CardSize.x + 25.0f + count * m_layoutInfo.WasteHorizontalOffset);
                            xAnimation.IterationBehavior(winrt::AnimationIterationBehavior::Count);
                            xAnimation.IterationCount(1);
                            xAnimation.Duration(duration);
                            xAnimation.DelayTime(delayTime);
                            visual.StartAnimation(L"Offset.X", xAnimation);

                            auto zAnimation = m_compositor.CreateScalarKeyFrameAnimation();
                            zAnimation.InsertKeyFrame(0, 0);
                            zAnimation.InsertKeyFrame(0.5f, 10.0f);
                            zAnimation.InsertKeyFrame(1, 0);
                            zAnimation.IterationBehavior(winrt::AnimationIterationBehavior::Count);
                            zAnimation.IterationCount(1);
                            zAnimation.Duration(duration);
                            zAnimation.DelayTime(delayTime);
                            visual.StartAnimation(L"Offset.Z", zAnimation);

                            card->AnimateIsFaceUp(true, duration, delayTime);

                            count++;
                        }

                        batch.Completed([=](auto&& ...)
                            {
                                for (auto& card : cards)
                                {
                                    m_visuals.Remove(card->Root());
                                }
                                m_waste->Discard(cards);
                                m_isDeckAnimationRunning = false;
                            });
                        m_isDeckAnimationRunning = true;
                        batch.End();
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
            case HitTestZone::Foundations:
            case HitTestZone::Waste:
            {
                auto containerHitTestRect = m_zoneRects[zoneType];
                auto [foundPile, hitTestResult, hitTestZone] = HitTestPiles(point, { Pile::HitTestTarget::Card });
                if (foundPile)
                {
                    auto canSplit = foundPile->CanSplit(hitTestResult.CardIndex);
                    auto canTake = foundPile->CanTake(hitTestResult.CardIndex);

                    if (canSplit || canTake)
                    {
                        m_lastPile = foundPile;
                        if (canSplit)
                        {
                            auto [containers, cards, operation] = foundPile->Split(hitTestResult.CardIndex);
                            m_selectedItemContainers = containers;
                            m_selectedCards = cards;
                            m_lastOperation = operation;
                        }
                        else if (canTake)
                        {
                            auto [container, card, operation] = foundPile->Take(hitTestResult.CardIndex);
                            m_selectedItemContainers = { container };
                            m_selectedCards = { card };
                            m_lastOperation = operation;
                        }
                        m_selectedVisual = m_selectedItemContainers.front().Root;
                        m_lastHitTest = hitTestResult;
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

    if (m_selectedVisual)
    {
        m_selectedLayer.Children().InsertAtTop(m_selectedVisual);
        winrt::float3 const offset = m_selectedVisual.Offset();
        m_offset.x = offset.x - point.x;
        m_offset.y = offset.y - point.y;
    }
}

void Game::OnPointerMoved(winrt::float2 const point)
{
    if (m_selectedVisual)
    {
        m_selectedVisual.Offset(
            {
                point.x + m_offset.x,
                point.y + m_offset.y,
                0.0f
            });
    }
}

void Game::OnPointerReleased(winrt::float2 const point)
{
    if (IsAnimating())
    {
        return;
    }

    if (m_selectedVisual)
    {
        m_selectedLayer.Children().RemoveAll();

        auto [foundPile, hitTestResult, hitTestZone] = HitTestPiles(point, { Pile::HitTestTarget::Card, Pile::HitTestTarget::Base });
        auto shouldBeInPile = foundPile && foundPile->CanAdd(m_selectedCards);

        for (auto& container : m_selectedItemContainers)
        {
            container.Content.Children().RemoveAll();
        }

        if (shouldBeInPile)
        {
            foundPile->Add(m_selectedCards);

            if (m_lastPile)
            {
                m_lastPile->CompleteRemoval(m_lastOperation);
            }

            // If we just added something to a foundation, let's check to see
            // if the player has won.
            if (hitTestZone == HitTestZone::Foundations)
            {
                auto hasWon = true;
                for (auto& foundation : m_foundations)
                {
                    auto cards = foundation->Cards();
                    if (cards.size() != 13)
                    {
                        hasWon = false;
                        break;
                    }
                }

                if (hasWon)
                {
                    DisplayWinMessage();
                }
            }
        }
        else if (m_lastPile)
        {
            m_lastPile->Return(m_selectedCards, m_lastOperation);
        }
        else
        {
            WINRT_ASSERT(false);
        }
    }
    m_selectedVisual = nullptr;
    m_selectedCards.clear();
    m_selectedItemContainers.clear();
    m_lastOperation = Pile::RemovalOperation();
    m_lastPile = nullptr;
    m_lastHitTest = Pile::HitTestResult();
}

void Game::OnSizeChanged(winrt::float2 const size)
{
    auto playAreaOffsetY = m_playAreaVisual.Offset().y;
    m_zoneRects.insert({ HitTestZone::PlayArea, { 0, playAreaOffsetY, size.x, size.y - playAreaOffsetY } });
    m_zoneRects[HitTestZone::Foundations] = { size.x - m_foundationVisual.Size().x, 0, m_foundationVisual.Size().x, m_foundationVisual.Size().y };
}

std::pair<std::vector<std::shared_ptr<CardStack>>, int> Game::ConstructStacks(Pile::CardList const& cards)
{
    const auto cardSize = CompositionCard::CardSize;

    std::vector<std::shared_ptr<CardStack>> stacks;
    auto playAreaVisuals = m_playAreaVisual.Children();
    playAreaVisuals.RemoveAll();
    auto cardsSoFar = 0;
    auto numberOfStacks = 7;
    for (int i = 0; i < numberOfStacks; i++)
    {
        auto numberOfCards = i + 1;

        auto start = cards.begin() + cardsSoFar;
        std::vector<std::shared_ptr<CompositionCard>> tempStack(start, start + numberOfCards);
        cardsSoFar += numberOfCards;

        auto stack = std::make_shared<CardStack>(m_shapeCache, tempStack);
        stack->SetLayoutOptions(m_layoutInfo.CardStackVerticalOffset);
        stack->ForceLayout();
        auto baseVisual = stack->Base();

        baseVisual.Offset({ (float)i * (cardSize.x + 15.0f), 0, 0 });
        playAreaVisuals.InsertAtTop(baseVisual);

        stacks.push_back(stack);
    }
    for (auto& stack : stacks)
    {
        auto cards = stack->Cards();
        for (auto& card : cards)
        {
            card->IsFaceUp(false);
        }
        cards.back()->IsFaceUp(true);
    }
    return { stacks, cardsSoFar };
}

std::unique_ptr<Deck> Game::ConstructDeck(Pile::CardList const& cards, int startAt)
{
    std::vector<std::shared_ptr<CompositionCard>> deck(cards.begin() + startAt, cards.end());
    auto result = std::make_unique<Deck>(m_shapeCache, deck);
    result->ForceLayout();
    m_deckVisual.Children().RemoveAll();
    m_deckVisual.Children().InsertAtTop(result->Base());
    return result;
}

std::shared_ptr<Waste> Game::ConstructWaste()
{
    auto waste = std::make_shared<Waste>(m_shapeCache);
    waste->SetLayoutOptions(m_layoutInfo.WasteHorizontalOffset);
    waste->ForceLayout();
    m_wasteVisual.Children().RemoveAll();
    m_wasteVisual.Children().InsertAtTop(waste->Base());
    return waste;
}

std::vector<std::shared_ptr<::Foundation>> Game::ConstructFoundations()
{
    const auto cardSize = CompositionCard::CardSize;

    std::vector<std::shared_ptr<::Foundation>> foundations;
    m_foundationVisual.Children().RemoveAll();
    for (int i = 0; i < 4; i++)
    {
        auto foundation = std::make_shared<::Foundation>(m_shapeCache);
        auto visual = foundation->Base();
        visual.Offset({ i * (cardSize.x + 15.0f), 0, 0 });
        m_foundationVisual.Children().InsertAtTop(visual);
        foundations.push_back(foundation);
    }
    return foundations;
}

winrt::fire_and_forget Game::DisplayWinMessage()
{
    auto dialog = winrt::MessageDialog(L"You won!");
    co_await dialog.ShowAsync();
}

void Game::SetNewLayout(LayoutInformation layoutInfo)
{
    m_layoutInfo = layoutInfo;
    m_waste->SetLayoutOptions(m_layoutInfo.WasteHorizontalOffset);
    m_waste->ForceLayout();
    for (auto& stack : m_stacks)
    {
        stack->SetLayoutOptions(m_layoutInfo.CardStackVerticalOffset);
        stack->ForceLayout();
    }
    auto cardSize = CompositionCard::CardSize;
    m_zoneRects[HitTestZone::Waste] = { cardSize.x + 25.0f, 0, (2.0f * m_layoutInfo.WasteHorizontalOffset) + cardSize.x, cardSize.y };
}

std::tuple<std::shared_ptr<Pile>, Pile::HitTestResult, HitTestZone> Game::HitTestPiles(
    winrt::float2 const point,
    std::initializer_list<Pile::HitTestTarget> const& desiredTargets)
{

    {
        auto [result, stack] = ::HitTestPiles(point, m_zoneRects, m_stacks, HitTestZone::PlayArea, desiredTargets);
        if (result.Target != Pile::HitTestTarget::None)
        {
            return { stack, result, HitTestZone::PlayArea };
        }
    }

    {
        auto [result, foundation] = ::HitTestPiles(point, m_zoneRects, m_foundations, HitTestZone::Foundations, desiredTargets);
        if (result.Target != Pile::HitTestTarget::None)
        {
            return { foundation, result, HitTestZone::Foundations };
        }
    }

    {
        auto [result, waste] = ::HitTestPiles<Waste>(point, m_zoneRects, { m_waste }, HitTestZone::Waste, desiredTargets);
        if (result.Target != Pile::HitTestTarget::None)
        {
            return { waste, result, HitTestZone::Waste };
        }
    }

    return { nullptr, Pile::HitTestResult(), HitTestZone::None };
}