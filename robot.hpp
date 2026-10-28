#pragma once

#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <span>
#include <random>

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

	// Let's do, say bias: f64, cons: [weight: f64, idx: u64, ...]X
	// So like 10.3 4.01 12 -0.01 180X
	// bias, then the weight and idx of each connection, delimiting with an X
	void serialize(std::ostream& os) const;

	friend std::istream& operator>>(std::istream& is, Node& node) {
		is >> node.bias;

		// Modified from https://en.cppreference.com/cpp/io/basic_istream/ignore
		while (true) {
			// Attempt to read in the weight of the next connection
			double weight;
			is >> weight;

			// If we hit the end or something is wrong or we couldn't read another weight,
			// then we're done
			if (is.eof() || is.bad() || is.fail()) {
				is.clear();
				// Skip ahead the X
				is.seekg(1, std::ios::cur);
				break;
			} else {
				// Read in the index of the backtarget
				size_t idx;
				is >> idx;
				node.cons.push_back(Connection { weight, idx });
			}
		}

		return is;
	}
};

void test_node_serial();

struct Robot {
	// Deck + Flip Pile + Aces + Columns * Max Rows + Tries
	// We add in the tries so that it can get itself unstuck as it tries over and over
	// TODO: Penalize using a lot of tries
	static constexpr size_t num_input = 1 + 1 + 4 + 7 * (7 + 13 - 1) + 1;
	static constexpr size_t num_output = 2;

	std::array<Node, num_input> inputs{};
	std::array<Node, num_output> outputs{};
	std::vector<Node> nodes{};

	double score = 0;

	Robot() {}

	template <typename RNG>
	Robot(RNG& gen) {
		std::uniform_real_distribution<> prob_dist{0, 1};
		std::uniform_real_distribution<> weight_dist{-1, 1};
		// Randomly connects the outputs to the inputs
		for (auto& outnode : outputs) {
			for (size_t i = 0; i < num_input; ++i) {
				outnode.cons.push_back(Connection {
					weight_dist(gen),
					i
				});
			}
		}
	}

	void set_inputs(std::span<double> vals);

	void backprop(Node& n);

	void backprop();

	Node& get_src_node(size_t idx);
	const Node& get_src_node(size_t idx) const;

	template <typename RNG>
	size_t select_src_idx(RNG& gen) {
		std::uniform_int_distribution<> src_distrib{0, static_cast<int>(num_input + nodes.size() - 1)};
		return src_distrib(gen);
	}

	Node& get_dst_node(size_t idx);
	const Node& get_dst_node(size_t idx) const;

	template <typename RNG>
	size_t select_dst_idx(RNG& gen) {
		std::uniform_int_distribution<> src_distrib{0, static_cast<int>(num_output + nodes.size() - 1)};
		return src_distrib(gen);
	}

	template <typename RNG>
	Node& select_dst_node(RNG& gen) {
		std::uniform_int_distribution<> dst_distrib{0, static_cast<int>(num_output + nodes.size() - 1)};
		int dst_idx = dst_distrib(gen);

		if (dst_idx < num_output) {
			return outputs[dst_idx];
		} else {
			return nodes[dst_idx - num_output];
		}
	}

	template <typename RNG>
	Node& select_any_node(RNG& gen) {
		std::uniform_int_distribution<> idx_distrib{0, static_cast<int>(num_input + num_output + nodes.size() - 1)};
		int idx = idx_distrib(gen);

		if (idx < num_output) {
			return outputs[idx];
		} else if (idx < num_output + num_input) {
			return inputs[idx - num_output];
		} else {
			return nodes[idx - num_output - num_input];
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
		const double WEIGHT_CHANCE = 0.4;
		const double BIAS_CHANCE = 0.5;
		const double CONNECTION_CHANCE = 0.3;
		const double NODE_CHANCE = 0.3;

		std::uniform_real_distribution<> prob_dist{0, 1};

		if (prob_dist(gen) < WEIGHT_CHANCE) {
			// std::cout << "Modifying connection weight\n";
			auto& node = select_dst_node(gen);

			if (!node.cons.empty()) {
				// Select the connection within the node
				std::uniform_int_distribution<> con_distrib{0, static_cast<int>(node.cons.size() - 1)};
				int con_idx = con_distrib(gen);

				std::uniform_real_distribution<> weight_dist{-0.1, 0.1};
				node.cons[con_idx].weight += weight_dist(gen);
			}
		} else if (prob_dist(gen) < BIAS_CHANCE) {
			// std::cout << "Modifying node bias\n";
			std::uniform_real_distribution<> bias_dist{-0.1, 0.1};
			select_any_node(gen).bias += bias_dist(gen);
		} else if (prob_dist(gen) < CONNECTION_CHANCE) {
			// std::cout << "Adding new connection\n";
			auto src = select_src_idx(gen);
			auto dst = select_dst_idx(gen);

			auto src_node = get_src_node(src);
			auto dst_node = get_dst_node(dst);

			// Only add it if there isn't already a link in the other direction
			// TODO: This still has the possibility to create cycles that are more than 2 nodes long
			if (
				std::find_if(src_node.cons.begin(), src_node.cons.end(), [&](Connection& con) {
					return con.backtarget == dst;
				}) == src_node.cons.end()
			) {
				dst_node.cons.push_back(Connection {
					0, // TODO Weight
					src
				});
			}
		} else if (prob_dist(gen) < NODE_CHANCE) {
			// std::cout << "Adding new node\n";
			std::uniform_int_distribution<> out_distrib{0, num_output - 1};
			nodes.push_back(Node{});
			select_dst_node(gen).cons.push_back(Connection {
				1, // TODO Bias
				num_input + nodes.size() - 1
			});
		}
	}

	// First N will be inputs, then the next amount will be outputs
	void serialize(std::ostream& os) const;

	friend std::istream& operator>>(std::istream& is, Robot& rob) {
		for (auto& n : rob.inputs) {
			is >> n;
		}

		for (auto& n : rob.outputs) {
			is >> n;
		}

		while (true) {
			// Attempt to read in the next node
			Node n;
			is >> n;

			// If we hit the end or something is wrong or we couldn't read another node,
			// then we're done
			if (is.eof() || is.bad() || is.fail()) {
				is.clear();
				break;
			} else {
				rob.nodes.push_back(n);
			}
		}

		return is;
	}

	void node_path(std::ostream& os, const Node& n, size_t depth) const;

	friend std::ostream& operator<<(std::ostream& os, const Robot& rob) {
		os << std::setprecision(2);
		for (int i = 0; i < rob.outputs.size(); ++i) {
			rob.node_path(os, rob.outputs[i], 0);
			os << "\n";
		}

		return os;
	}
};

void test_robot_serial();

void evolution();