#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graphics.h"

int can_pick_up(Stack *src, int idx)
{
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
int can_place(Stack *src, Stack *dst)
{
    int src_val = src->cards[0].rank;
    int dst_val = dst->cards[dst->num_cards - 1].rank;
    return (src_val == dst_val - 1 || dst->num_cards == 0);
}

bool is_over_extra_stacks(Graphics *graphics, int num_extra_stacks)
{
    int offset = graphics->card_w / 8;
    int margin = graphics->card_w / 16;

    int x1 = margin;
    int x2 = margin + graphics->card_w + (offset * num_extra_stacks);
    int y1 = margin;
    int y2 = margin + graphics->card_h;

    int x = graphics->mouse_x;
    int y = graphics->mouse_y;
    return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

/*
 * Deals the next set of cards and returns the remaining number of stacks to be dealt.
 */
int deal_next_set(Stack stacks[], Stack extra_stacks[], int num_stacks, int num_extra_stacks)
{
    if (num_extra_stacks > 0) {
        int i = 0;
        Stack *xs = &extra_stacks[num_extra_stacks - 1];
        while (xs->num_cards > 0) {
            Stack *s = &stacks[i % num_stacks];
            s->cards[s->num_cards] = xs->cards[xs->num_cards - 1];
            s->cards[s->num_cards].orientation = 1;
            s->num_cards++;
            xs->num_cards--;
            i++;
        }
    }
    return num_extra_stacks - 1;
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

    // Number of stacks in the main play area
    int num_stacks = 10;
    // Number of stacks to be dealt from during play
    int num_extra_stacks = 5;
    // Number of stacks to put completed series
    int num_goal_stacks = 8;

    Card deck[104];
    Stack stacks[num_stacks];
    // TODO rename "extra"
    Stack extra_stacks[num_extra_stacks];
    Stack goal_stacks[num_goal_stacks];

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
    for (int i = 0; i < num_extra_stacks; i++) {
        extra_stacks[i].num_cards = 0;
    }
    // TODO: do this for other stacks

    /* Populate stacks */

    /* Facedown cards */
    int i;
    for (i = 0; i < 34; i++) {
        deck[i].orientation = 0;
        Stack *stack = &stacks[i % num_stacks];
        stack->cards[stack->num_cards] = deck[i];
        stack->num_cards++;
    }
    /* Faceup cards */
    for (i = 34; i < 44; i++) {
        Stack *stack = &stacks[i % num_stacks];
        stack->cards[stack->num_cards] = deck[i];
        stack->num_cards++;
    }
    /* Extra stacks */
    for (i = 45; i < 104; i++) {
        Stack *stack = &extra_stacks[i % num_extra_stacks];
        stack->cards[stack->num_cards] = deck[i];
        stack->cards[stack->num_cards].orientation = 0;
        stack->num_cards = stack->num_cards + 1;
    }

    /* goal stacks TODO: remove this (used for debugging) */
    for (i = 0; i < num_goal_stacks; i++) {
        Stack *stack = &goal_stacks[i % num_goal_stacks];
        stack->num_cards = 0;
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

        target = get_mouse_target(&graphics, stacks, num_stacks);
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (can_pick_up(&stacks[target.stack], target.card)) {
                    src_stack_idx = target.stack;
                    move_stack(&stacks[target.stack], &mouse_stack, target.card);
                } else if (is_over_extra_stacks(&graphics, num_extra_stacks)) {
                    num_extra_stacks = deal_next_set(stacks, extra_stacks, num_stacks, num_extra_stacks);
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

        for (int i = 0; i < num_extra_stacks; i++) {
            int offset = graphics.card_w / 8;
            int margin = graphics.card_w / 16;
            extra_stacks[i].rect = make_rect(
                    offset * i + margin,
                    margin,
                    graphics.width / num_stacks - (margin * 2),
                    graphics.card_h - (margin * 2));
            draw_card(&graphics, &extra_stacks[i].cards[0], &extra_stacks[i].rect);
        }

        for (int i = 0; i < num_goal_stacks; i++) {
            int offset = graphics.card_w / 8;
            int margin = graphics.card_w / 16;
            goal_stacks[i].rect = make_rect(
                    graphics.width - graphics.card_w - (offset * i),
                    margin,
                    graphics.width / num_stacks - (margin * 2),
                    graphics.card_h - (margin * 2));
            draw_card(&graphics, &goal_stacks[i].cards[0], &goal_stacks[i].rect);
        }

        draw_stack(&graphics, &mouse_stack);
        SDL_RenderPresent(graphics.renderer);
    }

    /* Clean up */
    graphics_free(&graphics);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
