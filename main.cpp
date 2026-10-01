// GOALS:
// X Shuffle up a deck
// X Deal it out
// X Display it in the terminal
// Allow selecting an index and return the legal moves for it

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
	Card get_board_card(size_t idx) {
		return main_board[idx / COLS][idx % COLS];
	}

	Card get_ace_card(size_t idx) {
		return aces[idx];
	}

	Card get_deck_card(size_t idx) {
		if (auto card = draw_pile.view_top_card()) {
			return card.value();
		}

		return Card{};
	}

	Card get_flipped_card(size_t idx) {
		if (!flipped.empty()) {
			return flipped[flipped.size() - 1];
		}

		return Card{};
	}

	public:
	static constexpr size_t COLS = 7;
	static constexpr size_t ROWS = COLS + 13 - 1;

	std::array<std::array<Card, COLS>, ROWS> main_board{};
	std::array<Card, 4> aces{};
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

	Card index(size_t idx) {
		if (idx < COLS * ROWS) {
			return get_board_card(idx);
		}

		idx -= COLS * ROWS;
		if (idx < 4) {
			return get_ace_card(idx);
		}

		idx -= 4;
		if (idx == 0) {
			return get_deck_card(idx);
		}

		return get_flipped_card(idx);
	}

	bool is_legal_move(size_t idx_a, size_t idx_b) {
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
					return is_top_row(idx_b);
				} else {
					if (is_top_row(idx_b)) return false;
					if (is_empty(one_above(idx_b))) return false;
					if (is_same_color(idx_a, one_above(idx_b))) return false;
					if (get_rank(idx_a) + 1 != get_rank(one_above(idx_b))) return false;

					return true;
				}
			} else {
				if (!is_bottom(idx_a) && !is_empty(one_below(idx_a))) return false;

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
		// TODO
	}

	friend std::ostream& operator<<(std::ostream& os, const Board& board) {
		for (auto&& row : board.main_board) {
			for (auto&& card : row) {
				os << card << " ";
			}

			os << "\n";
		}

		for (auto&& card : board.aces) {
			os << card << " ";
		}

		if (auto deck_top = board.draw_pile.view_top_card()) {
			auto card = deck_top.value();
			card.face_down = true;
			os << card << " ";
		}

		if (!board.flipped.empty()) {
			auto card = board.flipped[board.flipped.size() - 1];
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

	std::cout << deck << "\n";

	Board board{};
	board.deal(deck);

	std::cout << board << "\n";

	for (size_t i = 0; i < 7 * 19 + 6; ++i) {
		std::cout << board.index(i) << ", ";
	}
	std::cout << "\n";

	size_t idx;
	std::cin >> idx;

	for (size_t i = 0; i < 7 * 19 + 6; ++i) {
		std::cout << board.is_legal_move(idx, i) << ", ";
	}
	std::cout << "\n";

	return 0;
}