#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "gui.h"

int init_sdl()
{
    int result = SDL_Init(SDL_INIT_VIDEO);
    if (result != 0) {
        printf("Failed to initialize video. SDL Error: %s\n", SDL_GetError());
    }
    return result;
}

int init_img()
{
    return IMG_Init(IMG_INIT_PNG);
}

SDL_Texture * create_texture_from_png(SDL_Renderer *renderer, char* filename)
{
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
        printf("Unable to load image %s. SDL Error: %s\n",
                filename, SDL_GetError());
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Rect make_rect(int x, int y, int w, int h)
{
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

void draw_card(SDL_Renderer *renderer, CardTextures *textures,
        Card *card, SDL_Rect *rect) {
    if (card->orientation == 1) {
        SDL_Rect suit_srcrect = make_rect(
                card->suit * SUIT_WIDTH, 0, SUIT_WIDTH, SUIT_HEIGHT);
        SDL_Rect text_srcrect = make_rect(
                card->rank * TEXT_WIDTH,
                (card->suit / 2) * TEXT_HEIGHT,
                TEXT_WIDTH,
                TEXT_HEIGHT);
        SDL_Rect suit_dstrect = make_rect(
                rect->x + rect->w / 12,
                rect->y + rect->h / 16,
                rect->w / 4,
                rect->w / 4);
        SDL_Rect text_dstrect = make_rect(
                rect->x + rect->w / 12 * 5,
                rect->y + rect->h / 16,
                rect->w / 2,
                rect->w / 4);
        SDL_Rect suit_center_dstrect = make_rect(
                rect->x + rect->w / 4,
                rect->y + rect->h / 3,
                rect->w / 2,
                rect->w / 2);
        SDL_RenderCopy(renderer, textures->front, NULL, rect);
        SDL_RenderCopy(renderer, textures->suits, &suit_srcrect, &suit_dstrect);
        SDL_RenderCopy(renderer, textures->text, &text_srcrect, &text_dstrect);
        SDL_RenderCopy(
                renderer, textures->suits, &suit_srcrect, &suit_center_dstrect);
    } else {
        SDL_RenderCopy(renderer, textures->back, NULL, rect);
    }
}

int get_stack_offset(Stack *stack, CardDimensions *card_dimensions)
{
    int offset = card_dimensions->h / 4;
    if (card_dimensions->h + (offset - 1) * stack->num_cards > stack->rect.h) {
        offset = (stack->rect.h - card_dimensions->h) / stack->num_cards;
    }
    return offset;
}

void draw_stack(SDL_Renderer *renderer, CardTextures *textures,
        Stack *stack, CardDimensions *card_dimensions)
{
    int offset = get_stack_offset(stack, card_dimensions);
    int margin = card_dimensions->w / 16;
    for (int i = 0; i < stack->num_cards; i++) {
        SDL_Rect rect = make_rect(
                stack->rect.x + margin,
                stack->rect.y + (offset * i) + margin,
                card_dimensions->w - (2 * margin),
                card_dimensions->h - (2 * margin));
        draw_card(renderer, textures, &stack->cards[i], &rect);
    }
}

void shuffle(Card *deck, size_t num_cards)
{
    if (num_cards > 1) {
        size_t i;
        for (i = 0; i < num_cards - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (num_cards - i) + 1);
            Card card = deck[j];
            deck[j] = deck[i];
            deck[i] = card;
        }
    }
}

void move_stack(Stack *srcstack, Stack *dststack, int srcidx)
{
    if (srcidx < 0) {
        return;
    }
    int end = srcstack->num_cards;
    for (int i = srcidx; i < end; i++) {
        dststack->cards[dststack->num_cards] =
            srcstack->cards[i];
        dststack->num_cards++;
        srcstack->num_cards--;
    }
    assert(srcstack->num_cards == srcidx);
}

int get_card_at_mouse_y(WindowState *window_state, 
        Stack *stack, CardDimensions *card_dimensions)
{

    /* Mouse position relative to the stack */
    int mouse_rel_y = window_state->mouse_y - stack->rect.y;

    /* The gap between the tops of the cards in the stack */
    int offset = get_stack_offset(stack, card_dimensions);

    /* 
     * The mouse will either be on a card buried in the stack, on the last
     * card on the stack, or below the stack.
     */
    if (mouse_rel_y / offset <= stack->num_cards - 1) {
        return mouse_rel_y / offset;
    } else if (mouse_rel_y <= offset * (stack->num_cards - 1)
            + card_dimensions->h) {
        return stack->num_cards - 1;
    }
    return -1;
}

