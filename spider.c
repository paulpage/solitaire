#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graphics.h"

/*
 * Whether or not a card pile can be picked up
 */
int can_pick_up(Pile *src, int idx)
{
    /* Don't allow moving facedown cards */
    if (src->cards[idx].orientation == FACEDOWN) {
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

/*
 * Whether or not a card pile can be set down
 */
int can_place(Pile *src, Pile *dst)
{
    int src_val = src->cards[0].rank;
    int dst_val = dst->cards[dst->num_cards - 1].rank;
    return (src_val == dst_val - 1 || dst->num_cards == 0);
}

/*
 * Whether or not the mouse is hovering over the deal piles
 */
bool is_over_deal_piles(Graphics *graphics, int num_deal_piles)
{
    int offset = graphics->margin * 2;
    /* int margin = graphics->card_w / 16; */

    int x1 = graphics->margin;
    int x2 = graphics->margin + graphics->card_w + (offset * num_deal_piles);
    int y1 = graphics->margin;
    int y2 = graphics->margin + graphics->card_h;

    int x = graphics->mouse_x;
    int y = graphics->mouse_y;
    return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

/*
 * Deals the next set of cards and returns the remaining number of piles to
 * be dealt.
 */
int deal_next_set(
        Pile piles[],
        Pile deal_piles[],
        int num_piles,
        int num_deal_piles)
{
    /* Don't deal if there are empty spaces */
    int i = 0;
    for (; i < num_piles; i++) {
        if (piles[i].num_cards == 0) {
            return num_deal_piles;
        }
    }
    if (num_deal_piles > 0) {
        i = 0;
        Pile *xs = &deal_piles[num_deal_piles - 1];
        for (; xs->num_cards > 0; i++) {
            Pile *s = &piles[i % num_piles];
            s->cards[s->num_cards] = xs->cards[xs->num_cards - 1];
            s->cards[s->num_cards].orientation = FACEUP;
            s->num_cards++;
            xs->num_cards--;
        }
    }
    return num_deal_piles - 1;
}

/*
 * Checks if the source pile contains a full series from king to ace of one 
 * suit. If so, it moves the series to the destination pile, and returns
 * true. Otherwise it returns false.
 */
int check_complete(Pile *srcpile, Pile *dstpile)
{
    for (int i = 0; i < srcpile->num_cards; i++) {
        if (srcpile->cards[i].rank == 12) {
            /* We found a king */
            int j = i + 1;
            int target_rank = 11;
            int target_suit = srcpile->cards[i].suit;
            /*
             * Crawl down the pile to see if it's complete and in
             * descending order
             */
            while (j < srcpile->num_cards) {
                if (srcpile->cards[j].rank != target_rank


                        || srcpile->cards[j].suit  != target_suit) {
                    break;
                }
                if (target_rank == 0) {
                    /* We made it to the ace */
                    move_pile(srcpile, dstpile, i);
                    if (srcpile->num_cards > 0) {
                        srcpile->cards[i - 1].orientation = FACEUP;
                    }
                    return true;
                }
                target_rank--;
                j++;
            }
        }
    }
    return false;
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
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

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

    int num_piles = 10; /* Number of piles in the main play area */
    int num_deal_piles = 5; /* Number of piles to be dealt from during play */
    int num_goal_piles = 8; /* Number of piles to put completed series */
    int num_completed_piles = 0; /* Number of series completed */

    Card deck[104];
    Pile piles[num_piles];
    Pile deal_piles[num_deal_piles];
    Pile goal_piles[num_goal_piles];

    update_graphics(&graphics, num_piles);

    /* Create a deck from two standard 52-card decks of cards. */
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 0; rank < 13; rank++) {
            Card card;
            card.suit = suit;
            card.rank = rank;
            card.orientation = FACEUP;
            deck[suit * 13 + rank] = card;
            deck[52 + suit * 13 + rank] = card;
        }
    }
    shuffle(deck, 104);

    /* Create piles */
    int i;
    for (i = 0; i < num_piles; i++) {
        piles[i].num_cards = 0;
        piles[i].rect = make_rect(
                graphics.width / num_piles * i,
                graphics.card_h,
                graphics.width / num_piles,
                graphics.height - graphics.card_h);
    }
    for (i = 0; i < num_deal_piles; i++) {
        deal_piles[i].num_cards = 0;
    }
    for (i = 0; i < num_goal_piles; i++) {
        goal_piles[i].num_cards = 0;
    }

    /* Populate piles */

    /* Facedown cards */
    i = 0;
    for (; i < 44; i++) {
        deck[i].orientation = FACEDOWN;
        Pile *pile = &piles[i % num_piles];
        pile->cards[pile->num_cards] = deck[i];
        pile->num_cards++;
    }
    /* Faceup cards */
    for (; i < 54; i++) {
        Pile *pile = &piles[i % num_piles];
        pile->cards[pile->num_cards] = deck[i];
        pile->num_cards++;
    }
    /* deal piles */
    for (; i < 104; i++) {
        Pile *pile = &deal_piles[i % num_deal_piles];
        pile->cards[pile->num_cards] = deck[i];
        pile->cards[pile->num_cards].orientation = FACEDOWN;
        pile->num_cards = pile->num_cards + 1;
    }

    MouseTarget target;
    Pile mouse_pile;
    mouse_pile.num_cards = 0;
    mouse_pile.rect = make_rect(0, 0, graphics.card_w, graphics.height);
    update_mouse_pile(&graphics, &mouse_pile);

    int src_pile_idx = 0;
    int dst_pile_idx = 0;

    while (!quit) {
        SDL_WaitEvent(&event);


        target = get_mouse_target(&graphics, piles, num_piles);
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_FINGERDOWN:
                if (can_pick_up(&piles[target.pile], target.card)) {
                    src_pile_idx = target.pile;
                    move_pile(&piles[target.pile], &mouse_pile, target.card);
                    set_mouse_target(&graphics, &piles[target.pile], &mouse_pile, target.card);
                } else if (is_over_deal_piles(&graphics, num_deal_piles)) {
                    num_deal_piles = deal_next_set(
                            piles,
                            deal_piles,
                            num_piles,
                            num_deal_piles);
                }
                break;
            case SDL_MOUSEBUTTONUP:
            case SDL_FINGERUP:
                dst_pile_idx = target.pile;
                if (can_place(&mouse_pile, &piles[target.pile])) {
                    move_pile(&mouse_pile, &piles[target.pile], 0);
                    if (check_complete(&piles[target.pile],
                                &goal_piles[num_completed_piles])) {
                        num_completed_piles++;
                        if (num_completed_piles == 8) {
                            printf("WIN!!!!");
                        }
                    }
                } else {
                    move_pile(&mouse_pile, &piles[src_pile_idx], 0);
                }
                if (dst_pile_idx != src_pile_idx) {
                    piles[src_pile_idx]
                        .cards[piles[src_pile_idx].num_cards - 1]
                        .orientation = FACEUP;
                }
                break;
            case SDL_FINGERMOTION:
                set_norm_mouse_pos(&graphics, event.tfinger.x, event.tfinger.y);
                break;
            case SDL_MOUSEMOTION:
                SDL_GetMouseState(&graphics.mouse_x, &graphics.mouse_y);
                break;

        }

        update_mouse_pile(&graphics, &mouse_pile);
        update_graphics(&graphics, num_piles);

        int offset = graphics.margin * 2;

        for (i = 0; i < num_piles; i++) {
            piles[i].rect = make_rect(
                    graphics.width / num_piles * i,
                    graphics.card_h,
                    graphics.width / num_piles,
                    graphics.height - graphics.card_h);
            draw_pile(&graphics, &piles[i]);
        }

        for (i = 0; i < num_deal_piles; i++) {
            deal_piles[i].rect = make_rect(
                    offset * i + graphics.margin,
                    graphics.margin,
                    graphics.width / num_piles - (graphics.margin * 2),
                    graphics.card_h - (graphics.margin * 2));
            draw_card(&graphics, &deal_piles[i].cards[0], &deal_piles[i].rect);
        }

        for (i = 0; i < num_goal_piles; i++) {
            if (goal_piles[i].num_cards > 0) {
                goal_piles[i].rect = make_rect(
                        graphics.width - graphics.card_w - (offset * i),
                        graphics.margin,
                        graphics.width / num_piles - (graphics.margin * 2),
                        graphics.card_h - (graphics.margin * 2));
                draw_card(
                        &graphics,
                        &goal_piles[i].cards[0],
                        &goal_piles[i].rect);
            }
        }

        draw_pile(&graphics, &mouse_pile);
        SDL_RenderPresent(graphics.renderer);
    }

    /* Clean up */
    graphics_free(&graphics);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
