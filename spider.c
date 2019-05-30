#include "stdio.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#define FALSE 0
#define TRUE 1

#define SUIT_WIDTH 128
#define SUIT_HEIGHT 128
#define TEXT_WIDTH 256
#define TEXT_HEIGHT 128

int screen_width = 640;
int screen_height = 480;

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

typedef struct {
    // There will never be more than 2 decks of cards in a stack
    Card cards[104];
    int num_cards;
    SDL_Rect rect;
} Stack;

int init_sdl() {
    int result = SDL_Init(SDL_INIT_VIDEO);
    if (result != 0) {
        printf("Failed to initialize video. SDL Error: %s\n", SDL_GetError());
    }
    return result;
}

int init_img() {
    return IMG_Init(IMG_INIT_PNG);
}

SDL_Texture * create_texture_from_png(SDL_Renderer *renderer, char* filename) {
    SDL_Surface *surface = IMG_Load(filename);
    if (surface == NULL) {
        printf("Unable to load image %s. SDL Error: %s\n", filename, SDL_GetError());
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Rect make_rect(int x, int y, int w, int h) {
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

void draw_card(SDL_Renderer *renderer, CardTextures *textures, Card *card, SDL_Rect *rect) {
    if (card->orientation == 1) {
        SDL_Rect suit_srcrect = make_rect(card->suit * SUIT_WIDTH, 0, SUIT_WIDTH, SUIT_HEIGHT);
        SDL_Rect text_srcrect = make_rect(card->rank * TEXT_WIDTH, (card->suit / 2) * TEXT_HEIGHT, TEXT_WIDTH, TEXT_HEIGHT);
        SDL_Rect suit_dstrect = make_rect(rect->x + rect->w / 12, rect->y + rect->h / 16, rect->w / 4, rect->w / 4);
        SDL_Rect text_dstrect = make_rect(rect->x + rect->w / 12 * 5, rect->y + rect->h / 16, rect->w / 2, rect->w / 4);
        SDL_Rect suit_center_dstrect = make_rect(rect->x + rect->w / 4, rect->y + rect->h / 3, rect->w / 2, rect->w / 2);
        SDL_RenderCopy(renderer, textures->front, NULL, rect);
        SDL_RenderCopy(renderer, textures->suits, &suit_srcrect, &suit_dstrect);
        SDL_RenderCopy(renderer, textures->text, &text_srcrect, &text_dstrect);
        SDL_RenderCopy(renderer, textures->suits, &suit_srcrect, &suit_center_dstrect);
    } else {
        SDL_RenderCopy(renderer, textures->back, NULL, rect);
    }
}

void draw_stack(SDL_Renderer *renderer, CardTextures *textures, Stack *stack, SDL_Rect *card_rect) {
    int offset = card_rect->h / 4;
    if (card_rect->h + (offset - 1) * stack->num_cards > stack->rect.h) {
        offset = (stack->rect.h - card_rect->h) / stack->num_cards;
    }
    for (int i = 0; i < stack->num_cards; i++) {
        SDL_Rect rect = make_rect(card_rect->x, stack->rect.y + (offset * i), card_rect->w, card_rect->h);
        draw_card(renderer, textures, &stack->cards[i], &rect);
    }
}

int main(int argc, char* argv[]) {
    int quit = FALSE;
    SDL_Event event;

    init_sdl();
    init_img();

    SDL_Window *window = SDL_CreateWindow("Spider Solitaire", 
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
            screen_width, screen_height, SDL_WINDOW_RESIZABLE);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    CardTextures *card_textures = malloc(sizeof(CardTextures));
    card_textures->back  = create_texture_from_png(renderer, "card_back.png");
    card_textures->front = create_texture_from_png(renderer, "card_front.png");
    card_textures->suits = create_texture_from_png(renderer, "suits.png");
    card_textures->text  = create_texture_from_png(renderer, "text.png");

    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);

    while (!quit) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                quit = TRUE;
                break;
        }

        SDL_RenderClear(renderer);

        SDL_GetWindowSize(window, &screen_width, &screen_height);
        int card_width = screen_width / 13;
        int card_height = card_width * 7 / 5;

        for (int suit = 0; suit < 4; suit++) {
            Stack stack;
            stack.rect = make_rect(suit * card_width + 3, 3, card_width - 5, screen_height - 6);
            stack.num_cards = 0;


            for (int rank = 0; rank < 13; rank++) {
                Card card;
                card.rank = rank;
                card.suit = suit;
                card.orientation = rank % 3;
                stack.cards[rank] = card;
                stack.num_cards++;
                /* draw_card(renderer, card_textures, card, &card_rect); */
            }
            SDL_Rect card_rect = make_rect(suit * card_width + 3, card_height + 5, card_width - 5, card_height - 5);
            draw_stack(renderer, card_textures, &stack, &card_rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(card_textures->back);
    SDL_DestroyTexture(card_textures->front);
    SDL_DestroyTexture(card_textures->suits);
    SDL_DestroyTexture(card_textures->text);
    free(card_textures);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
