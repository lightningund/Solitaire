// GOALS:
// X Shuffle up a deck
// Deal it out
// Display it in the terminal
// Allow selecting an index and return the legal moves for it

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
constexpr CardRep FLIPPED_REP = 53;

struct Card {
	private:
	const std::string EMPTY_STRING = "  ";
	const std::string FLIPPED_STRING = "[]";

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
		if (flipped) return FLIPPED_STRING;
		if (num == EMPTY_REP) return EMPTY_STRING;

		return get_rank_print(get_rank()) + get_suit_print(get_suit());
	}

	public:
	CardRep num{EMPTY_REP};
	bool flipped{false};

	Card() {}
	Card(size_t idx) : num{idx}, flipped{false} {}

	size_t get_rank() const {

	}

	size_t get_suit() const {

	}

	// Get the number that represents the current state of the card
	CardRep get_rep_num() const {
		return flipped ? FLIPPED_REP : num;
	}

	friend std::ostream& operator<<(std::ostream& os, const Card& card) {
		return os << card.get_printed_rep();
	}
};

struct Deck {
	std::vector<Card> cards{};

	Deck() {

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

	Deck clone() const {
		std::vector<Card> dest{};
		dest.resize(cards.size());
		std::copy(cards.begin(), cards.end(), dest.begin());
		return dest;
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

	std::array<std::array<Card, 18>, COLS> main_board{};
	std::array<Card, 4> aces{};
	Deck draw_pile{};
	std::vector<Card> flipped{};

	void deal(Deck& deck) {
		draw_pile = deck.clone();
		for (size_t i = 0; i < COLS; ++i) {
			for (size_t j = i; j < COLS; ++j) {
				auto card = deck.take_top_card();
				cols[j][i] = card;
				if(i == j){
					cols[i][j].flipped = true;
				}
			}
		}
	}

	Card index(size_t idx) {
		if (idx < 7 * 18) {
			return get_board_card(idx);
		}

		idx -= (7 * 18);
		if (idx < 4) {
			return get_ace_card(idx);
		}

		idx -= 4;
		if (idx == 0) {
			return get_deck_card(idx);
		}

		return get_flipped_card(idx);
	}
};

int main() {
	std::random_device rd;
	std::mt19937 g(rd());

	std::array<Card, 52> deck{};
	std::iota(deck.begin(), deck.end(), 1);

	std::cout << deck << "\n";

	std::shuffle(deck.begin(), deck.end(), g);

	std::cout << deck << "\n";

	return 0;
}