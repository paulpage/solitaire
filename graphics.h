#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>

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
    int width;
    int height;
    int mouse_x;
    int mouse_y;
    SDL_Window *window;
    SDL_Renderer *renderer;
    CardTextures *textures;
} Graphics;

/* Card sprite dimensions in pixels. */
typedef struct {
    int w;
    int h;
} CardSize;

typedef struct {
    int suit;
    int rank;
    int orientation;
} Card; /* TODO: Put in cards */

/* A stack of cards */
typedef struct {
    /* There will never be more than 2 decks of cards in a stack */
    Card cards[104];
    int num_cards;
    SDL_Rect rect;
} Stack;

int graphics_init(Graphics *graphics, char *name);
void graphics_free(Graphics *graphics);
SDL_Rect make_rect(int x, int y, int w, int h);
void draw_card(Graphics *graphics, Card *card, SDL_Rect *rect);
void draw_card(Graphics *graphics, Card *card, SDL_Rect *rect);
void draw_stack(
        Graphics *graphics,
        Stack *stack,
        CardSize *card_size);
void shuffle(Card *deck, size_t num_cards); /* TODO: put in cards */
void move_stack(Stack *srcstack, Stack *dststack, int srcidx);
int get_card_at_mouse_y(
        Graphics *graphics, 
        Stack *stack,
        CardSize *card_size);


#endif
