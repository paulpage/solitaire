#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "graphics.h"

SDL_Texture * load_texture(SDL_Renderer *renderer, char *filename)
{
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
        printf("Failed to load image %s. SDL Error: %s\n",
                filename, SDL_GetError());
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

int graphics_init(Graphics *graphics, char *name)
{
    graphics->window = SDL_CreateWindow(
            name,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            800,
            600,
            SDL_WINDOW_RESIZABLE);
    if (graphics->window == NULL) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    graphics->renderer = SDL_CreateRenderer(graphics->window, -1, 0);

    graphics->textures = malloc(sizeof(CardTextures));
    graphics->textures->back  = load_texture(graphics->renderer, "card_back.png");
    graphics->textures->front = load_texture(graphics->renderer, "card_front.png");
    graphics->textures->suits = load_texture(graphics->renderer, "suits.png");
    graphics->textures->text  = load_texture(graphics->renderer, "text.png");

    graphics->width = 800;
    graphics->height = 600;
    graphics->mouse_x = 0;
    graphics->mouse_y = 0;

    SDL_SetRenderDrawColor(graphics->renderer, 0, 100, 0, 255);
    return 0;
}

void graphics_free(Graphics *graphics)
{
    SDL_DestroyTexture(graphics->textures->back);
    SDL_DestroyTexture(graphics->textures->front);
    SDL_DestroyTexture(graphics->textures->suits);
    SDL_DestroyTexture(graphics->textures->text);
    SDL_DestroyRenderer(graphics->renderer);
    free(graphics->textures);
    graphics->textures = NULL;
    SDL_DestroyWindow(graphics->window);
}


SDL_Rect make_rect(int x, int y, int w, int h)
{
    SDL_Rect r = { .x = x, .y = y, .w = w, .h = h, };
    return r;
}

void draw_card(Graphics *graphics, Card *card, SDL_Rect *rect)
{
    if (card->orientation == 1) {
        /* Render the front of the card */
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
        SDL_RenderCopy(graphics->renderer,
                graphics->textures->front,
                NULL,
                rect);
        SDL_RenderCopy(
                graphics->renderer,
                graphics->textures->suits,
                &suit_srcrect,
                &suit_dstrect);
        SDL_RenderCopy(
                graphics->renderer,
                graphics->textures->text,
                &text_srcrect,
                &text_dstrect);
        SDL_RenderCopy(
                graphics->renderer,
                graphics->textures->suits,
                &suit_srcrect,
                &suit_center_dstrect);
    } else {
        /* Render the back of the card */
        SDL_RenderCopy(
                graphics->renderer,
                graphics->textures->back,
                NULL,
                rect);
    }
}

int get_stack_offset(Graphics *graphics, Stack *stack)
{
    int offset = graphics->card_h / 4;
    if (graphics->card_h + (offset - 1) * stack->num_cards > stack->rect.h) {
        offset = (stack->rect.h - graphics->card_h) / (stack->num_cards - 1);
    }
    /*
     * Don't return 0 offset because future calculations will
     * divide by the offset
     */
    return offset > 0 ? offset : 1;
}

void draw_stack(Graphics *graphics, Stack *stack)
{
    int offset = get_stack_offset(graphics, stack);
    int margin = graphics->card_w / 16;
    for (int i = 0; i < stack->num_cards; i++) {
        SDL_Rect rect = make_rect(
                stack->rect.x + margin,
                stack->rect.y + (offset * i) + margin,
                graphics->card_w - (2 * margin),
                graphics->card_h - (2 * margin));
        draw_card(graphics, &stack->cards[i], &rect);
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

MouseTarget get_mouse_target(
        Graphics *graphics,
        Stack **stacks,
        int num_stacks)
{
    /* Get the index of the stack that the mouse is over */
    int stack_idx = graphics->mouse_x / graphics->card_w;

    /* Bound the result between 0 and num_stacks */
    stack_idx = (stack_idx < 0 ? 0 : stack_idx);
    stack_idx = (stack_idx > num_stacks - 1
            ? num_stacks - 1
            : stack_idx);

    Stack *stack = &(*stacks)[stack_idx];

    /* Mouse position relative to the stack */
    int mouse_rel_y = graphics->mouse_y - stack->rect.y;

    /* The gap between the tops of the cards in the stack */
    int offset = get_stack_offset(graphics, stack);

    /* 
     * The mouse will either be on a card buried in the stack, on the last
     * card on the stack, or below the stack.
     */
    int card_idx = -1;
    if (mouse_rel_y / offset <= stack->num_cards - 1) {
        card_idx = mouse_rel_y / offset;
    } else if (mouse_rel_y <= offset * (stack->num_cards - 1)
            + graphics->card_h) {
        card_idx = stack->num_cards - 1;
    }

    MouseTarget result = {
        .stack = stack_idx,
        .card = card_idx,
    };
    return result;
}

void update_graphics(Graphics *graphics, int num_stacks)
{
        SDL_GetMouseState(&(graphics->mouse_x), &(graphics->mouse_y));
        SDL_GetWindowSize(
                graphics->window,
                &(graphics->width),
                &(graphics->height));
        graphics->card_w = graphics->width / num_stacks;
        /* Width must be at least 1 to avoid divide by 0 errors */
        graphics->card_w = graphics->card_w > 0 ? graphics->card_w : 1;
        graphics->card_h = graphics->card_w * 7 / 5;
        SDL_RenderClear(graphics->renderer);
}

void update_mouse_stack(Graphics *graphics, Stack *mouse_stack)
{
    mouse_stack->rect.x = graphics->mouse_x - graphics->card_w / 2;
    mouse_stack->rect.y = graphics->mouse_y - graphics->card_h / 4;
}
