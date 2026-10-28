#pragma once

#include <iostream>
#include <array>
#include <vector>

#include "cardpile.hpp"

template <typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr) {
	for (auto&& elem : arr) {
		os << elem << ", ";
	}

	return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& arr) {
	for (auto&& elem : arr) {
		os << elem << ", ";
	}

	return os;
}

struct Board {
	private:
	Card get_board_card(size_t idx) const;
	Card& get_board_card(size_t idx);

	Card get_ace_card(size_t idx) const;
	Card& get_ace_card(size_t idx);

	Card get_deck_card() const;
	Card& get_deck_card();

	Card get_flip_card() const;
	Card& get_flip_card();

	void draw_from_deck();

	void flip_card(size_t idx);

	void pop_card(size_t idx);

	void move_card(size_t src, size_t dst);

	bool has_won() const;
	size_t num_ace_cards() const;
	size_t num_face_up() const;

	public:
	const Card EMPTY_CARD{};

	static constexpr size_t COLS = 7;
	static constexpr size_t DECK_IDX = 0;
	static constexpr size_t FLIP_IDX = DECK_IDX + 1;
	static constexpr size_t ACE_IDX = FLIP_IDX + 1;
	static constexpr size_t BOARD_IDX = ACE_IDX + 4;
	// Outside the bounds, the last real index is this -1
	static constexpr size_t END_IDX = BOARD_IDX + COLS;

	std::array<std::vector<Card>, COLS> main_board{};
	std::array<std::vector<Card>, 4> aces = {std::vector<Card>{EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}, {EMPTY_CARD}};
	Deck draw_pile{};
	std::vector<Card> flipped{};

	void deal(Deck& deck);

	Card cindex(size_t idx) const;
	Card index(size_t idx) const;
	Card& index(size_t idx);

	std::vector<Card> column(size_t idx) const;
	std::vector<Card>& column(size_t idx);

	bool is_legal_move(size_t idx_a, size_t idx_b) const;

	void perform_move(size_t idx_a, size_t idx_b);

	double score() const;

	friend std::ostream& operator<<(std::ostream& os, const Board& board) {
		size_t col = 0;

		const auto print = [&]() {
			os << col++ << ": ";
		};

		print();
		if (auto deck_top = board.draw_pile.view_top_card()) {
			auto card = deck_top.value();
			card.face_down = true;
			os << card;
		} else {
			os << board.EMPTY_CARD;
		}
		os << "\n";

		print();
		if (!board.flipped.empty()) {
			auto card = board.flipped[board.flipped.size() - 1];
			os << card;
		} else {
			os << board.EMPTY_CARD;
		}
		os << "\n";

		for (auto&& card : board.aces) {
			print();
			os << card.back() << "\n";
		}

		// Prints the board sideways but whatever
		for (auto&& column : board.main_board) {
			print();
			for (auto&& card : column) {
				os << card << " ";
			}
			os << "\n";
		}

		return os;
	}
};
