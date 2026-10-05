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
		std::uniform_real_distribution<> weight_dist{-1, 1};
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

			// TODO: Add activation function
			n->value += con.backtarget->value * con.weight;
		}

		n->propagated = true;
	}

	void backprop() {
		for (auto& node : nodes) {
			node.propagated = false;
		}
		for (auto& node : outputs) {
			backprop(&node);
		}
	}

	template <typename RNG>
	void mutate(RNG& gen) {
		// Mutation Types:
			// Add connection
			// Add node
			// Change connection weight
			// Change node bias
		const double NODE_CHANCE = 0.3;
		const double CONNECTION_CHANCE = 0.3;
		const double WEIGHT_CHANCE = 0.3;
		const double BIAS_CHANCE = 0.3;

		std::uniform_real_distribution<> prob_dist{0, 1};

		if (prob_dist(gen) < NODE_CHANCE) {
			std::uniform_int_distribution<> out_distrib{0, num_output - 1};
			nodes.push_back(Node{});
			outputs[out_distrib(gen)].cons.push_back(Connection {
				1, // TODO Bias
				&nodes.back()
			});
		} else if (prob_dist(gen) < CONNECTION_CHANCE) {
			// Split this into select_src_node() and select_dst_node()
			std::uniform_int_distribution<> src_distrib{0, num_input + nodes.size() - 1};
			std::uniform_int_distribution<> dst_distrib{0, num_output + nodes.size() - 1};
			auto src_idx = src_distrib(gen);
			auto dst_idx = dst_distrib(gen);

			Node* src;
			Node* dst;
			if (src_idx < num_input) {
				src = &inputs[src_idx];
			} else {
				src = &nodes[src_idx - num_input];
			}

			if (dst_idx < num_output) {
				dst = &outputs[dst_idx];
			} else {
				dst = &nodes[dst_idx - num_output];
			}

			dst->cons.push_back(Connection {
				0, // TODO Weight
				src
			});
		} else if (prob_dist(gen) < WEIGHT_CHANCE) {
			// Split this into select_dst_node()
			std::uniform_int_distribution<> idx_distrib{0, num_output + nodes.size() - 1};
			std::uniform_real_distribution<> weight_dist{-0.1, 0.1};

			auto idx = idx_distrib(gen);
			Node* node;
			if (idx < num_output) {
				node = &outputs[idx];
			} else {
				node = &nodes[idx - num_output];
			}

			// Select the connection within the node
			std::uniform_int_distribution<> con_distrib{0, static_cast<int>(node->cons.size()) - 1};
			auto con_idx = con_distrib(gen);

			node->cons[con_idx].weight += weight_dist(gen);
		} else if (prob_dist(gen) < BIAS_CHANCE) {
			// Split this into select_any_node()
			std::uniform_int_distribution<> idx_distrib{0, num_input + num_output + nodes.size() - 1};
			std::uniform_real_distribution<> bias_dist{-0.1, 0.1};

			auto idx = idx_distrib(gen);
			Node* node;
			if (idx < num_output) {
				node = &outputs[idx];
			} else if (idx < num_output + num_input) {
				node = &inputs[idx - num_output];
			} else {
				node = &nodes[idx - num_output - num_input];
			}

			node->bias += weight_dist(gen);
		}
	}

	std::ostream& serialize(std::ostream& os) const {
		// TODO
	}

	static Robot read(std::istream& is) {
		// TODO
	}
};

template <typename RNG>
void random_player(RNG& gen, Board& board) {
	// Exclude the deck
	std::uniform_int_distribution<> idx_distrib{1, Board::END_IDX - 1};

	for (int i = 0; i < 1000; ++i) {
		size_t tries = 0;
		size_t idx_a = 0;
		size_t idx_b = 0;
		do {
			idx_a = idx_distrib(gen);
			idx_b = idx_distrib(gen);
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
}

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

	// random_player(g, board);

	std::cout << board.draw_pile.cards << "\n";

	return 0;
}