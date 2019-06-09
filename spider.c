#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graphics.h"

int can_pick_up(Stack *src, int idx) {
    /* Don't allow moving facedown cards */
    if (src->cards[idx].orientation == 0) {
        return false;
    }
    Card *prev_card;
    Card *curr_card;
    for (int i = idx; i < src->num_cards - 1; i++) {
        prev_card = &src->cards[i];
        curr_card = &src->cards[i + 1];
        if (curr_card->suit != prev_card->suit
                || curr_card->rank != prev_card->rank - 1) {
            return false;
        }
    }
    return true;
}

/* Whether or not a card stack can be set down */
int can_place(Stack *src, Stack *dst) {
    int src_val = src->cards[0].rank;
    int dst_val = dst->cards[dst->num_cards - 1].rank;
    return (src_val == dst_val - 1);
}

int main(int argc, char* argv[])
{
    /* Seed the random number generator */
    srand(time(NULL));

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO != 0)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    /* Initialize SDL Image library */
    IMG_Init(IMG_INIT_PNG);

    Graphics graphics;
    if (graphics_init(&graphics, "Spider Solitaire") != 0) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    SDL_Event event;

    int num_stacks = 10;
    Card *deck = malloc(sizeof(Card) * 104);
    Stack *stacks = malloc(sizeof(Stack) * num_stacks);

    update_graphics(&graphics, num_stacks);

    // Create a deck from two standard 52-card decks of cards.
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 0; rank < 13; rank++) {
            Card card;
            card.suit = suit;
            card.rank = rank;
            card.orientation = 1;
            deck[suit * 13 + rank] = card;
            deck[52 + suit * 13 + rank] = card;
        }
    }
    shuffle(deck, 104);

    /* Create stacks */
    for (int i = 0; i < num_stacks; i++) {
        stacks[i].num_cards = 0;
        stacks[i].rect = make_rect(
                graphics.width / num_stacks * i,
                graphics.card_h,
                graphics.width / num_stacks,
                graphics.height - graphics.card_h);
    }

    /* Populate stacks */

    /* Facedown cards */
    for (int i = 0; i < 34; i++) {
        deck[i].orientation = 0;
        Stack *stack = &stacks[i % num_stacks];
        stack->cards[stack->num_cards] = deck[i];
        stack->num_cards++;
    }
    /* Faceup cards */
    for (int i = 34; i < 44; i++) {
        Stack *stack = &stacks[i % num_stacks];
        stack->cards[stack->num_cards] = deck[i];
        stack->num_cards++;
    }

    MouseTarget target;
    Stack mouse_stack;
    mouse_stack.num_cards = 0;
    mouse_stack.rect = make_rect(0, 0, graphics.card_w, graphics.height);
    update_mouse_stack(&graphics, &mouse_stack);

    int src_stack_idx = 0;
    int dst_stack_idx = 0;

    while (!quit) {
        SDL_WaitEvent(&event);

        target = get_mouse_target(&graphics, &stacks, num_stacks);
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (can_pick_up(&stacks[target.stack], target.card)) {
                    src_stack_idx = target.stack;
                    move_stack(&stacks[target.stack], &mouse_stack, target.card);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                dst_stack_idx = target.stack;
                if (can_place(&mouse_stack, &stacks[target.stack])) {
                    move_stack(&mouse_stack, &stacks[target.stack], 0);
                } else {
                    move_stack(&mouse_stack, &stacks[src_stack_idx], 0);
                }
                if (dst_stack_idx != src_stack_idx) {
                    stacks[src_stack_idx]
                        .cards[stacks[src_stack_idx].num_cards - 1]
                        .orientation = 1;
                }
                break;
        }

        update_graphics(&graphics, num_stacks);
        update_mouse_stack(&graphics, &mouse_stack);

        for (int i = 0; i < num_stacks; i++) {
            stacks[i].rect = make_rect(
                    graphics.width / num_stacks * i,
                    graphics.card_h,
                    graphics.width / num_stacks,
                    graphics.height - graphics.card_h);
            draw_stack(&graphics, &stacks[i]);

        }

        draw_stack(&graphics, &mouse_stack);
        SDL_RenderPresent(graphics.renderer);
    }

    /* Clean up */
    free(deck);
    free(stacks);
    graphics_free(&graphics);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
