#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "graphics.h"

int main(int argc, char* argv[])
{
    /* Seed the random number generator */
    srand(time(NULL));

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO != 0)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    /* Initialize SDL Image library */
    IMG_Init(IMG_INIT_PNG);

    Graphics graphics;
    if (graphics_init(&graphics, "Spider Solitaire") != 0) {
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    bool quit = false;
    SDL_Event event;

    Card *deck = malloc(sizeof(Card) * 104);
    Stack *stacks = malloc(sizeof(Stack) * 10);

    // Create a deck from two standard 52-card decks of cards.
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

    for (int i = 0; i < 10; i++) {
        stacks[i].num_cards = 0;
        stacks[i].rect = make_rect(
                graphics.width / 10 * i + 3,
                3,
                graphics.width / 10 - 5,
                graphics.height - 5);
    }
    for (int i = 0; i < 104; i++) {
        Stack *stack = &stacks[i % 10];
        stack->cards[stack->num_cards] = deck[i];
        stack->num_cards++;
    }

    CardSize card_size = {
        .w = graphics.width / 10,
        .h = (graphics.width / 10) * 7 / 5,
    };
    Stack mouse_stack;
    mouse_stack.num_cards = 0;
    mouse_stack.rect = make_rect(0, 0, card_size.w, graphics.height);
    SDL_GetMouseState(&(mouse_stack.rect.x), &(mouse_stack.rect.y));
    mouse_stack.rect.x -= card_size.w / 2;
    mouse_stack.rect.y -= card_size.h / 2;

    while (!quit) {
        SDL_WaitEvent(&event);

        int target_idx = (mouse_stack.rect.x + (card_size.w / 2))
            / card_size.w;
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
                            &graphics,
                            &stacks[target_idx],
                            &card_size));
                break;
            case SDL_MOUSEBUTTONUP:
                move_stack(
                        &mouse_stack,
                        &stacks[target_idx],
                        0);
                break;

        }

        SDL_GetMouseState(&(graphics.mouse_x), &(graphics.mouse_y));
        SDL_GetMouseState(&(mouse_stack.rect.x), &(mouse_stack.rect.y));
        mouse_stack.rect.x -= card_size.w / 2;
        mouse_stack.rect.y -= card_size.h / 4;

        SDL_RenderClear(graphics.renderer);

        SDL_GetWindowSize(graphics.window, &graphics.width, &graphics.height);
        card_size.w = graphics.width / 10;
        card_size.h = card_size.w * 7 / 5;


        for (int i = 0; i < 10; i++) {
            stacks[i].rect = make_rect(
                    graphics.width / 10 * i + 3,
                    3,
                    graphics.width / 10 - 5,
                    graphics.height - 5);
            draw_stack(&graphics, &stacks[i], &card_size);

        }

        draw_stack(&graphics, &mouse_stack, &card_size);
        SDL_RenderPresent(graphics.renderer);
    }


    /* Clean up */
    free(deck);
    free(stacks);
    graphics_free(&graphics);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
