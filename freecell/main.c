#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "graphics.h"

#define MAIN_PILE_COUNT 8

// TODO this should be dynamic for scaling
#define STACKING_OFFSET 24

// TODO make sure card_height and card_width include the margins -
// So cards being spaced out will be purely cosmetic; the logical
// cards will be touching. This will make math easier.

typedef enum { SUIT_NONE, SUIT_SPADE, SUIT_CLUB, SUIT_HEART, SUIT_DIAMOND } Suit;
typedef struct {
    int rank;
    Suit suit;
} Card;

// GLOBAL VARS
// ----------------------------------------

Card CARD_NONE = {0};

// Resources
Texture tex_card_back;
Texture tex_card_front;
Texture tex_card_suits;
Texture tex_card_text;

// HELPER FUNCTIONS
// ----------------------------------------

int pile_len(Card pile[]) {
    int i = 0;
    while (i < 52 && pile[i].suit != SUIT_NONE) i += 1;
    return i;
}

void shuffle(Card deck[], int num_cards) {
    if (num_cards > 1) {
        for (int i = 0; i < num_cards - 1; i++) {
            int j = i + rand() / (RAND_MAX / (num_cards - i) + 1);
            Card card = deck[j];
            deck[j] = deck[i];
            deck[i] = card;
        }
    }
}

// HELPER PROCS
// ----------------------------------------

void draw_card(Card card, Rect rect) {
    Rect suit_src_rect = {
        tex_card_suits.height * (card.suit - 1),
        0,
        tex_card_suits.width / 4,
        tex_card_suits.height,
    };
    Rect suit_dest_rect = {
        rect.x + 5,
        rect.y + 5,
        16,
        16,
    };
    Rect rank_src_rect = {
        tex_card_text.width * (card.rank - 1) / 13,
        tex_card_text.height * ((card.suit - 1) / 2) / 2,
        tex_card_text.width / 13,
        tex_card_text.height / 2,
    };
    Rect rank_dest_rect = {
        rect.x + 30,
        rect.y + 5,
        20,
        20,
    };
    draw_texture(tex_card_front, rect);
    draw_partial_texture(tex_card_suits, suit_src_rect, suit_dest_rect);
    draw_partial_texture(tex_card_text, rank_src_rect, rank_dest_rect);
}

// MAIN
// ----------------------------------------

int main(int argc, char **argv) {
    srand(time(NULL));
    assert(graphics_init("Freecell", 800, 600));

    tex_card_back = load_texture("../res/card_back.png");
    tex_card_front = load_texture("../res/card_front.png");
    tex_card_suits = load_texture("../res/suits_16.png");
    tex_card_text = load_texture("../res/text.png");

    Texture font = load_font("../res/Vera.ttf");

    // Game Data
    Card deck[52];
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 0; rank < 13; rank++) {
            deck[suit * 13 + rank] = (Card){rank + 1, suit + 1};
        }
    }
    shuffle(deck, 52);
    Card piles[MAIN_PILE_COUNT][52] = {0};
    for (int i = 0; i < 52; i++) {
        piles[i % MAIN_PILE_COUNT][i / MAIN_PILE_COUNT] = deck[i];
    }

    Card free_cells[4] = {0};
    Card destination_cells[4] = {0};
    Card held_pile[52] = {0};

    int last_active_pile = 0;

    // Geometry
    int card_width, card_height;
    {
        int width = get_screen_width();
        card_width = width / MAIN_PILE_COUNT - 2;
        card_height = card_width * tex_card_front.height / tex_card_front.width;
    }
    int mouse_offset_x = 0, mouse_offset_y = 0;

    // Platform state
    int mouse_x = 0, mouse_y = 0;
    bool mouse_just_pressed = false;
    bool mouse_just_released = false;
    SDL_Event event;
    bool quit = false;

    int dbg_active_pile_idx;
    int dbg_active_card_idx;
    int dbg_active_freecell_idx;
    int dbg_active_destination_idx;

    // Main loop
    // ========================================
    while (!quit) {

        // Update game state
        // ========================================
        SDL_GetMouseState(&mouse_x, &mouse_y);

        dbg_active_pile_idx = -1;
        dbg_active_card_idx = -1;
        dbg_active_freecell_idx = -1;
        dbg_active_destination_idx = -1;

        if (mouse_y > card_height) {
            // We're in the main piles
            int active_pile = mouse_x / card_width;
            int pile_size = 0;
            while (piles[active_pile][pile_size].suit != SUIT_NONE) pile_size += 1;
            int active_card = (mouse_y - card_height) / STACKING_OFFSET;
            active_card = active_card > pile_size - 1 ? pile_size - 1 : active_card;

            if (mouse_just_released) {
                mouse_just_released = false;

                // Check if we can legally place this card on the current stack
                bool can_place = true;
                Card last_card = CARD_NONE;
                int i = 1;
                while (piles[active_pile][i].suit != SUIT_NONE) i += 1;
                Card dest = piles[active_pile][i - 1];
                Card src = held_pile[0];

                if (src.suit / 3 == dest.suit / 3) can_place = false;
                if (src.rank != dest.rank - 1) can_place = false;

                // Put held cards back onto the last pile
                int start = 0;
                int target_pile = can_place ? active_pile : last_active_pile;
                if (target_pile >= 8) {
                    free_cells[target_pile - 8] = held_pile[0];
                    held_pile[0] = CARD_NONE;
                } else {
                    while (piles[target_pile][start].suit != SUIT_NONE) {
                        start += 1;
                    }
                    for (int i = 0; held_pile[i].suit != SUIT_NONE; i++) {
                        piles[target_pile][start + i] = held_pile[i];
                        held_pile[i] = CARD_NONE;
                    }
                }
            }

            // TODO Move mouse_just_pressed higher so this doesn't get
            // triggered by dragging the mouse into the target zone
            if (mouse_just_pressed) {
                mouse_just_pressed = false;

                bool can_pick_up = true;
                int len = 0;
                for (int i = active_card; piles[active_pile][i].suit != SUIT_NONE; i++) len += 1;
                if (len > 1) {
                    for (int i = active_card; i < active_card + len - 1; i++) {
                        Card this = piles[active_pile][i];
                        Card next = piles[active_pile][i + 1];
                        if (next.suit / 3 == this.suit / 3) can_pick_up = false;
                        if (next.rank != this.rank - 1) can_pick_up = false;
                    }
                }

                if (can_pick_up) {
                    for (int i = active_card; piles[active_pile][i].suit != SUIT_NONE; i++) {
                        held_pile[i - active_card] = piles[active_pile][i];
                        piles[active_pile][i] = CARD_NONE;
                    }
                }
                dbg_active_pile_idx = active_pile;
                dbg_active_card_idx = active_card;
                last_active_pile = active_pile; // TODO does this go in the if (can_pick_up) or not?
            }

        } else if (mouse_x < card_width * 4) {
            // We're in the free cells
            int active = mouse_x / card_width;
            if (mouse_just_pressed) {
                mouse_just_pressed = false;
                if (free_cells[active].suit != SUIT_NONE) {
                    held_pile[0] = free_cells[active];
                    free_cells[active] = CARD_NONE;
                }
                last_active_pile = active + 8;
            }
            if (mouse_just_released) {
                mouse_just_released = false;
                if (free_cells[active].suit == SUIT_NONE && pile_len(held_pile) == 1) {
                    free_cells[active] = held_pile[0];
                    held_pile[0] = CARD_NONE;
                }
            }
            dbg_active_freecell_idx = active;

        } else {
            // We're in the destination cells
            int active = (mouse_x - get_screen_width() / 2) / card_width;

            // TODO we maybe don't need this because we can't move cards out of
            // the destination cells, but should we have it here anyway to
            // maintain the correct mouse state?
            if (mouse_just_pressed) {
                mouse_just_pressed = false;
            }

            if (mouse_just_released) {
                mouse_just_released = false;

                bool can_place = false;
                if (pile_len(held_pile) == 1) {
                    can_place = true;
                    if (destination_cells[active].suit != SUIT_NONE &&
                            destination_cells[active].suit != held_pile[0].suit) {
                        can_place = false;
                    }
                    if (destination_cells[active].rank != held_pile[0].rank - 1) {
                        can_place = false;
                    }
                }

                if (can_place) {
                    destination_cells[active] = held_pile[0];
                    held_pile[0] = CARD_NONE;
                } else {
                    if (last_active_pile >= 8) {
                        free_cells[last_active_pile - 8] = held_pile[0];
                        held_pile[0] = CARD_NONE;
                    } else {
                        int start = 0;
                        while (piles[last_active_pile][start].suit != SUIT_NONE) {
                            start += 1;
                        }
                        for (int i = 0; held_pile[i].suit != SUIT_NONE; i++) {
                            piles[last_active_pile][start + i] = held_pile[i];
                            held_pile[i] = CARD_NONE;
                        }
                    }
                }
            }
            dbg_active_destination_idx = active;
        }

        // Handle events
        // ========================================
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_FINGERDOWN:
                    mouse_just_pressed = true;
                    break;
                case SDL_MOUSEBUTTONUP:
                case SDL_FINGERUP:
                    mouse_just_released = true;
                    break;
                case SDL_KEYDOWN:
                    {
                        SDL_Keycode code = event.key.keysym.sym;
                        switch (code) {
                            case SDLK_q:
                                quit = true;
                                break;
                        }
                    }
                    break;
            }
        }

        // Draw
        // ========================================
        clear_screen(64, 128, 64, 255);

        if (dbg_active_freecell_idx >= 0) {
            Rect rect = {
                dbg_active_freecell_idx * card_width,
                0,
                card_width,
                card_height,
            };
            Color color = {0, 255, 0, 255};
            draw_rect(rect, color);
        }
        if (dbg_active_destination_idx >= 0) {
            Rect rect = {
                card_width * 4 + dbg_active_destination_idx * card_width,
                0,
                card_width,
                card_height,
            };
            Color color = {0, 0, 255, 255};
            draw_rect(rect, color);
        }

        for (int i = 0; i < 4; i++) {
            if (free_cells[i].suit != SUIT_NONE) {
                Rect rect = {
                    i * card_width,
                    0,
                    card_width,
                    card_height,
                };
                draw_card(free_cells[i], rect);
            }
        }

        for (int i = 0; i < 4; i++) {
            if (destination_cells[i].suit != SUIT_NONE) {
                Rect rect = {
                    (i + 4) * card_width,
                    0,
                    card_width,
                    card_height,
                };
                draw_card(destination_cells[i], rect);
            }
        }

        for (int x = 0; x < MAIN_PILE_COUNT; x++) {
            for (int y = 0; y < 52; y++) {
                if (piles[x][y].suit == SUIT_NONE) {
                    break;
                }
                Rect rect = {
                    x * (card_width + 2),
                    card_height + y * STACKING_OFFSET,
                    card_width,
                    card_height,
                };
                draw_card(piles[x][y], rect);
            }
        }

        for (int i = 0; i < 52; i++) {
            if (held_pile[i].suit == SUIT_NONE) {
                break;
            }
            Rect rect = {
                mouse_x,
                mouse_y + i * STACKING_OFFSET,
                card_width,
                card_height,
            };
            draw_card(held_pile[i], rect);
        }

        Rect rect = {0, 0, 256, 256};
        draw_text(font, rect, rect);

        graphics_swap();
    }


    graphics_free();
}
