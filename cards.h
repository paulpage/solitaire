#ifndef CARDS_H
#define CARDS_H

typedef enum { FACEDOWN, FACEUP } Orientation;

typedef struct {
    int suit;
    int rank;
    Orientation orientation;
} Card;

void shuffle(Card *deck, size_t num_cards); /* TODO: put in cards */

#endif
