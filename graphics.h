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

    /* Mouse coordinates */
    int mouse_x;
    int mouse_y;

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

/* A stack of cards */
typedef struct {
    /* There will never be more than 2 decks of cards in a stack */
    Card cards[104];
    int num_cards;
    SDL_Rect rect;
} Stack;

/*
 * The indices of the stack and card currently targeted by the mouse.
 */
typedef struct {
    int stack;
    int card;
} MouseTarget;

int graphics_init(Graphics *graphics, char *name);
void graphics_free(Graphics *graphics);
SDL_Rect make_rect(int x, int y, int w, int h);
void draw_card(Graphics *graphics, Card *card, SDL_Rect *rect);
void draw_card(Graphics *graphics, Card *card, SDL_Rect *rect);
void draw_stack(Graphics *graphics, Stack *stack);
void move_stack(Stack *srcstack, Stack *dststack, int srcidx);
MouseTarget get_mouse_target(
        Graphics *graphics,
        Stack **stacks,
        int num_stacks);
void update_graphics(Graphics *graphics, int num_stacks);
void update_mouse_stack(Graphics *graphics, Stack *mouse_stacks);

#endif
