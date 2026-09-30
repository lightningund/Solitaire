// GOALS:
// X Shuffle up a deck
// X Deal it out
// Display it in the terminal
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
	std::string get_printed_rep() const {
		if (face_down) return FACE_DOWN_STRING;
		if (num == EMPTY_REP) return EMPTY_STRING;

		return get_rank_print(get_rank()) + get_suit_print(get_suit());
	}

	public:
	CardRep num{EMPTY_REP};
	bool face_down{false};

	Card() {}
	Card(size_t idx) : num{idx}, face_down{false} {}

	size_t get_rank() const {
		return num % 13;
	}

	size_t get_suit() const {
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
		return os << card.get_printed_rep();
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
	static constexpr size_t COLS = 7;
	static constexpr size_t ROWS = COLS + 13;

	std::array<std::array<Card, COLS>, ROWS> main_board{};
	std::array<Card, 4> aces{};
	Deck draw_pile{};
	std::vector<Card> flipped{};

	void deal(Deck& deck) {
		draw_pile = deck.clone();
		for (size_t row = 0; row < COLS; ++row) {
			for (size_t col = row; col < COLS; ++col) {
				if (auto card = deck.take_top_card()) {
					main_board[row][col] = card.value();
					main_board[row][col].face_down = (row != col);
				}
			}
		}
	}

	Card index(size_t idx) {
		// TODO
		// if (idx < 7 * 18) {
		// 	return get_board_card(idx);
		// }

		// idx -= (7 * 18);
		// if (idx < 4) {
		// 	return get_ace_card(idx);
		// }

		// idx -= 4;
		// if (idx == 0) {
		// 	return get_deck_card(idx);
		// }

		// return get_flipped_card(idx);
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

	// std::array<Card, 52> deck{};
	// std::iota(deck.begin(), deck.end(), 1);

	// std::cout << deck << "\n";

	// std::shuffle(deck.begin(), deck.end(), g);

	// std::cout << deck << "\n";

	return 0;
}