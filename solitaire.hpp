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

	Card get_flipped_card() const;
	Card& get_flipped_card();

	void draw_from_deck();

	void flip_card(size_t idx);

	void pop_card(size_t idx);

	void move_card(size_t src, size_t dst);

	public:
	static constexpr size_t COLS = 7;
	static constexpr size_t ROWS = COLS + 13 - 1;
	static constexpr size_t DECK_IDX = ROWS * COLS + 5;

	std::array<std::array<Card, COLS>, ROWS> main_board{};
	std::array<std::vector<Card>, 4> aces = {std::vector<Card>{EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}};
	Deck draw_pile{};
	std::vector<Card> flipped{};

	void deal(Deck& deck);

	Card index(size_t idx) const;

	Card& index(size_t idx);

	// God this is just begging to have actual testing
	// Test all the internal sub-functions with toy setups
	// Test a whole bunch of premade boards to ensure that the set of legal moves are correct
	// I miss rust's testing suite
	bool is_legal_move(size_t idx_a, size_t idx_b) const;

	void perform_move(size_t idx_a, size_t idx_b);

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
