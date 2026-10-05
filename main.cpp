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

#include <iomanip>
#include <iostream>
#include <random>
#include <span>

#include "solitaire.hpp"

struct Node;

struct Connection {
	double weight;
	// The one whose value we source from
	// Indexed as if the input nodes and the free floating nodes are smushed together
	size_t backtarget;
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
			for (size_t i = 0; i < num_input; ++i) {
				if (prob_dist(gen) > 0.5) {
					outnode.cons.push_back(Connection {
						weight_dist(gen),
						i
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

	void backprop(Node& n) {
		if (n.propagated) return;

		n.value = n.bias;
		for (auto& con : n.cons) {
			auto back = get_src_node(con.backtarget);
			backprop(back);

			// TODO: Add activation function
			n.value += back.value * con.weight;
		}

		n.propagated = true;
	}

	void backprop() {
		for (auto& node : nodes) {
			node.propagated = false;
		}

		for (auto& node : outputs) {
			node.propagated = false;
			backprop(node);
		}
	}

	Node& get_src_node(size_t idx) {
		if (idx < num_input) {
			return inputs[idx];
		} else {
			return nodes[idx - num_input];
		}
	}
	const Node& get_src_node(size_t idx) const {
		if (idx < num_input) {
			return inputs[idx];
		} else {
			return nodes[idx - num_input];
		}
	}

	template <typename RNG>
	size_t select_src_idx(RNG& gen) {
		std::uniform_int_distribution<> src_distrib{0, static_cast<int>(num_input + nodes.size() - 1)};
		return src_distrib(gen);
	}

	template <typename RNG>
	Node* select_dst_node(RNG& gen) {
		std::uniform_int_distribution<> dst_distrib{0, static_cast<int>(num_output + nodes.size() - 1)};
		int dst_idx = dst_distrib(gen);

		if (dst_idx < num_output) {
			return &outputs[dst_idx];
		} else {
			return &nodes[dst_idx - num_output];
		}
	}

	template <typename RNG>
	Node* select_any_node(RNG& gen) {
		std::uniform_int_distribution<> idx_distrib{0, static_cast<int>(num_input + num_output + nodes.size() - 1)};
		int idx = idx_distrib(gen);

		if (idx < num_output) {
			return &outputs[idx];
		} else if (idx < num_output + num_input) {
			return &inputs[idx - num_output];
		} else {
			return &nodes[idx - num_output - num_input];
		}
	}

	template <typename RNG>
	void mutate(RNG& gen) {
		// Mutation Types:
			// Add connection
			// Add node
			// Change connection weight
			// Change node bias
			// TODO: Remove connection?
			// TODO: Remove node?
		const double NODE_CHANCE = 0.3;
		const double CONNECTION_CHANCE = 0.7;
		const double WEIGHT_CHANCE = 0.3;
		const double BIAS_CHANCE = 0.3;

		std::uniform_real_distribution<> prob_dist{0, 1};

		if (prob_dist(gen) < CONNECTION_CHANCE) {
			std::cout << "Adding new connection\n";
			auto src = select_src_idx(gen);
			Node* dst = select_dst_node(gen);

			dst->cons.push_back(Connection {
				0, // TODO Weight
				src
			});
		} else if (prob_dist(gen) < NODE_CHANCE) {
			std::cout << "Adding new node\n";
			std::uniform_int_distribution<> out_distrib{0, num_output - 1};
			nodes.push_back(Node{});
			select_dst_node(gen)->cons.push_back(Connection {
				1, // TODO Bias
				num_input + nodes.size() - 1
			});
		} else if (prob_dist(gen) < WEIGHT_CHANCE) {
			std::cout << "Modifying connection weight\n";
			Node* node = select_dst_node(gen);

			if (!node->cons.empty()) {
				// Select the connection within the node
				std::uniform_int_distribution<> con_distrib{0, static_cast<int>(node->cons.size() - 1)};
				int con_idx = con_distrib(gen);

				std::uniform_real_distribution<> weight_dist{-0.1, 0.1};
				node->cons[con_idx].weight += weight_dist(gen);
			}
		} else if (prob_dist(gen) < BIAS_CHANCE) {
			std::cout << "Modifying node bias\n";
			std::uniform_real_distribution<> bias_dist{-0.1, 0.1};
			select_any_node(gen)->bias += bias_dist(gen);
		}
	}

	std::ostream& serialize(std::ostream& os) const {
		// TODO
	}

	static Robot read(std::istream& is) {
		// TODO
	}

	void node_path(std::ostream& os, const Node& n, size_t depth) const {
		os << n.bias;
		for (auto&& con : n.cons) {
			os << "\n" << std::string(depth + 1, '\t');
			node_path(os, get_src_node(con.backtarget), depth + 1);
		}
	}

	friend std::ostream& operator<<(std::ostream& os, const Robot& rob) {
		os << std::setprecision(2);
		for (int i = 0; i < rob.outputs.size(); ++i) {
			rob.node_path(os, rob.outputs[i], 0);
			os << "\n";
		}

		return os;
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

	for (int i = 0; i < 100; ++i) {
		robot.mutate(g);
		robot.backprop();
		std::cout << robot.outputs << "\n";
	}

	std::cout << robot.nodes.size() << "\n";

	std::cout << robot << "\n";

	// random_player(g, board);

	std::cout << board.draw_pile.cards << "\n";

	return 0;
}