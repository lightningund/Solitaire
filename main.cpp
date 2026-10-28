// GOALS:
// X Shuffle up a deck
// X Deal it out
// X Display it in the terminal
// X Allow selecting an index and return the legal moves for it
// X Move performing
// ~ Win checking

// Threefold repeat checking(?)

// ~ NEAT algorithm
// X Connect it to solitaire
// X Save the progress over time for later viewing
// ~ Animate the AI playing solitaire

// 7x19 for the main board
// 4 aces
// deck
// turned card

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

#include "cardpile.hpp"
#include "solitaire.hpp"
#include "robot.hpp"

void console_player(Board& board) {
	while (true) {
		std::cout << board << "\n";

		size_t idx_a;
		std::cin >> idx_a;

		std::cout << idx_a << ": " << board.column(idx_a) << "\n";

		size_t idx_b;
		std::cin >> idx_b;

		std::cout << idx_b << ": " << board.column(idx_b) << "\n";

		std::cout << board.is_legal_move(idx_a, idx_b) << "\n";

		board.perform_move(idx_a, idx_b);
	}
}

template <typename RNG>
void random_player(RNG& gen, Board& board) {
	std::uniform_int_distribution<> idx_distrib{0, Board::END_IDX - 1};

	for (int i = 0; i < 100; ++i) {
		size_t tries = 0;
		size_t idx_a = 0;
		size_t idx_b = 0;
		do {
			idx_a = idx_distrib(gen);
			idx_b = idx_distrib(gen);
			++tries;

			std::cout << tries << "\r";
		} while (!board.is_legal_move(idx_a, idx_b) && tries < 5000);

		// If we tripped the failsafe but can't draw just give up
		if (!board.is_legal_move(idx_a, idx_b)) {
			break;
		}

		std::cout << "Tries: " << tries << ", indices: " << idx_a << ", " << idx_b << "\n";
		board.perform_move(idx_a, idx_b);
		std::cout << board << "\n";
	}
}

template <typename RNG>
void run_robot(Robot& robot, Board& board, size_t num_turns, RNG& gen) {
	std::uniform_int_distribution<> idx_distrib{0, Board::END_IDX - 1};

	// If the robot node value is out of bounds
	auto out_of_bounds = [](double value) {
		return value > (double)(Board::END_IDX - 1) || value < 0;
	};

	// // Incl. Randomness
	// auto pick_move = [&](double value) -> size_t {
	// 	// If the robot outputs out of range, just pick a random value
	// 	// This might not be the best for learning, but it will make things more interesting
	// 	if (out_of_bounds(value)) {
	// 		return idx_distrib(gen);
	// 	} else {
	// 		return std::clamp(value, 0.0, (double)(Board::END_IDX - 1));
	// 	}
	// };

	// // Just clamps
	// auto pick_move = [&](double value) -> size_t {
	// 	return std::clamp(value, 0.0, (double)(Board::END_IDX - 1));
	// };

	// Modulo
	auto pick_move = [&](double value) -> size_t {
		auto modu = std::fmod(value, (double)(Board::END_IDX) - 1.0);
		if (modu < 0) modu += (double)(Board::END_IDX);
		return modu;
	};

	// std::cout << robot << "\n";

	for (int i = 0; i < num_turns; ++i) {
		size_t idx_a;
		size_t idx_b;
		size_t tries = 0;

		std::vector<double> board_arr{};

		for (size_t i = 0; i < Board::BOARD_IDX; ++i) {
			board_arr.push_back(board.column(i).back().get_rep_num());
		}

		const size_t MAX_ROWS = Board::COLS + 13 - 1;

		// We pad the board columns to ensure a consistent size and layout
		for (size_t i = Board::BOARD_IDX; i < Board::END_IDX; ++i) {
			auto col = board.column(i);
			for (size_t j = 0; j < MAX_ROWS; ++j) {
				if (j < col.size()) {
					board_arr.push_back(col[j].get_rep_num());
				} else {
					board_arr.push_back(EMPTY_REP);
				}
			}
		}

		board_arr.push_back(tries);

		do {
			board_arr.back() = tries++;
			// std::cout << board_arr << "\n";

			robot.set_inputs(board_arr);

			// std::cout << "\r";
			// for (auto& n : robot.nodes) {
			// 	n.serialize(std::cout);
			// }

			robot.backprop();

			idx_a = pick_move(robot.outputs[0].value);
			idx_b = pick_move(robot.outputs[1].value);

			// std::cout << "Raw: " << robot.outputs << "\n";
			// std::cout << "Clamped: " << idx_a << ", " << idx_b << "\n";

		} while(!board.is_legal_move(idx_a, idx_b) && tries < 5000);

		// std::cout << "Indices: " << idx_a << ", " << idx_b << "| Tries: " << tries << "\n";
		if (board.is_legal_move(idx_a, idx_b)) {
			board.perform_move(idx_a, idx_b);
		}
		// std::cout << board << "\n";
	}

	// std::cout << board << "\n";
}

template <typename RNG>
void robo_player(RNG& gen, Board& board) {
	Robot robot{gen};

	run_robot(robot, board, 100, gen);
}

// Assumes the robots list is already sorted
template <typename RNG>
void duplicate_best_bots(RNG& gen, std::vector<Robot>& robots, size_t duplicates, size_t saved) {
	for (size_t i = 0; i < saved; ++i) {
		for (size_t j = 0; j < duplicates; ++j) {
			robots[i + j * duplicates] = robots[i];
			robots[i + j * duplicates].mutate(gen);
		}
	}
}

void evolution() {
	const size_t pop_size = 100;
	// How many times we duplicate the surviving members
	// This dictates the survival percentage
	// So 2 would be the top 50%, and 10 would be the top 10%
	const size_t duplicates = 10;
	const double cutoff = 1.0 / duplicates;
	const size_t num_saved = pop_size * cutoff;

	// The number of different board deals we try on each robot each time
	const size_t num_runs = 10;

	const size_t turns = 10;
	const size_t generations = 1;

	std::random_device random_dev;
	std::mt19937 gen(random_dev());

	std::vector<Robot> robots{};

	for (int i = 0; i < pop_size; ++i) {
		robots.push_back(Robot(gen));
		// robots.push_back(Robot{});
	}

	// Loop for number of generations
		// X Run each robot
		// Save some stats
		// X Filter the best
		// X Duplicate and mutate

	for (int generation = 0; generation < generations; ++generation) {
	// size_t generation = 0;
	// while (++generation) {
		// TODO: Parallelize
		for (size_t i = 0; i < pop_size; ++i) {
			auto& robot = robots[i];
			robot.score = 0;
			Board board;
			for (int j = 0; j < num_runs; ++j) {
				Deck deck{};
				deck.shuffle(gen);
				board.deal(deck);
				run_robot(robot, board, turns, gen);
				robot.score += board.score();
			}

			if (i % 10 == 0) {
				std::cout << 100.0 * (float)i / pop_size << "%\r";
				std::cout.flush();
			}
		}

		// Shuffle the population so that ones with the same score get moved around
		std::shuffle(robots.begin(), robots.end(), gen);

		std::sort(robots.begin(), robots.end(), [](auto& a, auto& b) {
			return a.score > b.score;
		});

		duplicate_best_bots(gen, robots, duplicates, num_saved);

		std::cout << "Best robot of generation " << generation << ": ";
		robots[0].serialize(std::cout);
		std::cout << "\nWith a score of: " << robots[0].score << "\n\n";
	}
}

int main() {
	// test_node_serial();
	// test_robot_serial();

	std::random_device rd;
	std::mt19937 g(rd());

	Deck deck{};
	deck.shuffle(g);

	Board board{};
	board.deal(deck);

	// console_player(board);

	// random_player(g, board);

	// robo_player(g, board);

	evolution();

	return 0;
}