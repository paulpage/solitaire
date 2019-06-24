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

    int num_stacks = 10;
    Card *deck = malloc(sizeof(Card) * 104);
    Stack *stacks = malloc(sizeof(Stack) * num_stacks);

    /* Create a deck from two standard 52-card decks of cards. */
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

    for (int i = 0; i < num_stacks; i++) {
        stacks[i].num_cards = 0;
        stacks[i].rect = make_rect(
                graphics.width / num_stacks * i + 3,
                3,
                graphics.width / num_stacks - 5,
                graphics.height - 5);
    }
    for (int i = 0; i < 104; i++) {
        Stack *stack = &stacks[i % num_stacks];
        stack->cards[stack->num_cards] = deck[i];
        stack->num_cards++;
    }

    update_graphics(&graphics, num_stacks);

    MouseTarget target;
    Stack mouse_stack;
    mouse_stack.num_cards = 0;
    mouse_stack.rect = make_rect(0, 0, graphics.card_w, graphics.height);
    update_mouse_stack(&graphics, &mouse_stack);

    while (!quit) {
        SDL_WaitEvent(&event);

        target = get_mouse_target(&graphics, &stacks, num_stacks);
        switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_MOUSEBUTTONDOWN:
                move_stack(&stacks[target.stack], &mouse_stack, target.card);
                break;
            case SDL_MOUSEBUTTONUP:
                move_stack(&mouse_stack, &stacks[target.stack], 0);
                break;
        }

        update_graphics(&graphics, num_stacks);
        update_mouse_stack(&graphics, &mouse_stack);

        for (int i = 0; i < num_stacks; i++) {
            stacks[i].rect = make_rect(
                    graphics.width / num_stacks * i + 3,
                    3,
                    graphics.width / num_stacks - 5,
                    graphics.height - 5);
            draw_stack(&graphics, &stacks[i]);

        }

        draw_stack(&graphics, &mouse_stack);
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
