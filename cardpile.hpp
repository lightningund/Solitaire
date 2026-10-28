#pragma once

#include <vector>
#include <optional>
#include <algorithm>
#include <iostream>

using CardRep = size_t;

// [0, 51] - cards
// 52 - empty
// 53 - face down
constexpr CardRep EMPTY_REP = 52;
constexpr CardRep FACE_DOWN_REP = 53;

struct Card {
	private:
	const std::string EMPTY_STRING = "__";
	const std::string FACE_DOWN_STRING = "[]";

	// ONLY FOR DEBUGGING AND USER FACING
	static std::string get_rank_print(size_t rank) {
		static const std::vector<std::string> RANKS = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "X", "J", "Q", "K"};
		return RANKS[rank];
	}

	// ONLY FOR DEBUGGING AND USER FACING
	static std::string get_suit_print(size_t suit) {
		static const std::vector<std::string> SUITS = {"H", "D", "C", "S"};
		return SUITS[suit];
	}

	// ONLY FOR DEBUGGING AND USER FACING
	std::string get_print() const;

	public:
	CardRep num{EMPTY_REP};
	bool face_down{false};

	Card() {}
	Card(size_t idx) : num{idx}, face_down{false} {}

	size_t rank() const;

	size_t suit() const;

	// Get the number that represents the current state of the card
	CardRep get_rep_num() const;

	bool empty() const;

	Card operator=(const Card& src);

	friend std::ostream& operator<<(std::ostream& os, const Card& card) {
		return os << card.get_print();
	}
};

// Will always show as having at least one element
// If you remove the last real card, a new empty card will be generated
// If you add a card on top of the empty card, it will be replaced
struct CardPile {
	protected:
	inline static const Card EMPTY_CARD{};
	std::vector<Card> cards{EMPTY_CARD};

	public:
	void push(Card c);
	Card pop();

	void clear();

	bool empty() const;

	size_t size() const;

	Card* begin();
	const Card* begin() const;
	const Card* cbegin() const;
	Card* end();
	const Card* end() const;
	const Card* cend() const;

	[[nodiscard]] Card back() const;
	[[nodiscard]] Card& back();

	[[nodiscard]] Card operator[](size_t idx) const;
	[[nodiscard]] Card& operator[](size_t idx);

	friend void move_tail(CardPile& src, CardPile& dst, size_t cutoff) {
		// If the dst has an empty card at the end, get rid of it so it can be replaced
		if (dst.back().empty()) {
			dst.cards.pop_back();
		}

		dst.cards.insert(
			dst.cards.end(),
			std::make_move_iterator(src.cards.begin() + cutoff),
			std::make_move_iterator(src.cards.end())
		);
		src.cards.erase(src.cards.begin() + cutoff, src.cards.end());

		// Make sure to maintain the invariant
		if (src.cards.empty()) {
			src.cards.push_back(EMPTY_CARD);
		}

		// If we copied over no cards and cleared out dst at the start, then we would be left without the empty card
		if (dst.cards.empty()) {
			src.cards.push_back(EMPTY_CARD);
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const CardPile& pile) {
		for (auto&& elem : pile.cards) {
			os << elem << ", ";
		}

		return os;
	}
};

struct Deck : public CardPile {
	// std::vector<Card> cards{};

	// Deck();
	// Deck(std::vector<Card> src) : cards{src} {}

	template <typename RandomGen>
	void shuffle(RandomGen& gen) {
		std::shuffle(cards.begin(), cards.end(), gen);
	}

	std::optional<Card> take_top_card();

	std::optional<Card> view_top_card() const;

	Deck clone() const;

	// friend std::ostream& operator<<(std::ostream& os, const Deck& deck) {
	// 	for (auto&& elem : deck.cards) {
	// 		os << elem << ", ";
	// 	}

	// 	return os;
	// }
};