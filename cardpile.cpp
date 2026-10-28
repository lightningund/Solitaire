#include "cardpile.hpp"
#include <algorithm>
#include <numeric>

// ONLY FOR DEBUGGING AND USER FACING
std::string Card::get_print() const {
	if (face_down) return FACE_DOWN_STRING;
	if (num == EMPTY_REP) return EMPTY_STRING;

	return get_rank_print(rank()) + get_suit_print(suit());
}

size_t Card::rank() const {
	return num % 13;
}

size_t Card::suit() const {
	return num / 13;
}

// Get the number that represents the current state of the card
CardRep Card::get_rep_num() const {
	return face_down ? FACE_DOWN_REP : num;
}

bool Card::empty() const {
	return num == EMPTY_REP;
}

Card Card::operator=(const Card& src) {
	num = src.num;
	face_down = src.face_down;
	return *this;
}

Deck::Deck() {
	cards.resize(52);
	std::iota(cards.begin(), cards.end(), 0);
}

std::optional<Card> Deck::take_top_card() {
	std::optional<Card> top_card{};
	if (!cards.empty()) {
		top_card = cards[cards.size() - 1];
		cards.pop_back();
	}

	return top_card;
}

std::optional<Card> Deck::view_top_card() const {
	std::optional<Card> top_card{};
	if (!cards.empty()) {
		top_card = cards[cards.size() - 1];
	}

	return top_card;
}

Deck Deck::clone() const {
	std::vector<Card> dest{};
	dest.resize(cards.size());
	std::copy(cards.begin(), cards.end(), dest.begin());
	return dest;
}