#pragma once
#include "Pile.h"

struct LayoutInformation
{
    float CardStackVerticalOffset = 47.88f;
    float WasteHorizontalOffset = 65.0f;
};

enum class HitTestZone
{
    None,
    Deck,
    Waste,
    Foundations,
    PlayArea
};

class Game
{
public:
    Game(winrt::Windows::UI::Composition::Compositor const& compositor, winrt::Windows::Foundation::Numerics::float2 const hostSize);

    winrt::Windows::UI::Composition::Visual Root() { return m_root; }

    void NewGame();
    void OnPointerPressed(winrt::Windows::Foundation::Numerics::float2 const point);
    void OnPointerMoved(winrt::Windows::Foundation::Numerics::float2 const point);
    void OnPointerReleased(winrt::Windows::Foundation::Numerics::float2 const point);
    void OnSizeChanged(winrt::Windows::Foundation::Numerics::float2 const size);

    bool IsAnimating() { return m_isDeckAnimationRunning; }

    // TODO: Remove these
    LayoutInformation LayoutInfo() { return m_layoutInfo; }
    void LayoutInfo(LayoutInformation layoutInfo)
    {
        SetNewLayout(layoutInfo);
    }

private:
    std::pair<std::vector<std::shared_ptr<CardStack>>, int> ConstructStacks(Pile::CardList const& cards);
    std::unique_ptr<Deck> ConstructDeck(Pile::CardList const& cards, int startAt);
    std::shared_ptr<Waste> ConstructWaste();
    std::vector<std::shared_ptr<::Foundation>> ConstructFoundations();
    winrt::fire_and_forget DisplayWinMessage();
    void SetNewLayout(LayoutInformation layoutInfo);
    std::tuple<std::shared_ptr<Pile>, Pile::HitTestResult, HitTestZone> HitTestPiles(
        winrt::Windows::Foundation::Numerics::float2 const point,
        std::initializer_list<Pile::HitTestTarget> const& desiredTargets);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_root{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_boardLayer{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_foundationVisual{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_deckVisual{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_wasteVisual{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_playAreaVisual{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_selectedLayer{ nullptr };
    winrt::Windows::UI::Composition::VisualCollection m_visuals{ nullptr };

    winrt::Windows::UI::Composition::Visual m_selectedVisual{ nullptr };
    Pile::CardList m_selectedCards;
    Pile::ItemContainerList m_selectedItemContainers;
    Pile::RemovalOperation m_lastOperation;
    std::shared_ptr<Pile> m_lastPile;
    Pile::HitTestResult m_lastHitTest;
    winrt::Windows::Foundation::Numerics::float2 m_offset{};

    bool m_isDeckAnimationRunning = false;
    LayoutInformation m_layoutInfo{};

    std::shared_ptr<ShapeCache> m_shapeCache;
    std::unique_ptr<Pack> m_pack;
    std::vector<std::shared_ptr<CardStack>> m_stacks;
    std::map<HitTestZone, winrt::Windows::Foundation::Rect> m_zoneRects;
    std::unique_ptr<Deck> m_deck;
    std::shared_ptr<Waste> m_waste;
    std::vector<std::shared_ptr<::Foundation>> m_foundations;
};