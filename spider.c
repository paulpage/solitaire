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

void draw_card(SDL_Renderer *renderer, CardTextures *textures, int rank, int suit, SDL_Rect *rect) {
    SDL_Rect suit_srcrect = make_rect(suit * SUIT_WIDTH, 0, SUIT_WIDTH, SUIT_HEIGHT);
    SDL_Rect text_srcrect = make_rect(rank * TEXT_WIDTH, (suit / 2) * TEXT_HEIGHT, TEXT_WIDTH, TEXT_HEIGHT);
    SDL_Rect suit_dstrect = make_rect(rect->x + rect->w / 8, rect->y + rect->h / 4, rect->w / 4, rect->w / 4);
    SDL_Rect text_dstrect = make_rect(rect->x + rect->w / 2, rect->y + rect->h / 4, rect->w / 2, rect->w / 4);
    SDL_RenderCopy(renderer, textures->front, NULL, rect);
    SDL_RenderCopy(renderer, textures->suits, &suit_srcrect, &suit_dstrect);
    SDL_RenderCopy(renderer, textures->text, &text_srcrect, &text_dstrect);
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
            for (int rank = 0; rank < 13; rank++) {
                SDL_Rect card_rect = make_rect(rank * card_width + 3, card_height * suit + 5, card_width - 5, card_height - 5);
                draw_card(renderer, card_textures, rank, suit, &card_rect);
            }
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
