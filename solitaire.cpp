#include <array>
#include <vector>
#include <algorithm>
#include <numeric>
#include <optional>

#include "solitaire.hpp"

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

Card Board::get_board_card(size_t idx) const {
	return main_board[idx / COLS][idx % COLS];
}
Card& Board::get_board_card(size_t idx) {
	return main_board[idx / COLS][idx % COLS];
}

Card Board::get_ace_card(size_t idx) const {
	return aces[idx].back();
}
Card& Board::get_ace_card(size_t idx) {
	return aces[idx].back();
}

Card Board::get_deck_card() const {
	if (auto card = draw_pile.view_top_card()) {
		return card.value();
	}

	return EMPTY_CARD;
}
Card& Board::get_deck_card() {
	return draw_pile.cards.back();
}

Card Board::get_flip_card() const {
	if (!flipped.empty()) {
		return flipped.back();
	}

	return EMPTY_CARD;
}
Card& Board::get_flip_card() {
	return flipped.back();
}

void Board::draw_from_deck() {
	if (auto card = draw_pile.take_top_card()) {
		flipped.push_back(card.value());
	} else {
		for (auto card : flipped) {
			if (card.empty()) continue;
			draw_pile.cards.push_back(card);
		}
		flipped.clear();
	}
}

void Board::flip_card(size_t idx) {
	index(idx).face_down = false;
}

void Board::pop_card(size_t idx) {
	if (idx == FLIP_IDX) {
		flipped.pop_back();
		if (flipped.empty()) {
			flipped.push_back(EMPTY_CARD);
		}
	} else {
		idx -= ACE_IDX;
		aces[idx].pop_back();
		if (aces[idx].empty()) {
			aces[idx].push_back(EMPTY_CARD);
		}
	}
}

void Board::move_card(size_t src, size_t dst) {
	index(dst) = index(src);
	index(src) = EMPTY_CARD;
}

void Board::deal(Deck& deck) {
	draw_pile = deck.clone();
	for (size_t row = 0; row < COLS; ++row) {
		for (size_t col = row; col < COLS; ++col) {
			if (auto card = draw_pile.take_top_card()) {
				main_board[row][col] = card.value();
				main_board[row][col].face_down = (row != col);
			}
		}
	}
}

Card Board::index(size_t idx) const {
	if (idx == DECK_IDX) {
		return get_deck_card();
	}
	if (idx == FLIP_IDX) {
		return get_flip_card();
	}
	if (idx < BOARD_IDX) {
		return get_ace_card(idx - ACE_IDX);
	}

	return get_board_card(idx - BOARD_IDX);
}
Card Board::cindex(size_t idx) const {
	return index(idx);
}
Card& Board::index(size_t idx) {
	if (idx == DECK_IDX) {
		return get_deck_card();
	}
	if (idx == FLIP_IDX) {
		return get_flip_card();
	}
	if (idx < BOARD_IDX) {
		return get_ace_card(idx - ACE_IDX);
	}

	return get_board_card(idx - BOARD_IDX);
}

// Check if the deck can be used, either drawing or reshuffling
bool Board::can_click_deck() const { return !(draw_pile.cards.empty() && flipped.empty()); };

/// Properties of an index itself
// Checks if the index is the draw pile
bool Board::is_deck(size_t idx) const { return idx == DECK_IDX; };
// Checks if the index is the flip pile
bool Board::is_flip_pile(size_t idx) const { return idx == FLIP_IDX; };
// Checks if the index is one of the four ace slots
bool Board::is_ace_slot(size_t idx) const { return idx >= ACE_IDX && idx < BOARD_IDX; };
// Checks if the index is in the main board of cards
bool Board::is_board(size_t idx) const { return idx >= BOARD_IDX; };
// Checks if the index is in the top row of the board
// Meaningless if idx is not on the board
bool Board::is_top_row(size_t idx) const { return idx < COLS + BOARD_IDX; };
// Checks if the index is in the bottom row of the board
bool Board::is_bottom_row(size_t idx) const { return idx > (COLS * (ROWS - 1) + BOARD_IDX); };

/// Index modifying functions
// Get the index of the card one above this one
// Returns the same index if it's in the top row
// UB if you call it with an index outside of the board
size_t Board::one_above(size_t idx) const {
	if (is_top_row(idx)) return idx;
	return idx - COLS;
};
// Get the index of the card one below this one
// Returns the same index if it's in the bottom row
// UB if you call it with an index outside of the board
size_t Board::one_below(size_t idx) const {
	if (is_bottom_row(idx)) return idx;
	return idx + COLS;
};
// Get the index of the nth ace
size_t Board::get_ace(size_t n) const { return ACE_IDX + n; };

/// Properties of the card at an index
// Check if the card at an index is empty
bool Board::is_empty(size_t idx) const { return index(idx).empty(); };
// Check if the card at an index is face down
bool Board::is_face_down(size_t idx) const { return index(idx).face_down; };
// Check if the card at an index is a king
bool Board::is_king(size_t idx) const { return index(idx).rank() == 12; };
// Check if the card at an index is an ace
bool Board::is_ace(size_t idx) const { return index(idx).rank() == 0; };
// Get the rank of a card at an index
size_t Board::get_rank(size_t idx) const { return index(idx).rank(); };
// Get the suit of a card at an index
size_t Board::get_suit(size_t idx) const { return index(idx).suit(); };

// Check if two cards are suits of the same color
bool Board::is_same_color(size_t idx_a, size_t idx_b) const {
	// We divide by two since the suits are laid out like Heart Diamond Club Spade
	// So the two pairs are next to each other and we can just divide and see if they are in the same "group"
	return (index(idx_a).suit() / 2) == (index(idx_b).suit() / 2);
};

// God this is just begging to have actual testing
// Test all the internal sub-functions with toy setups
// Test a whole bunch of premade boards to ensure that the set of legal moves are correct
// I miss rust's testing suite

/// Takes the two columns as parameters
bool Board::is_legal_move(size_t src, size_t dst) const {
	if (is_deck(src)) {
		return can_click_deck();
	}

	if (is_board(dst)) {
		const auto target = get_bottom_column_card(dst);

		if (is_board(src)) {
			// TODO
		}
	}

	// if (is_deck(src)) {
	// 	return can_click_deck();
	// } else {
	// 	if (is_empty(src)) return false;
	// 	if (is_deck(dst)) return false;
	// 	if (is_flip_pile(dst)) return false;

	// 	if (is_board(dst)) {
	// 		if (!is_empty(dst)) return false;
	// 		if (is_king(src)) {
	// 			if (!is_top_row(dst)) return false;
	// 			// ADDITIONAL STIPULATION:
	// 			// If the king is already on the top, also illegal
	// 			// This prevents the AI from moving it back and forth forever
	// 			if (is_board(src) && is_top_row(src)) return false;

	// 			return true;
	// 		} else {
	// 			if (is_top_row(dst)) return false;
	// 			if (is_empty(one_above(dst))) return false;
	// 			if (is_same_color(src, one_above(dst))) return false;
	// 			if (get_rank(src) + 1 != get_rank(one_above(dst))) return false;

	// 			// ADDITIONAL STIPULATION:
	// 			// If the moving card already had a match behind it,
	// 			//	only allow it if the card behind the moving card has a matching ace
	// 			//	that is also one rank below
	// 			// This prevents the AI from just shuffling them back and forth
	// 			// But still allows shifting them around when you need to get to the ace
	// 			if (is_board(src) && get_rank(src) + 1 == get_rank(one_above(src))) {
	// 				auto target_suit = get_suit(one_above(src));
	// 				auto target_rank = get_rank(one_above(src)) - 1;
	// 				bool ace_match = false;
	// 				for (size_t i = 0; i < 4; ++i) {
	// 					if (get_suit(get_ace(i)) == target_suit && get_rank(get_ace(i)) == target_rank) {
	// 						ace_match = true;
	// 						break;
	// 					}
	// 				}

	// 				if (!ace_match) return false;
	// 			}

	// 			return true;
	// 		}
	// 	} else {
	// 		// ADDITIONAL STIPULATION:
	// 		// Prevent moving an ace between slots
	// 		if (is_ace_slot(src)) return false;
	// 		if (is_board(src) && !is_bottom_row(src) && !is_empty(one_below(src))) return false;

	// 		if (is_ace(src)) {
	// 			if (!is_empty(dst)) return false;

	// 			return true;
	// 		} else {
	// 			if (is_empty(dst)) return false;
	// 			if (get_rank(src) != get_rank(dst) + 1) return false;
	// 			if (get_suit(src) != get_suit(dst)) return false;

	// 			return true;
	// 		}
	// 	}
	// }
}

void Board::perform_move(size_t src, size_t dst) {
	if (!is_legal_move(src, dst)) return;

	if (is_deck(src)) {
		draw_from_deck();
	} else {
		move_card(src, dst);

		if (is_ace_slot(src) || is_flip_pile(src)) {
			pop_card(src);
		} else if (is_board(src)) {
			if (!is_top_row(src) && is_face_down(one_above(src))) {
				flip_card(one_above(src));
			}

			if (is_board(dst)) {
				auto temp_a = src;
				auto temp_b = dst;
				while (!is_bottom_row(temp_a)) {
					temp_a = one_below(temp_a);
					temp_b = one_below(temp_b);

					if (is_empty(temp_a)) break;

					move_card(temp_a, temp_b);
				}
			}
		}
	}
}

bool Board::has_won() const {
	for (size_t i = 0; i < 4; ++i) {
		if (get_ace_card(i).rank() != 12) return false;
	}

	return true;
}

size_t Board::num_ace_cards() const {
	size_t num = 0;
	for (size_t i = 0; i < 4; ++i) {
		num += aces[i].size() - 1;
	}

	return num;
}

size_t Board::num_face_up() const {
	size_t num = 0;
	for (auto&& row : main_board) {
		for (auto&& card : row) {
			if (!card.empty() && !card.face_down) ++num;
		}
	}

	num += flipped.size() - 1;

	return num;
}

double Board::score() const {
	static const double win_bonus = 1e6;
	static const double ace_score = 100.0;
	static const double face_up_score = 1.0;

	if (has_won()) {
		// TODO: subtract number of turns
		return win_bonus;
	} else {
		return num_ace_cards() * ace_score + num_face_up() * face_up_score;
	}
}