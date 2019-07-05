#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>

#include "cards.h"

/* Dimensions of the suit and text sprites in pixels. */
#define SUIT_WIDTH 128
#define SUIT_HEIGHT 128
#define TEXT_WIDTH 256
#define TEXT_HEIGHT 128

/* Textures used to render cards */
typedef struct {
    SDL_Texture *back;
    SDL_Texture *front;
    SDL_Texture *suits;
    SDL_Texture *text;
} CardTextures;

typedef struct {
    /* Window dimensions */
    int width;
    int height;

    /* Space between cards */
    int margin;

    /* Mouse coordinates */
    int mouse_x;
    int mouse_y;

    /* Mouse position relative to the card it's holding */
    int mouse_offset_x;
    int mouse_offset_y;

    /* Card dimensions */
    int card_w;
    int card_h;

    /* SDL entities */
    SDL_Window *window;
    SDL_Renderer *renderer;
    CardTextures *textures;
} Graphics;

/* Card sprite dimensions in pixels. */
typedef struct {
    int w;
    int h;
} CardSize;

/* A pile of cards */
typedef struct {
    /* There will never be more than 2 decks of cards in a pile */
    Card cards[104];
    int num_cards;
    SDL_Rect rect;
} Pile;

/*
 * The indices of the pile and card currently targeted by the mouse.
 */
typedef struct {
    int pile;
    int card;
} MouseTarget;

int graphics_init(Graphics *graphics, char *name);
void graphics_free(Graphics *graphics);
SDL_Rect make_rect(int x, int y, int w, int h);
void draw_card(Graphics *graphics, Card *card, SDL_Rect *rect);
void draw_pile(Graphics *graphics, Pile *pile, SDL_Rect *rect);
void move_pile(Pile *srcpile, Pile *dstpile, int srcidx);
MouseTarget get_mouse_target(
        Graphics *graphics,
        Pile piles[],
        SDL_Rect rects[],
        int num_piles);
void set_norm_mouse_pos(Graphics *graphics, float x, float y);
void update_graphics(Graphics *graphics, int num_piles);
void set_mouse_target(
        Graphics *graphics,
        Pile *pile,
        SDL_Rect *pile_rect,
        Pile *mouse_pile,
        SDL_Rect *mouse_pile_rect,
        int card_idx);
void update_mouse_pile(Graphics *graphics, SDL_Rect *mouse_pile_rect);

#endif
