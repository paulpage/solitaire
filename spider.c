#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "gui.h"

int main(int argc, char* argv[])
{
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

        int target_idx = (mouse_stack.rect.x + (card_dimensions.w / 2))
            / card_dimensions.w;
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
                        get_card_at_mouse_y(
                            &window_state,
                            &stacks[target_idx],
                            &card_dimensions));
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
