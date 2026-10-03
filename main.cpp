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
#include <span>

#include "solitaire.hpp"

struct Node;

struct Connection {
	double weight;
	// The one whose value we source from
	Node* backtarget;
};

struct Node {
	std::vector<Connection> cons;
	double bias;
	double value;
	bool propagated{false};

	friend std::ostream& operator<<(std::ostream& os, const Node& node) {
		return os << node.value;
	}
};

template <size_t num_input, size_t num_output>
struct Robot {
	std::array<Node, num_input> inputs{};
	std::array<Node, num_output> outputs{};
	std::vector<Node> nodes{};

	template <typename RNG>
	Robot(RNG& gen) {
		std::uniform_real_distribution<> prob_dist{0, 1};
		std::uniform_real_distribution<> weight_dist{-5, 5};
		for (auto& outnode : outputs) {
			for (auto& innode : inputs) {
				if (prob_dist(gen) > 0.5) {
					outnode.cons.push_back(Connection {
						weight_dist(gen),
						&innode
					});
				}
			}
		}
	}

	void set_inputs(std::span<double> vals) {
		for (int i = 0; i < num_input; ++i) {
			inputs[i].value = vals[i];
			inputs[i].propagated = true;
		}
	}

	static void backprop(Node* n) {
		n->value = n->bias;
		for (auto& con : n->cons) {
			if (!con.backtarget->propagated) {
				backprop(con.backtarget);
			}

			n->value += con.backtarget->value * con.weight;
		}

		n->propagated = true;
	}

	void backprop() {
		for (auto& node : outputs) {
			backprop(&node);
		}
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

	Robot<Board::END_IDX, 2> robot{g};

	std::vector<double> board_arr{};

	for (size_t i = 0; i < Board::END_IDX; ++i) {
		board_arr.push_back(board.cindex(i).get_rep_num());
	}

	robot.set_inputs(board_arr);

	robot.backprop();

	std::cout << robot.outputs << "\n";

	// Exclude the deck
	std::uniform_int_distribution<> idx_distrib{1, Board::END_IDX - 1};

	for (int i = 0; i < 1000; ++i) {
		size_t tries = 0;
		size_t idx_a = 0;
		size_t idx_b = 0;
		do {
			idx_a = idx_distrib(g);
			idx_b = idx_distrib(g);
			++tries;

			std::cout << tries << "\r";

			// Failsafe for if we don't have any legal moves
			if (tries > 50000) {
				idx_a = Board::DECK_IDX;
				break;
			}
		} while (!board.is_legal_move(idx_a, idx_b));

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