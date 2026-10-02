// GOALS:
// X Shuffle up a deck
// X Deal it out
// X Display it in the terminal
// X Allow selecting an index and return the legal moves for it
// X Move performing
// Win checking

// NEAT algorithm
// Connect it to solitaire
// Save the progress over time for later viewing
// Animate the AI playing solitaire

// 7x19 for the main board
// 4 aces
// deck
// turned card

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <optional>
#include <cassert>

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
	std::string get_print() const {
		if (face_down) return FACE_DOWN_STRING;
		if (num == EMPTY_REP) return EMPTY_STRING;

		return get_rank_print(rank()) + get_suit_print(suit());
	}

	public:
	CardRep num{EMPTY_REP};
	bool face_down{false};

	Card() {}
	Card(size_t idx) : num{idx}, face_down{false} {}

	size_t rank() const {
		return num % 13;
	}

	size_t suit() const {
		return num / 13;
	}

	// Get the number that represents the current state of the card
	CardRep get_rep_num() const {
		return face_down ? FACE_DOWN_REP : num;
	}

	bool empty() const {
		return num == EMPTY_REP;
	}

	Card operator=(const Card& src) {
		num = src.num;
		face_down = src.face_down;
		return *this;
	}

	friend std::ostream& operator<<(std::ostream& os, const Card& card) {
		return os << card.get_print();
	}
};

struct Deck {
	std::vector<Card> cards{};

	Deck() {
		cards.resize(52);
		std::iota(cards.begin(), cards.end(), 0);
	}

	Deck(std::vector<Card> src) : cards{src} {}

	template <typename RandomGen>
	void shuffle(RandomGen& gen) {
		std::shuffle(cards.begin(), cards.end(), gen);
	}

	std::optional<Card> take_top_card() {
		std::optional<Card> top_card{};
		if (!cards.empty()) {
			top_card = cards[cards.size() - 1];
			cards.pop_back();
		}

		return top_card;
	}

	std::optional<Card> view_top_card() const {
		std::optional<Card> top_card{};
		if (!cards.empty()) {
			top_card = cards[cards.size() - 1];
		}

		return top_card;
	}

	Deck clone() const {
		std::vector<Card> dest{};
		dest.resize(cards.size());
		std::copy(cards.begin(), cards.end(), dest.begin());
		return dest;
	}

	friend std::ostream& operator<<(std::ostream& os, const Deck& deck) {
		for (auto&& elem : deck.cards) {
			os << elem << ", ";
		}

		return os;
	}
};

template <typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr) {
	for (auto&& elem : arr) {
		os << elem << ", ";
	}

	return os;
}

struct Board {
	private:
	const Card EMPTY_CARD{};

	Card get_board_card(size_t idx) const {
		return main_board[idx / COLS][idx % COLS];
	}
	Card& get_board_card(size_t idx) {
		return main_board[idx / COLS][idx % COLS];
	}

	Card get_ace_card(size_t idx) const {
		return aces[idx].back();
	}
	Card& get_ace_card(size_t idx) {
		return aces[idx].back();
	}

	Card get_deck_card() const {
		if (auto card = draw_pile.view_top_card()) {
			return card.value();
		}

		return EMPTY_CARD;
	}
	Card& get_deck_card() {
		return draw_pile.cards.back();
	}

	Card get_flipped_card() const {
		if (!flipped.empty()) {
			return flipped.back();
		}

		return EMPTY_CARD;
	}
	Card& get_flipped_card() {
		return flipped.back();
	}

	void draw_from_deck() {
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

	void flip_card(size_t idx) {
		index(idx).face_down = false;
	}

	void pop_card(size_t idx) {
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

	void move_card(size_t src, size_t dst) {
		index(dst) = index(src);
		index(src) = EMPTY_CARD;
	}

	public:
	static constexpr size_t COLS = 7;
	static constexpr size_t ROWS = COLS + 13 - 1;
	static constexpr size_t DECK_IDX = ROWS * COLS + 5;

	std::array<std::array<Card, COLS>, ROWS> main_board{};
	std::array<std::vector<Card>, 4> aces = {std::vector<Card>{EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}};
	Deck draw_pile{};
	std::vector<Card> flipped{};

	void deal(Deck& deck) {
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

	Card index(size_t idx) const {
		if (idx < COLS * ROWS) {
			return get_board_card(idx);
		}

		idx -= COLS * ROWS;
		if (idx < 4) {
			return get_ace_card(idx);
		}

		idx -= 4;
		if (idx == 0) {
			return get_flipped_card();
		}

		return get_deck_card();
	}

	Card& index(size_t idx) {
		if (idx < COLS * ROWS) {
			return get_board_card(idx);
		}

		idx -= COLS * ROWS;
		if (idx < 4) {
			return get_ace_card(idx);
		}

		idx -= 4;
		if (idx == 0) {
			return get_flipped_card();
		}

		return get_deck_card();
	}

	// God this is just begging to have actual testing
	// Test all the internal sub-functions with toy setups
	// Test a whole bunch of premade boards to ensure that the set of legal moves are correct
	// I miss rust's testing suite
	bool is_legal_move(size_t idx_a, size_t idx_b) const {
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
		const auto is_deck = [](size_t idx) { return idx == (COLS * ROWS + 5); };
		// Checks if the index is the flip pile
		const auto is_flip_pile = [](size_t idx) { return idx == (COLS * ROWS + 4); };
		// Checks if the index is in the main board of cards
		const auto is_board = [](size_t idx) { return idx < (COLS * ROWS); };
		// Checks if the index is in the top row of the board
		const auto is_top_row = [](size_t idx) { return idx < COLS; };
		// Checks if the index is in the bottom row of the board
		const auto is_bottom_row = [](size_t idx) { return idx > (COLS * (ROWS - 1)) && idx < (COLS * ROWS); };

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

					return true;
				}
			} else {
				if (!is_bottom_row(idx_a) && !is_empty(one_below(idx_a))) return false;

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

	void perform_move(size_t idx_a, size_t idx_b) {
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
		const auto is_deck = [](size_t idx) { return idx == (COLS * ROWS + 5); };
		// Checks if the index is the flip pile
		const auto is_flip_pile = [](size_t idx) { return idx == (COLS * ROWS + 4); };
		// Checks if the index is one of the four ace slots
		const auto is_ace_slot = [](size_t idx) { return idx > (COLS * ROWS) && idx < (COLS * ROWS + 4); };
		// Checks if the index is in the main board of cards
		const auto is_board = [](size_t idx) { return idx < (COLS * ROWS); };
		// Checks if the index is in the top row of the board
		const auto is_top_row = [](size_t idx) { return idx < COLS; };
		// Checks if the index is in the bottom row of the board
		const auto is_bottom_row = [](size_t idx) { return idx > (COLS * (ROWS - 1)) && idx < (COLS * ROWS); };

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

	friend std::ostream& operator<<(std::ostream& os, const Board& board) {
		for (auto&& row : board.main_board) {
			for (auto&& card : row) {
				os << card << " ";
			}

			os << "\n";
		}

		for (auto&& card : board.aces) {
			os << card.back() << " ";
		}

		if (!board.flipped.empty()) {
			auto card = board.flipped[board.flipped.size() - 1];
			os << card << " ";
		}

		if (auto deck_top = board.draw_pile.view_top_card()) {
			auto card = deck_top.value();
			card.face_down = true;
			os << card << " ";
		}

		return os;
	}
};

int main() {
	std::random_device rd;
	std::mt19937 g(rd());

	Deck deck{};
	deck.shuffle(g);

	Board board{};
	board.deal(deck);

	std::cout << board << "\n";

	// Exclude the deck
	std::uniform_int_distribution<> idx_distrib{0, (Board::COLS * Board::ROWS + 4)};

	for (int i = 0; i < 1000; ++i) {
		size_t tries = 0;
		size_t idx_a = 0;
		size_t idx_b = 0;
		while (!board.is_legal_move(idx_a, idx_b)) {
			idx_a = idx_distrib(g);
			idx_b = idx_distrib(g);
			++tries;

			std::cout << tries << "\r";

			// Failsafe for if we don't have any legal moves
			if (tries > 50000) {
				idx_a = Board::DECK_IDX;
				break;
			}
		}

		// If we tripped the failsafe but can't draw just give up
		if (!board.is_legal_move(idx_a, idx_b)) {
			break;
		}

		std::cout << "Tries: " << tries << ", indices: " << idx_a << ", " << idx_b << "\n";
		board.perform_move(idx_a, idx_b);
		std::cout << board << "\n";
	}

	// size_t idx_a;
	// std::cin >> idx_a;

	// std::cout << idx_a << ", " << board.index(idx_a) << "\n";

	// size_t idx_b;
	// std::cin >> idx_b;

	// std::cout << idx_b << ", " << board.index(idx_b) << "\n";

	// std::cout << board.is_legal_move(idx_a, idx_b) << "\n";

	// board.perform_move(idx_a, idx_b);

	// std::cout << board << "\n";

	return 0;
}