#include <vector>
#include <iostream>
#include <span>
#include <sstream>

#include "robot.hpp"

// Let's do, say bias: f64, cons: [weight: f64, idx: u64, ...]X
// So like 10.3 4.01 12 -0.01 180X
// bias, then the weight and idx of each connection, delimiting with an X
void Node::serialize(std::ostream& os) const {
	os << bias << " ";

	for (auto con : cons) {
		os << con.weight << " " << con.backtarget << " ";
	}

	os << "X";
}

void test_node_serial() {
	Node n{};
	n.bias = 12.3;
	n.cons.push_back(Connection { 0.01, 180 });

	std::stringstream ss{};

	n.serialize(std::cout);
	std::cout << "\n";
	n.serialize(ss);
	ss.seekg(0, std::ios::beg);

	Node remade{};
	ss >> remade;

	remade.serialize(std::cout);
	std::cout << "\n";
}

void Robot::set_inputs(std::span<double> vals) {
	for (int i = 0; i < num_input; ++i) {
		inputs[i].value = vals[i];
		inputs[i].propagated = true;
	}
}

void Robot::backprop(Node& n) {
	if (n.propagated) return;

	n.value = n.bias;
	for (auto& con : n.cons) {
		auto back = get_src_node(con.backtarget);
		if (!back.propagated) backprop(back);

		// TODO: Add activation function
		n.value += back.value * con.weight;
	}

	n.propagated = true;
}

void Robot::backprop() {
	for (auto& node : nodes) {
		node.propagated = false;
	}

	for (auto& node : outputs) {
		node.propagated = false;
		backprop(node);
	}
}

Node& Robot::get_src_node(size_t idx) {
	if (idx < num_input) {
		return inputs[idx];
	} else {
		return nodes[idx - num_input];
	}
}
const Node& Robot::get_src_node(size_t idx) const {
	if (idx < num_input) {
		return inputs[idx];
	} else {
		return nodes[idx - num_input];
	}
}

Node& Robot::get_dst_node(size_t idx) {
	if (idx < num_output) {
		return outputs[idx];
	} else {
		return nodes[idx - num_output];
	}
}
const Node& Robot::get_dst_node(size_t idx) const {
	if (idx < num_output) {
		return outputs[idx];
	} else {
		return nodes[idx - num_output];
	}
}

// First N will be inputs, then the next amount will be outputs
void Robot::serialize(std::ostream& os) const {
	for (auto& n : inputs) {
		n.serialize(os);
	}

	for (auto& n : outputs) {
		n.serialize(os);
	}

	for (auto& n : nodes) {
		n.serialize(os);
	}
}

void Robot::node_path(std::ostream& os, const Node& n, size_t depth) const {
	os << n.bias;
	for (auto&& con : n.cons) {
		os << "\n" << std::string(depth + 1, '\t');
		node_path(os, get_src_node(con.backtarget), depth + 1);
	}
}

void test_robot_serial() {
	std::random_device rd;
	std::mt19937 g(rd());

	Robot robot{g};

	std::stringstream ss{};

	robot.serialize(std::cout);
	std::cout << "\n";
	robot.serialize(ss);
	ss.seekg(0, std::ios::beg);

	Robot remade{g};
	ss >> remade;

	remade.serialize(std::cout);
	std::cout << "\n";

	for (int i = 0; i < 100; ++i) {
		robot.mutate(g);
	}

	std::stringstream ss2{};
	robot.serialize(std::cout);
	std::cout << "\n";
	robot.serialize(ss2);
	ss2.seekg(0, std::ios::beg);

	Robot remade2{g};
	ss2 >> remade2;

	remade2.serialize(std::cout);
	std::cout << "\n";
}