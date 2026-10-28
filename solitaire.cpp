#include <array>
#include <optional>

#include "solitaire.hpp"

void Board::draw_from_deck() {
	if (auto card = draw_pile.take_top_card()) {
		flipped.push(card.value());
	} else {
		for (auto card : flipped) {
			draw_pile.push(card);
		}
		flipped.clear();
	}
}

void Board::move_card(size_t src, size_t dst) {
	auto temp = column(src).pop();
	column(dst).push(temp);
}

void Board::deal(Deck& deck) {
	draw_pile = deck.clone();
	for (size_t col = 0; col < COLS; ++col) {
		for (size_t row = 0; row <= col; ++row) {
			if (auto card = draw_pile.take_top_card()) {
				main_board[col].push(card.value());
				main_board[col].back().face_down = (col != row);
			}
		}
	}
}

CardPile Board::column(size_t idx) const {
	if (idx == DECK_IDX) return draw_pile;
	if (idx == FLIP_IDX) return flipped;
	if (idx >= ACE_IDX && idx < BOARD_IDX) return aces[idx - ACE_IDX];
	if (idx >= BOARD_IDX) return main_board[idx - BOARD_IDX];

	throw "Index out of bounds";
}

CardPile& Board::column(size_t idx) {
	if (idx == DECK_IDX) return draw_pile;
	if (idx == FLIP_IDX) return flipped;
	if (idx >= ACE_IDX && idx < BOARD_IDX) return aces[idx - ACE_IDX];
	if (idx >= BOARD_IDX) return main_board[idx - BOARD_IDX];

	// Throw an error if it's not one of the main ones
	throw "Index out of bounds";
}

// A whole bunch of functions designed to be used when checking for move legality
struct BoardChecker {
	const Board& board;

	// Check if the deck can be used, either drawing or reshuffling
	bool can_click_deck() const { return !(board.draw_pile.empty() && board.flipped.empty()); }

	/// Properties of an index itself
	// Checks if the index is the draw pile
	bool is_deck(size_t idx) const { return idx == board.DECK_IDX; }
	// Checks if the index is the flip pile
	bool is_flip_pile(size_t idx) const { return idx == board.FLIP_IDX; }
	// Checks if the index is one of the four ace slots
	bool is_ace_slot(size_t idx) const { return idx >= board.ACE_IDX && idx < board.BOARD_IDX; }
	// Checks if the index is in the main board of cards
	bool is_board(size_t idx) const { return idx >= board.BOARD_IDX; }

	/// Index modifying functions
	// Get the index of the nth ace
	size_t get_ace_idx(size_t n) const { return board.ACE_IDX + n; }

	// /// Properties of the card at an index
	// Check if the card at an index is a king
	bool is_king(Card card) const { return card.rank() == 12; }
	// Check if the card at an index is an ace
	bool is_ace(Card card) const { return card.rank() == 0; }

	Card get_last_card(size_t idx) const {
		if (is_board(idx)) {
			const auto col = board.main_board[idx - board.BOARD_IDX];
			return col.empty() ? board.EMPTY_CARD : col.back();
		}

		if (is_ace_slot(idx)) {
			const auto col = board.aces[idx - board.ACE_IDX];
			return col.empty() ? board.EMPTY_CARD : col.back();
		}

		if (is_flip_pile(idx)) {
			const auto col = board.flipped;
			return col.empty() ? board.EMPTY_CARD : col.back();
		}

		return board.EMPTY_CARD;
	}

	CardPile get_board_column(size_t idx) const {
		return board.main_board[idx - board.BOARD_IDX];
	}

	// Check if two cards are suits of the same color
	bool is_same_color(Card card_a, Card card_b) const {
		// We divide by two since the suits are laid out like Heart Diamond Club Spade
		// So the two pairs are next to each other and we can just divide and see if they are in the same "group"
		return (card_a.suit() / 2) == (card_b.suit() / 2);
	}

	// Check if a card can be placed underneath another one on the board
	bool can_go_under(Card moving, Card dst) const {
		if (dst.empty()) return is_king(moving);

		return !is_same_color(moving, dst) && moving.rank() + 1 == dst.rank();
	}

	// Check if a card can be placed on top of another one on an ace pile
	bool can_go_on_ace(Card moving, Card dst) const {
		if (dst.empty()) return is_ace(moving);

		return moving.suit() == dst.suit() && moving.rank() - 1 == dst.rank();
	}

	/// Takes the two columns as parameters
	bool is_legal_move(size_t src, size_t dst) const {
		// Missing additional rules:
			// If the moving card already had a match behind it,
			//	only allow it if the card behind the moving card has a matching ace
			//	that is also one rank below
			// This prevents the AI from just shuffling them back and forth
			// But still allows shifting them around when you need to get to the ace

			// If the king is already on the top, also illegal
			// This prevents the AI from moving it back and forth forever

		// ADDITIONAL STIPULATION:
		// Both indices have to be the deck.
		// This reduces the chance of accidentally clicking it
		if (is_deck(src) && is_deck(dst)) {
			// std::cout << "Clicked on deck\n";
			return can_click_deck();
		}

		// If we are trying to pull from an empty source, then it's obviously nonsense
		if (board.column(src).empty()) return false;

		if (is_board(dst)) {
			// std::cout << "Board Target\n";
			if (is_board(src)) {
				// std::cout << "Board Source\n";
				const auto target = get_last_card(dst);
				for (auto c : get_board_column(src)) {
					if (c.empty() || c.face_down) continue;
					if (can_go_under(c, target)) {
						return true;
					}
				}

				return false;
			}

			// std::cout << "Trying to move " << get_last_card(src) << " onto " << get_last_card(dst) << "\n";
			return can_go_under(get_last_card(src), get_last_card(dst));
		}

		if (is_ace_slot(dst)) {
			// std::cout << "Ace Target\n";
			// ADDITIONAL STIPULATION:
			// No moving aces between slots.
			// This prevents the AI from getting in a loop
			if (is_ace_slot(src)) return false;
			// std::cout << "Non-Ace Source\n";

			// std::cout << "Trying to move " << get_last_card(src) << " onto " << get_last_card(dst) << "\n";
			return can_go_on_ace(get_last_card(src), get_last_card(dst));
		}

		// std::cout << "Illegal Destination\n";
		return false;
	}
};

// God this is just begging to have actual testing
// Test all the internal sub-functions with toy setups
// Test a whole bunch of premade boards to ensure that the set of legal moves are correct
// I miss rust's testing suite

/// Takes the two columns as parameters
bool Board::is_legal_move(size_t src, size_t dst) const {
	BoardChecker bc{*this};
	return bc.is_legal_move(src, dst);
}

void Board::perform_move(size_t src, size_t dst) {
	if (!is_legal_move(src, dst)) return;

	BoardChecker bc{*this};

	if (bc.is_deck(src)) {
		draw_from_deck();
		return;
	}

	if (bc.is_board(src)) {
		if (bc.is_board(dst)) {
			const auto target = bc.get_last_card(dst);
			const auto col = bc.get_board_column(src);

			for (size_t i = 0; i < col.size(); ++i) {
				const Card& c = col[i];
				if (c.empty() || c.face_down) continue;
				if (bc.can_go_under(c, target)) {
					// Move the rest of the vector from one column to the other
					move_tail(column(src), column(dst), i);
					break;
				}
			}

			// if the last card in the column is now face down, flip it
			if (!column(src).empty()) {
				column(src).back().face_down = false;
			}

			return;
		}

		move_card(src, dst);

		// if the last card in the column is now face down, flip it
		if (!column(src).empty()) {
			column(src).back().face_down = false;
		}
	} else {
		move_card(src, dst);
	}
}

bool Board::has_won() const {
	for (size_t i = 0; i < 4; ++i) {
		if (aces[i].back().rank() != 12) return false;
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