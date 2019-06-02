#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* Costants representing the dimensions of the suit and text sprites */
#define SUIT_WIDTH 128
#define SUIT_HEIGHT 128
#define TEXT_WIDTH 256
#define TEXT_HEIGHT 128

typedef struct {
    int width;
    int height;
    int mouse_x;
    int mouse_y;
    bool mouse_down;
} WindowState;

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
        printf("Unable to load image %s. SDL Error: %s\n",
                filename, SDL_GetError());
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

int get_stack_offset(Stack *stack, CardDimensions *card_dimensions) {
    int offset = card_dimensions->h / 4;
    if (card_dimensions->h + (offset - 1) * stack->num_cards > stack->rect.h) {
        offset = (stack->rect.h - card_dimensions->h) / stack->num_cards;
    }
    return offset;
}

void draw_stack(SDL_Renderer *renderer, CardTextures *textures, Stack *stack, CardDimensions *card_dimensions) {
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

void shuffle(Card *deck, size_t num_cards) {
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

void move_stack(Stack *srcstack, Stack *dststack, int srcidx) {
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

int get_card_at_mouse_y(WindowState *window_state, Stack *stack, CardDimensions *card_dimensions) {

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
    } else if (mouse_rel_y <= offset * (stack->num_cards - 1) + card_dimensions->h) {
        return stack->num_cards - 1;
    }
    return -1;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    bool quit = false;
    SDL_Event event;

    init_sdl();
    init_img();

    WindowState window_state = {
        .width = 640,
        .height = 480,
        .mouse_x = 0,
        .mouse_y = 0,
        .mouse_down = false,
    };

    SDL_Window *window = SDL_CreateWindow(
            "Spider Solitaire",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            window_state.width,
            window_state.height,
            SDL_WINDOW_RESIZABLE);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    CardTextures card_textures = {
        .back  = create_texture_from_png(renderer, "card_back.png"),
        .front = create_texture_from_png(renderer, "card_front.png"),
        .suits = create_texture_from_png(renderer, "suits.png"),
        .text  = create_texture_from_png(renderer, "text.png"),
    };

    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);

    // Create a deck from two standard 52-card decks of cards.
    Card *deck = malloc(sizeof(Card) * 104);
    for (int suit = 0; suit < 4; suit++) {
        for (int rank = 0; rank < 13; rank++) {
            Card card;
            card.suit = suit;
            card.rank = rank;
            card.orientation = 1;
            deck[suit * 13 + rank] = card;
            deck[52 + suit * 13 + rank] = card;
        }
    }
    shuffle(deck, 104);

    Stack *stacks = malloc(sizeof(Stack) * 10);
    for (int i = 0; i < 10; i++) {
        stacks[i].num_cards = 0;
        stacks[i].rect = make_rect(
                window_state.width / 10 * i + 3,
                3,
                window_state.width / 10 - 5,
                window_state.height - 5);
    }
    for (int i = 0; i < 104; i++) {
        Stack *stack = &stacks[i % 10];
        stack->cards[stack->num_cards] = deck[i];
        stack->num_cards++;
    }

    CardDimensions card_dimensions = {
        .w = window_state.width / 10,
        .h = (window_state.width / 10) * 7 / 5,
    };
    Stack mouse_stack;
    mouse_stack.num_cards = 0;
    mouse_stack.rect = make_rect(0, 0, card_dimensions.w, window_state.height);
    SDL_GetMouseState(&(mouse_stack.rect.x), &(mouse_stack.rect.y));
    mouse_stack.rect.x -= card_dimensions.w / 2;
    mouse_stack.rect.y -= card_dimensions.h / 2;

    while (!quit) {
        SDL_WaitEvent(&event);

        int target_idx = (mouse_stack.rect.x + (card_dimensions.w / 2)) / card_dimensions.w;
        target_idx = (target_idx < 0 ? 0 : target_idx);
        target_idx = (target_idx > 10 - 1 ? 10 - 1 : target_idx);
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                move_stack(
                        &stacks[target_idx],
                        &mouse_stack,
                        get_card_at_mouse_y(&window_state, &stacks[target_idx], &card_dimensions));
                break;
            case SDL_MOUSEBUTTONUP:
                move_stack(
                        &mouse_stack,
                        &stacks[target_idx],
                        0);
                break;

        }

        SDL_GetMouseState(&(window_state.mouse_x), &(window_state.mouse_y));
        SDL_GetMouseState(&(mouse_stack.rect.x), &(mouse_stack.rect.y));
        mouse_stack.rect.x -= card_dimensions.w / 2;
        mouse_stack.rect.y -= card_dimensions.h / 4;

        SDL_RenderClear(renderer);

        SDL_GetWindowSize(window, &window_state.width, &window_state.height);
        card_dimensions.w = window_state.width / 10;
        card_dimensions.h = card_dimensions.w * 7 / 5;


        for (int i = 0; i < 10; i++) {
            stacks[i].rect = make_rect(
                    window_state.width / 10 * i + 3,
                    3,
                    window_state.width / 10 - 5,
                    window_state.height - 5);
            draw_stack(renderer, &card_textures, &stacks[i], &card_dimensions);
        }

        draw_stack(
                renderer,
                &card_textures,
                &mouse_stack,
                &card_dimensions);
        SDL_RenderPresent(renderer);
    }

    /* Clean up */
    free(deck);
    free(stacks);
    SDL_DestroyTexture(card_textures.back);
    SDL_DestroyTexture(card_textures.front);
    SDL_DestroyTexture(card_textures.suits);
    SDL_DestroyTexture(card_textures.text);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
