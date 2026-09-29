// GOALS:
// Shuffle up a deck
// Deal it out
// Display it in the terminal
// Allow selecting an index and return the legal moves for it

// 7x19 for the main board
// 4 aces
// deck
// turned card

// 0 - empty
// [1, 52] - cards
// 53 - face down

#include <iostream>
#include <array>
#include <algorithm>
#include <random>

template <typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr) {
	for (auto&& elem : arr) {
		os << elem << ", ";
	}
	return os;
}

int main() {
	std::random_device rd;
	std::mt19937 g(rd());

	std::array<size_t, 52> deck{};
	std::iota(deck.begin(), deck.end(), 1);

	std::cout << deck << "\n";

	std::shuffle(deck.begin(), deck.end(), g);

	std::cout << deck << "\n";

	return 0;
}