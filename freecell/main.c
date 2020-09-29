#include "graphics.h"
#include "stdbool.h"
#include "stdlib.h"
#include "time.h"
#include "assert.h"

#define MAIN_PILE_COUNT 8

// TODO this should be dynamic for scaling
#define STACKING_OFFSET 20

// TODO make sure card_height and card_width include the margins -
// So cards being spaced out will be purely cosmetic; the logical
// cards will be touching. This will make math easier.

typedef enum { SUIT_NONE, SUIT_SPADE, SUIT_CLUB, SUIT_HEART, SUIT_DIAMOND } Suit;
typedef struct {
    int rank;
    Suit suit;
} Card;

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

int main(int argc, char **argv) {
    srand(time(NULL));
    assert(graphics_init("Freecell", 800, 600));

    // Resources
    Texture tex_card_back = load_texture("res/card_back.png");
    Texture tex_card_front = load_texture("res/card_front.png");
    Texture tex_card_suits = load_texture("res/suits.png");
    Texture tex_card_text = load_texture("res/text.png");

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
    bool is_mouse_down = false;
    SDL_Event event;
    bool quit = false;

    int dbg_active_pile_idx;
    int dbg_active_card_idx;

    // Main loop
    // ========================================
    while (!quit) {

        // Update game state
        // ========================================
        SDL_GetMouseState(&mouse_x, &mouse_y);

                            if (mouse_y > card_height) {
                                // We're in the main piles
                                int active_pile = mouse_x / card_width;
                                int active_card = (mouse_y - card_height) / STACKING_OFFSET;
                                for (int i = active_card; i < 52; i++) {
                                    printf("i: %d\n", i);
                                    printf("active pile: %d\n", active_pile);
                                    printf("mouse_x: %d\n", mouse_x);
                                    if (piles[active_pile][i].suit != SUIT_NONE) {
                                        held_pile[i - active_card] = piles[active_pile][i];
                                    }
                                }
                                dbg_active_pile_idx = active_pile;
                                dbg_active_card_idx = active_card;
                            } else {
                                // We're in either the freecells or the destination cells
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
                    // This check avoids sending duplicate touch and click events
                    {
                        if (!is_mouse_down) {
                            is_mouse_down = true;

                        }
                    }
                case SDL_MOUSEBUTTONUP:
                case SDL_FINGERUP:
                    is_mouse_down = false;
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

        // Render
        // ========================================
        clear_screen(64, 128, 64, 255);
        for (int x = 0; x < MAIN_PILE_COUNT; x++) {
            for (int y = 0; y < 52; y++) {
                if (piles[x][y].suit != SUIT_NONE) {
                    Rect rect = {
                        x * (card_width + 2),
                        card_height + y * STACKING_OFFSET,
                        card_width,
                        card_height,
                    };
                    Rect suit_src_rect = {
                        tex_card_suits.height * (piles[x][y].suit - 1),
                        0,
                        tex_card_suits.width / 4,
                        tex_card_suits.height,
                    };
                    Rect suit_dest_rect = {
                        rect.x + 5,
                        rect.y + 5,
                        20,
                        20,
                    };
                    Rect rank_src_rect = {
                        tex_card_text.width * (piles[x][y].rank - 1) / 13,
                        tex_card_text.height * ((piles[x][y].suit - 1) / 2) / 2,
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
                    if (x == dbg_active_pile_idx && y == dbg_active_card_idx) {
                        draw_texture(tex_card_back, rect);
                    }
                    draw_partial_texture(tex_card_suits, suit_src_rect, suit_dest_rect);
                    draw_partial_texture(tex_card_text, rank_src_rect, rank_dest_rect);
                    /* draw_texture(tex_card_suits, suit_dest_rect); */
                }
            }
        }
        graphics_swap();

    }

    graphics_free();
}
