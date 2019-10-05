#pragma once

class CompositionCard;

class CardStack
{
public:
    CardStack(std::vector<std::shared_ptr<CompositionCard>> cards);

    winrt::Windows::UI::Composition::Visual Base() { return !m_stack.empty() ? m_stack.front()->Root() : nullptr; }
    const std::vector<std::shared_ptr<CompositionCard>>& Cards() const { return m_stack; }
    void ForceLayout(float verticalOffset);

private:
    std::vector<std::shared_ptr<CompositionCard>> m_stack;
};