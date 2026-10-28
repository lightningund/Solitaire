- Legal move types:
	1) Opposing suit card on to next higher number:
		- Conditions:
			- Suit colors don't match
			- Moving card has a rank one lower than the Destination card
			- Moving card is face up
			- Destination card doesn't have any other cards below it
			- Destination card is face up
		- Side effects:
			- If the card behind the moving card is face down, flip it up
			- If the moving card has other cards below it, move them too
	2) King to empty column:
		- Conditions:
			- Moving card is a King
			- Moving card is face up
			- Destination column is empty
		- Side effects:
			- If the card behind the moving card is face down, flip it up
			- If the moving card has other cards below it, move them too
	3) Ace to empty Ace Slot:
		- Conditions:
			- Moving card is an Ace
			- Moving card is face up
			- Ace slot is empty
		- Side effects:
			- If the card behind the moving card is face down, flip it up
	4) Card to Ace Slot:
		- Conditions:
			- Suits are the same
			- Moving card has a rank one higher than the Destination card
			- Moving card is face up
			- Moving card has no other cards below it
		- Side effects:
			- If the card behind the moving card is face down, flip it up
	5) Turn over card from deck:
		- Conditions:
			- There must be at least one card in the draw deck
		- Side effects:
			- N/A
	6) Put flipped pile back into deck:
		- Conditions:
			- The draw deck must be empty
			- There must be at least one card in the flipped pile
		- Side effects:
			- N/A

- Column-based check plan:
	- `src` on deck:
		- Second idx doesn't matter
		- Can only be 5) or 6)
	- `dst` on board:
		- Get the bottom card and store it
			- Pretend empty is a rank above king or something?
		- `src` on board:
			- Search for a card in the stack that is one rank below and an opposite suit
		- `src` on ace/flip pile:
			- Check if the top card of the pile works on the bottom of the stack
	- `dst` on ace:
		- `src` on board/flip pile:
			- Check if the top card of the pile works on the bottom of the stack

- Check order plan:
	- First click on deck:
		- Second idx doesn't matter
		- Can only be 5) or 6)
	- Otherwise:
		- Instant DQs:
			- Moving column is empty
			- Destination is deck
			- Destination is flip pile
		- If the destination is on the board:
			- If destination column is empty:
				- Instant DQs:
					- Moving column does not have a king
				- return (king_idx, 0)
			- Otherwise:
				- Instant DQs:
					- Destination is in the top row
					- Card one above the destination is empty
					- Card one above the destination is not the opposite color suit
					- The moving card is not one rank lower than the card one above the destination
		- Otherwise (ace destination):
			- Instant DQs:
				- Moving card has cards under it
			- If moving card is an ace:
				- Instant DQs:
					- Destination is not empty
			- Otherwise:
				- Instant DQs:
					- Destination is empty
					- The moving card is not one rank higher than the destination
					- The moving card does not match the suit of the destination

# Move Performing
- Legal move types:
	1) Opposing suit card on to next higher number:
		- Effects:
			- The moving card goes to the indicated slot
			- If the card behind the moving card is face down, flip it up
			- If the moving card has other cards below it, move them too
	2) King to empty column:
		- Effects:
			- The moving card goes to the indicated slot
			- If the card behind the moving card is face down, flip it up
			- If the moving card has other cards below it, move them too
	3) Ace to empty Ace Slot:
		- Effects:
			- The moving card goes to the indicated slot
			- If the card behind the moving card is face down, flip it up
	4) Card to Ace Slot:
		- Effects:
			- The moving card goes to the indicated slot
			- If the card behind the moving card is face down, flip it up
	5) Turn over card from deck:
		- Effects:
			- The top card of the draw pile is put into the flip pile
	6) Put flipped pile back into deck:
		- Effects:
			- The entire flip pile is put back into the draw pile

- Check Plan:
	- If the source is the deck
		- Either draw or reset it
	- Move the card to the destination
	- If the source was an ace or the flip pile
		- Pop it off the end
	- If the source was on the board
		- Check if there is a card behind to flip
		- If the destination was also on the board
			- Move any cards below



```c++
#define JOIN_EXPAND(a, b)        a##b
#define JOIN(a, b)               JOIN_EXPAND(a, b)

#define SECOND_EXPAND(a, b, ...) b
#define SECOND(...)              EXPAND(SECOND_EXPAND(__VA_ARGS__))

#define HIDDEN0                  unused, 0
#define CHECK0(value)            SECOND(JOIN(HIDDEN, value), 1, unused)

#define HELLO0                   std::cout << "Hello"
#define HELLO1
#define HELLO(value)             JOIN(HELLO, CHECK0(value))
```

`#define MIN_ONE(a, ...) a` being called with just one argument works on every compiler I tested on godbolt. But apparently it is a restriction in some versions of C/C++