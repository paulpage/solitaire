#include <stdlib.h>

#include "cards.h"

void shuffle(Card *deck, size_t num_cards)
{
    if (num_cards > 1) {
        size_t i;
        for (i = 0; i < num_cards - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (num_cards - i) + 1);
            Card card = deck[j];
            deck[j] = deck[i];
            deck[i] = card;
        }
    }
}

