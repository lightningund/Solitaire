#include <iostream>
#include <array>
#include <vector>
#include <optional>
#include <algorithm>

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

struct Deck {
	std::vector<Card> cards{};

	Deck();
	Deck(std::vector<Card> src) : cards{src} {}

	template <typename RandomGen>
	void shuffle(RandomGen& gen) {
		std::shuffle(cards.begin(), cards.end(), gen);
	}

	std::optional<Card> take_top_card();

	std::optional<Card> view_top_card() const;

	Deck clone() const;

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

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& arr) {
	for (auto&& elem : arr) {
		os << elem << ", ";
	}

	return os;
}

struct Board {
	private:
	const Card EMPTY_CARD{};

	Card get_board_card(size_t idx) const;
	Card& get_board_card(size_t idx);

	Card get_ace_card(size_t idx) const;
	Card& get_ace_card(size_t idx);

	Card get_deck_card() const;
	Card& get_deck_card();

	Card get_flip_card() const;
	Card& get_flip_card();

	void draw_from_deck();

	void flip_card(size_t idx);

	void pop_card(size_t idx);

	void move_card(size_t src, size_t dst);

	bool has_won() const;
	size_t num_ace_cards() const;
	size_t num_face_up() const;

	// Check if the deck can be used, either drawing or reshuffling
	bool can_click_deck() const;

	/// Properties of an index itself
	// Checks if the index is the draw pile
	bool is_deck(size_t idx) const;
	// Checks if the index is the flip pile
	bool is_flip_pile(size_t idx) const;
	// Checks if the index is one of the four ace slots
	bool is_ace_slot(size_t idx) const;
	// Checks if the index is in the main board of cards
	bool is_board(size_t idx) const;
	// Checks if the index is in the top row of the board
	// Meaningless if idx is not on the board
	bool is_top_row(size_t idx) const;
	// Checks if the index is in the bottom row of the board
	bool is_bottom_row(size_t idx) const;

	/// Index modifying functions
	// Get the index of the card one above this one
	// Returns the same index if it's in the top row
	// UB if you call it with an index outside of the board
	size_t one_above(size_t idx) const;
	// Get the index of the card one below this one
	// Returns the same index if it's in the bottom row
	// UB if you call it with an index outside of the board
	size_t one_below(size_t idx) const;
	// Get the index of the nth ace
	size_t get_ace(size_t n) const;

	/// Properties of the card at an index
	// Check if the card at an index is empty
	bool is_empty(size_t idx) const;
	// Check if the card at an index is face down
	bool is_face_down(size_t idx) const;
	// Check if the card at an index is a king
	bool is_king(size_t idx) const;
	// Check if the card at an index is an ace
	bool is_ace(size_t idx) const;
	// Get the rank of a card at an index
	size_t get_rank(size_t idx) const;
	// Get the suit of a card at an index
	size_t get_suit(size_t idx) const;

	// Check if two cards are suits of the same color
	bool is_same_color(size_t idx_a, size_t idx_b) const;

	public:
	static constexpr size_t COLS = 7;
	static constexpr size_t DECK_IDX = 0;
	static constexpr size_t FLIP_IDX = DECK_IDX + 1;
	static constexpr size_t ACE_IDX = FLIP_IDX + 1;
	static constexpr size_t BOARD_IDX = ACE_IDX + 4;
	// Outside the bounds, the last real index is this -1
	static constexpr size_t END_IDX = BOARD_IDX + COLS;

	std::array<std::vector<Card>, COLS> main_board{};
	std::array<std::vector<Card>, 4> aces = {std::vector<Card>{EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}};
	Deck draw_pile{};
	std::vector<Card> flipped{};

	void deal(Deck& deck);

	Card cindex(size_t idx) const;
	Card index(size_t idx) const;
	Card& index(size_t idx);

	bool is_legal_move(size_t idx_a, size_t idx_b) const;

	void perform_move(size_t idx_a, size_t idx_b);

	double score() const;

	friend std::ostream& operator<<(std::ostream& os, const Board& board) {
		if (auto deck_top = board.draw_pile.view_top_card()) {
			auto card = deck_top.value();
			card.face_down = true;
			os << card;
		} else {
			os << board.EMPTY_CARD;
		}
		os << "\n";

		if (!board.flipped.empty()) {
			auto card = board.flipped[board.flipped.size() - 1];
			os << card;
		} else {
			os << board.EMPTY_CARD;
		}
		os << "\n";

		for (auto&& card : board.aces) {
			os << card.back() << "\n";
		}

		// Prints the board sideways but whatever
		for (auto&& col : board.main_board) {
			for (auto&& card : col) {
				os << card << " ";
			}

			os << "\n";
		}

		return os;
	}
};
