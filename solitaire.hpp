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

	public:
	const Card EMPTY_CARD{};

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

	std::vector<Card> column(size_t idx) const;
	std::vector<Card>& column(size_t idx);

	bool is_legal_move(size_t idx_a, size_t idx_b) const;

	void perform_move(size_t idx_a, size_t idx_b);

	double score() const;

	friend std::ostream& operator<<(std::ostream& os, const Board& board) {
		size_t col = 0;

		const auto print = [&]() {
			os << col++ << ": ";
		};

		print();
		if (auto deck_top = board.draw_pile.view_top_card()) {
			auto card = deck_top.value();
			card.face_down = true;
			os << card;
		} else {
			os << board.EMPTY_CARD;
		}
		os << "\n";

		print();
		if (!board.flipped.empty()) {
			auto card = board.flipped[board.flipped.size() - 1];
			os << card;
		} else {
			os << board.EMPTY_CARD;
		}
		os << "\n";

		for (auto&& card : board.aces) {
			print();
			os << card.back() << "\n";
		}

		// Prints the board sideways but whatever
		for (auto&& column : board.main_board) {
			print();
			for (auto&& card : column) {
				os << card << " ";
			}
			os << "\n";
		}

		return os;
	}
};
