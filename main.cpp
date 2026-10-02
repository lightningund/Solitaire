// GOALS:
// X Shuffle up a deck
// X Deal it out
// X Display it in the terminal
// X Allow selecting an index and return the legal moves for it
// X Move performing
// Win checking

// Threefold repeat checking(?)

// NEAT algorithm
// Connect it to solitaire
// Save the progress over time for later viewing
// Animate the AI playing solitaire

// 7x19 for the main board
// 4 aces
// deck
// turned card

#include <iostream>
#include <random>

#include "solitaire.hpp"

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

	std::cout << board.draw_pile.cards << "\n";

	return 0;
}