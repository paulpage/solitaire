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

int get_pile_offset(Graphics *graphics, Pile *pile)
{
    int offset = graphics->card_h / 4;
    if (graphics->card_h + (offset - 1) * pile->num_cards > pile->rect.h) {
        offset = (pile->rect.h - graphics->card_h) / (pile->num_cards - 1);
    }
    /*
     * Don't return 0 offset because future calculations will
     * divide by the offset
     */
    return offset > 0 ? offset : 1;
}

void draw_pile(Graphics *graphics, Pile *pile)
{
    int offset = get_pile_offset(graphics, pile);
    int margin = graphics->card_w / 16;
    for (int i = 0; i < pile->num_cards; i++) {
        SDL_Rect rect = make_rect(
                pile->rect.x + margin,
                pile->rect.y + (offset * i) + margin,
                graphics->card_w - (2 * margin),
                graphics->card_h - (2 * margin));
        draw_card(graphics, &pile->cards[i], &rect);
    }
}

void move_pile(Pile *srcpile, Pile *dstpile, int srcidx)
{
    if (srcidx < 0) {
        return;
    }
    int end = srcpile->num_cards;
    for (int i = srcidx; i < end; i++) {
        dstpile->cards[dstpile->num_cards] =
            srcpile->cards[i];
        dstpile->num_cards++;
        srcpile->num_cards--;
    }
    assert(srcpile->num_cards == srcidx);
}

MouseTarget get_mouse_target(
        Graphics *graphics,
        Pile piles[],
        int num_piles)
{
    /* Get the index of the pile that the mouse is over */
    int pile_idx = graphics->mouse_x / graphics->card_w;

    /* Bound the result between 0 and num_piles */
    pile_idx = (pile_idx < 0 ? 0 : pile_idx);
    pile_idx = (pile_idx > num_piles - 1
            ? num_piles - 1
            : pile_idx);

    Pile pile = piles[pile_idx];

    /* Mouse position relative to the pile */
    int mouse_rel_y = graphics->mouse_y - pile.rect.y;

    /* The gap between the tops of the cards in the pile */
    int offset = get_pile_offset(graphics, &pile);

    /* 
     * The mouse will either be on a card buried in the pile, on the last
     * card on the pile, or below the pile.
     */
    int card_idx = -1;
    if (mouse_rel_y / offset <= pile.num_cards - 1) {
        card_idx = mouse_rel_y / offset;
    } else if (mouse_rel_y <= offset * (pile.num_cards - 1)
            + graphics->card_h) {
        card_idx = pile.num_cards - 1;
    }

    MouseTarget result = {
        .pile = pile_idx,
        .card = card_idx,
    };
    return result;
}

void update_graphics(Graphics *graphics, int num_piles)
{
        SDL_GetMouseState(&(graphics->mouse_x), &(graphics->mouse_y));
        SDL_GetWindowSize(
                graphics->window,
                &(graphics->width),
                &(graphics->height));
        graphics->card_w = graphics->width / num_piles;
        /* Width must be at least 1 to avoid divide by 0 errors */
        graphics->card_w = graphics->card_w > 0 ? graphics->card_w : 1;
        graphics->card_h = graphics->card_w * 7 / 5;
        SDL_RenderClear(graphics->renderer);
}

void update_mouse_pile(Graphics *graphics, Pile *mouse_pile)
{
    mouse_pile->rect.x = graphics->mouse_x - graphics->card_w / 2;
    mouse_pile->rect.y = graphics->mouse_y - graphics->card_h / 4;
}
