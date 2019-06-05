#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* Dimensions of the suit and text sprites in pixels. */
#define SUIT_WIDTH 128
#define SUIT_HEIGHT 128
#define TEXT_WIDTH 256
#define TEXT_HEIGHT 128

/* Window data. All integer values are in pixels. */
typedef struct {
    int width;
    int height;
    int mouse_x;
    int mouse_y;
    bool mouse_down;
} WindowState;

/* Card sprite dimensions in pixels. */
typedef struct {
    int w;
    int h;
} CardDimensions;

/* Textures used to render cards */
typedef struct {
    SDL_Texture *back;
    SDL_Texture *front;
    SDL_Texture *suits;
    SDL_Texture *text;
} CardTextures;

typedef struct {
    int suit;
    int rank;
    int orientation;
} Card;

/* A stack of cards */
typedef struct {
    /* There will never be more than 2 decks of cards in a stack */
    Card cards[104];
    int num_cards;
    SDL_Rect rect;
} Stack;

int init_sdl();
int init_img();
SDL_Texture * create_texture_from_png(SDL_Renderer *renderer, char* filename);
SDL_Rect make_rect(int x, int y, int w, int h);
void draw_card(SDL_Renderer *renderer, CardTextures *textures, 
        Card *card, SDL_Rect *rect);
int get_stack_offset(Stack *stack, CardDimensions *card_dimensions);
void draw_stack(SDL_Renderer *renderer, CardTextures *textures,
        Stack *stack, CardDimensions *card_dimensions);
void shuffle(Card *deck, size_t num_cards);
void move_stack(Stack *srcstack, Stack *dststack, int srcidx);
int get_card_at_mouse_y(WindowState *Window_state, Stack *stack,
        CardDimensions *card_dimensions);
