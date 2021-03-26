#pragma once

enum class Face
{
    Ace = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
    Nine = 9,
    Ten = 10,
    Jack = 11,
    Queen = 12,
    King = 13
};

enum class Suit
{
    Diamond = 0,
    Spade = 1,
    Heart = 2,
    Club = 3
};

struct Card
{
    Card() : m_face(Face::Ace), m_suit(Suit::Diamond) {}
    Card(Face face, Suit suit) : m_face(face), m_suit(suit) {}
    ~Card() {}

    Face Face() const { return m_face; }
    Suit Suit() const { return m_suit; }
    winrt::hstring ToString() const
    {
        winrt::hstring result;
        switch (m_face)
        {
        case Face::Ace:
            result = L"A";
            break;
        case Face::Two:
            result = L"2";
            break;
        case Face::Three:
            result = L"3";
            break;
        case Face::Four:
            result = L"4";
            break;
        case Face::Five:
            result = L"5";
            break;
        case Face::Six:
            result = L"6";
            break;
        case Face::Seven:
            result = L"7";
            break;
        case Face::Eight:
            result = L"8";
            break;
        case Face::Nine:
            result = L"9";
            break;
        case Face::Ten:
            result = L"10";
            break;
        case Face::Jack:
            result = L"J";
            break;
        case Face::Queen:
            result = L"Q";
            break;
        case Face::King:
            result = L"K";
            break;
        }
        switch (m_suit)
        {
        case Suit::Diamond:
            result = result + L"♦";
            break;
        case Suit::Spade:
            result = result + L"♠";
            break;
        case Suit::Heart:
            result = result + L"♥";
            break;
        case Suit::Club:
            result = result + L"♣";
            break;
        }
        return result;
    }
    bool IsRed() const { return (int)m_suit % 2 == 0; }

    bool operator==(Card const& other) const { return m_face == other.m_face && m_suit == other.m_suit; }
    bool operator<(Card const& other) const
    {
        return m_suit == other.m_suit ? m_face < other.m_face : m_suit < other.m_suit;
    }

private:
    ::Face m_face;
    ::Suit m_suit;
};