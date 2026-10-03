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

Card Board::get_flipped_card() const {
	if (!flipped.empty()) {
		return flipped.back();
	}

	return EMPTY_CARD;
}
Card& Board::get_flipped_card() {
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
	idx -= COLS * ROWS;
	if (idx < 4) {
		aces[idx].pop_back();
		if (aces[idx].empty()) {
			aces[idx].push_back(EMPTY_CARD);
		}
	} else {
		flipped.pop_back();
		if (flipped.empty()) {
			flipped.push_back(EMPTY_CARD);
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

// God this is just begging to have actual testing
// Test all the internal sub-functions with toy setups
// Test a whole bunch of premade boards to ensure that the set of legal moves are correct
// I miss rust's testing suite
bool Board::is_legal_move(size_t idx_a, size_t idx_b) const {
	// Legal move types:
		// 1) Opposing suit card on to next higher number:
			// Conditions:
				// Suit colors don't match
				// Moving card has a rank one lower than the Destination card
				// Moving card is face up
				// Destination card doesn't have any other cards below it
				// Destination card is face up
			// Side effects:
				// If the card behind the moving card is face down, flip it up
				// If the moving card has other cards below it, move them too
		// 2) King to empty column:
			// Conditions:
				// Moving card is a King
				// Moving card is face up
				// Destination column is empty
			// Side effects:
				// If the card behind the moving card is face down, flip it up
				// If the moving card has other cards below it, move them too
		// 3) Ace to empty Ace Slot:
			// Conditions:
				// Moving card is an Ace
				// Moving card is face up
				// Ace slot is empty
			// Side effects:
				// If the card behind the moving card is face down, flip it up
		// 4) Card to Ace Slot:
			// Conditions:
				// Suits are the same
				// Moving card has a rank one higher than the Destination card
				// Moving card is face up
				// Moving card has no other cards below it
			// Side effects:
				// If the card behind the moving card is face down, flip it up
		// 5) Turn over card from deck:
			// Conditions:
				// There must be at least one card in the draw deck
			// Side effects:
				// N/A
		// 6) Put flipped pile back into deck:
			// Conditions:
				// The draw deck must be empty
				// There must be at least one card in the flipped pile
			// Side effects:
				// N/A

	// Check order plan:
		// First click on deck:
			// Second idx doesn't matter
			// Can only be 5) or 6)
		// Otherwise:
			// Instant DQs:
				// Moving card is empty
				// Moving card is face down
				// Destination is deck
				// Destination is flip pile
			// If the destination is on the board:
				// Instant DQs:
					// Destination is not empty
				// If moving card is a King:
					// Instant DQs:
						// Destination is not in the top row
				// Otherwise:
					// Instant DQs:
						// Destination is in the top row
						// Card one above the destination is empty
						// Card one above the destination is not the opposite color suit of the moving card
						// Card one above the destination is not one rank higher than the moving card
			// Otherwise (ace destination):
				// Instant DQs:
					// Moving card has cards under it
				// If moving card is an ace:
					// Instant DQs:
						// Destination is not empty
				// Otherwise:
					// Instant DQs:
						// Destination is empty
						// The moving card is not one rank higher than the destination
						// The moving card does not match the suit of the destination

	// Check if the deck can be used, either drawing or reshuffling
	const auto can_click_deck = [this]() { return !(draw_pile.cards.empty() && flipped.empty()); };

	/// Properties of an index itself
	// Checks if the index is the draw pile
	const auto is_deck = [](size_t idx) { return idx == DECK_IDX; };
	// Checks if the index is the flip pile
	const auto is_flip_pile = [](size_t idx) { return idx == FLIP_IDX; };
	// Checks if the index is one of the four ace slots
	const auto is_ace_slot = [](size_t idx) { return idx >= ACE_IDX && idx < BOARD_IDX; };
	// Checks if the index is in the main board of cards
	const auto is_board = [](size_t idx) { return idx >= BOARD_IDX; };
	// Checks if the index is in the top row of the board
	// Meaningless if idx is not on the board
	const auto is_top_row = [](size_t idx) { return (idx - BOARD_IDX) < COLS; };
	// Checks if the index is in the bottom row of the board
	const auto is_bottom_row = [](size_t idx) { return idx > (COLS * (ROWS - 1) + BOARD_IDX); };

	/// Index modifying functions
	// Get the index of the card one above this one
	// Returns the same index if it's in the top row
	// UB if you call it with an index outside of the board
	const auto one_above = [&](size_t idx) {
		if (is_top_row(idx)) return idx;
		return idx - COLS;
	};
	// Get the index of the card one below this one
	// Returns the same index if it's in the bottom row
	// UB if you call it with an index outside of the board
	const auto one_below = [&](size_t idx) {
		if (is_bottom_row(idx)) return idx;
		return idx + COLS;
	};
	// Get the index of the nth ace
	const auto get_ace = [](size_t n) { return ACE_IDX + n; }

	/// Properties of the card at an index
	// Check if the card at an index is empty
	const auto is_empty = [this](size_t idx) { return index(idx).empty(); };
	// Check if the card at an index is face down
	const auto is_face_down = [this](size_t idx) { return index(idx).face_down; };
	// Check if the card at an index is a king
	const auto is_king = [this](size_t idx) { return index(idx).rank() == 12; };
	// Check if the card at an index is an ace
	const auto is_ace = [this](size_t idx) { return index(idx).rank() == 0; };
	// Get the rank of a card at an index
	const auto get_rank = [this](size_t idx) { return index(idx).rank(); };
	// Get the suit of a card at an index
	const auto get_suit = [this](size_t idx) { return index(idx).suit(); };

	// Check if two cards are suits of the same color
	const auto is_same_color = [this](size_t idx_a, size_t idx_b) {
		// We divide by two since the suits are laid out like Heart Diamond Club Spade
		// So the two pairs are next to each other and we can just divide and see if they are in the same "group"
		return (index(idx_a).suit() / 2) == (index(idx_b).suit() / 2);
	};

	if (is_deck(idx_a)) {
		return can_click_deck();
	} else {
		if (is_empty(idx_a)) return false;
		if (is_face_down(idx_a)) return false;
		if (is_deck(idx_b)) return false;
		if (is_flip_pile(idx_b)) return false;

		if (is_board(idx_b)) {
			if (!is_empty(idx_b)) return false;
			if (is_king(idx_a)) {
				if (!is_top_row(idx_b)) return false;
				// ADDITIONAL STIPULATION:
				// If the king is already on the top, also illegal
				// This prevents the AI from moving it back and forth forever
				if (is_top_row(idx_a)) return false;

				return true;
			} else {
				if (is_top_row(idx_b)) return false;
				if (is_empty(one_above(idx_b))) return false;
				if (is_same_color(idx_a, one_above(idx_b))) return false;
				if (get_rank(idx_a) + 1 != get_rank(one_above(idx_b))) return false;

				// ADDITIONAL STIPULATION:
				// If the moving card already had a match behind it,
				//	only allow it if the card behind the moving card has a matching ace
				//	that is also one rank below
				// This prevents the AI from just shuffling them back and forth
				// But still allows shifting them around when you need to get to the ace
				if (get_rank(idx_a) + 1 == get_rank(one_above(idx_a))) {
					auto target_suit = get_suit(one_above(idx_a));
					auto target_rank = get_rank(one_above(idx_a)) - 1;
					bool ace_match = false;
					for (size_t i = 0; i < 4; ++i) {
						if (get_suit(get_ace(i)) == target_suit && get_rank(get_ace(i)) == target_rank) {
							ace_match = true;
							break;
						}
					}

					if (!ace_match) return false;
				}

				return true;
			}
		} else {
			// ADDITIONAL STIPULATION:
			// Prevent moving an ace between slots
			if (is_ace_slot(idx_a)) return false;
			if (is_board(idx_a) && !is_bottom_row(idx_a) && !is_empty(one_below(idx_a))) return false;

			if (is_ace(idx_a)) {
				if (!is_empty(idx_b)) return false;

				return true;
			} else {
				if (is_empty(idx_b)) return false;
				if (get_rank(idx_a) != get_rank(idx_b) + 1) return false;
				if (get_suit(idx_a) != get_suit(idx_b)) return false;

				return true;
			}
		}
	}
}

void Board::perform_move(size_t idx_a, size_t idx_b) {
	// Legal move types:
		// 1) Opposing suit card on to next higher number:
			// Effects:
				// The moving card goes to the indicated slot
				// If the card behind the moving card is face down, flip it up
				// If the moving card has other cards below it, move them too
		// 2) King to empty column:
			// Effects:
				// The moving card goes to the indicated slot
				// If the card behind the moving card is face down, flip it up
				// If the moving card has other cards below it, move them too
		// 3) Ace to empty Ace Slot:
			// Effects:
				// The moving card goes to the indicated slot
				// If the card behind the moving card is face down, flip it up
		// 4) Card to Ace Slot:
			// Effects:
				// The moving card goes to the indicated slot
				// If the card behind the moving card is face down, flip it up
		// 5) Turn over card from deck:
			// Effects:
				// The top card of the draw pile is put into the flip pile
		// 6) Put flipped pile back into deck:
			// Effects:
				// The entire flip pile is put back into the draw pile

	// Check Plan:
		// If the source is the deck
			// Either draw or reset it
		// Move the card to the destination
		// If the source was an ace or the flip pile
			// Pop it off the end
		// If the source was on the board
			// Check if there is a card behind to flip
			// If the destination was also on the board
				// Move any cards below

	// Check if the deck can be used, either drawing or reshuffling
	const auto can_click_deck = [this]() { return !(draw_pile.cards.empty() && flipped.empty()); };

	/// Properties of an index itself
	// Checks if the index is the draw pile
	const auto is_deck = [](size_t idx) { return idx == DECK_IDX; };
	// Checks if the index is the flip pile
	const auto is_flip_pile = [](size_t idx) { return idx == FLIP_IDX; };
	// Checks if the index is one of the four ace slots
	const auto is_ace_slot = [](size_t idx) { return idx >= ACE_IDX && idx < BOARD_IDX; };
	// Checks if the index is in the main board of cards
	const auto is_board = [](size_t idx) { return idx >= BOARD_IDX; };
	// Checks if the index is in the top row of the board
	// Meaningless if idx is not on the board
	const auto is_top_row = [](size_t idx) { return (idx - BOARD_IDX) < COLS; };
	// Checks if the index is in the bottom row of the board
	const auto is_bottom_row = [](size_t idx) { return idx > (COLS * (ROWS - 1) + BOARD_IDX); };

	/// Index modifying functions
	// Get the index of the card one above this one
	// Returns the same index if it's in the top row
	// UB if you call it with an index outside of the board
	const auto one_above = [&](size_t idx) {
		if (is_top_row(idx)) return idx;
		return idx - COLS;
	};
	// Get the index of the card one below this one
	// Returns the same index if it's in the bottom row
	// UB if you call it with an index outside of the board
	const auto one_below = [&](size_t idx) {
		if (is_bottom_row(idx)) return idx;
		return idx + COLS;
	};

	/// Properties of the card at an index
	// Check if the card at an index is empty
	const auto is_empty = [this](size_t idx) { return index(idx).empty(); };
	// Check if the card at an index is face down
	const auto is_face_down = [this](size_t idx) { return index(idx).face_down; };
	// Check if the card at an index is a king
	const auto is_king = [this](size_t idx) { return index(idx).rank() == 12; };
	// Check if the card at an index is an ace
	const auto is_ace = [this](size_t idx) { return index(idx).rank() == 0; };
	// Get the rank of a card at an index
	const auto get_rank = [this](size_t idx) { return index(idx).rank(); };
	// Get the suit of a card at an index
	const auto get_suit = [this](size_t idx) { return index(idx).suit(); };

	// Check if two cards are suits of the same color
	const auto is_same_color = [this](size_t idx_a, size_t idx_b) {
		// We divide by two since the suits are laid out like Heart Diamond Club Spade
		// So the two pairs are next to each other and we can just divide and see if they are in the same "group"
		return (index(idx_a).suit() / 2) == (index(idx_b).suit() / 2);
	};

	if (!is_legal_move(idx_a, idx_b)) return;

	if (is_deck(idx_a)) {
		draw_from_deck();
	} else {
		move_card(idx_a, idx_b);

		if (is_ace_slot(idx_a) || is_flip_pile(idx_a)) {
			pop_card(idx_a);
		} else if (is_board(idx_a)) {
			if (!is_top_row(idx_a) && is_face_down(one_above(idx_a))) {
				flip_card(one_above(idx_a));
			}

			if (is_board(idx_b)) {
				auto temp_a = idx_a;
				auto temp_b = idx_b;
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
