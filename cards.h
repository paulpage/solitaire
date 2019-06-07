#ifndef CARDS_H
#define CARDS_H

typedef struct {
    int suit;
    int rank;
    int orientation;
} Card;

void shuffle(Card *deck, size_t num_cards); /* TODO: put in cards */

#endif
